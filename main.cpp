#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <QVariant>
#include <QCloseEvent>
#include <QQmlContext>
#include <QUrl>
#include <QProcess>
#include <QDateTime>
#include <QFuture>
#include "ReleaseFetcher.h"
#include "LauncherConfig.h"
#include "CurlDownloader.h"
#include "ArchiveExtractor.h"
#include <nlohmann/json.hpp>
#include <date.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>

using json = nlohmann::json;

class CurlThread : public QThread
{
    Q_OBJECT
public:
    bool continueDownload = true;
    std::vector<std::uint8_t> data;
    CurlThread(const std::string& url)
    {
        m_downloader = std::make_unique<CurlDownloader>(url, [&](double total, double current) -> bool {
            emit progressChanged(total, current);
            return continueDownload;
        });
    }

    void run()
    {
        data = m_downloader->get();
        return;
    }

    QString error() const
    {
        return QString(m_downloader->getError().c_str());
    }

    Release release() const
    {
        return m_release;
    }

    void setRelease(Release rel)
    {
        m_release = rel;
    }

signals:
    void progressChanged(double total, double current);

private:
    std::unique_ptr<CurlDownloader> m_downloader;
    Release m_release;
};

/// Represents an entry in the installation list
class InstallationEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

public:
    void setName(QString name)
    {
        m_name = name;
        emit nameChanged(name);
    }

    QString name() const
    {
        return m_name;
    }

    void setUrl(QString url)
    {
        m_url = url;
        emit urlChanged(url);
    }

    QString url() const
    {
        return m_url;
    }

signals:
    void nameChanged(QString);
    void urlChanged(QString);

private:
    QString m_name;
    QString m_url;
};

static void createLinkToRelease(QString tagName)
{
    QDir home = QDir::home();
    home.cd(".REGoth");

    QDir current = QDir::home();
    current.cd(".REGoth");
    if(current.cd("current")) current.removeRecursively();

    QDir version = QDir::home();
    version.cd(".REGoth");
    version.cd("releases");
    version.cd(tagName);

#ifdef Q_OS_WIN
    QFile::link(version.absolutePath(), home.absoluteFilePath("current") + QString(".lnk"));
#else
    QFile::link(version.absolutePath(), home.absoluteFilePath("current"));
#endif
}

class MainWindow : QObject
{
    Q_OBJECT
public:

    MainWindow(QQmlApplicationEngine& engine, LauncherConfig& cfg)
        : m_cfg(cfg)
        , m_engine(engine)
    {
        m_ctx = engine.rootContext();

        for (const auto& inst : cfg.getGothicInstallations()) {
            InstallationEntry *entry = new InstallationEntry();
            entry->setName(QString(inst.Name.c_str()));
            entry->setUrl(QString(inst.Url.c_str()));

            m_installationList.push_back(entry);
        }

        m_ctx->setContextProperty("installations", QVariant::fromValue(m_installationList));

        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        if (engine.rootObjects().isEmpty())
            throw std::runtime_error("Error while loading main.qml");

        m_root = engine.rootObjects()[0];

        // Check if this is the first time the launcher is used
        if (m_cfg.getDefaultRelease().empty() && m_cfg.getReleases().empty())
        {
            QObject *dialog = m_root->findChild<QObject*>("FirstStartupDialog");
            connect(dialog, SIGNAL(yes()),
                SLOT(downloadLatestVersion()));
            QMetaObject::invokeMethod(dialog, "open");
        }

        connect(m_root, SIGNAL(addInstallation(QString)),
            SLOT(addInstallation(QString)));

        connect(m_root, SIGNAL(playGame(QString)),
                SLOT(playGame(QString)));

        connect(m_root, SIGNAL(checkNewReleases()),
                SLOT(checkNewReleases()));
    }

public slots:
    void downloadLatestVersion()
    {
        ReleaseFetcher fetcher(m_cfg.getReleasesEndpoint());

        Release rel;
        fetcher.getLatestRelease(&rel);

        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        QMetaObject::invokeMethod(dialog, "show");
        dialog->setProperty("versionName", QString(rel.Name.c_str()));
        dialog->setProperty("fileName", QString(rel.DownloadUrl.c_str()));

        m_downloadThread = new CurlThread(rel.DownloadUrl);
        connect(m_downloadThread, SIGNAL(progressChanged(double, double)),
            SLOT(updateProgress(double, double)));
        connect(m_downloadThread, SIGNAL(finished()),
            SLOT(downloadComplete()));

        connect(dialog, SIGNAL(cancelDownload()),
            SLOT(cancelDownload()));

        m_downloadThread->setRelease(rel);

        // Run, Forest, run!
        m_downloadThread->start();
    }

    void addInstallation(QString urlString)
    {
        Installation inst;
        inst.Name = QDir(urlString).dirName().toStdString();
        inst.Url = urlString.toStdString();

        m_cfg.getGothicInstallations().push_back(inst);
        InstallationEntry *entry = new InstallationEntry();
        entry->setName(QDir(urlString).dirName());
        entry->setUrl(urlString);
        m_installationList.push_back(entry);
        m_ctx->setContextProperty("installations", QVariant::fromValue(m_installationList));
    }

    void checkNewReleases()
    {
        ReleaseFetcher fetcher(m_cfg.getReleasesEndpoint());
        Release latest;
        fetcher.getLatestRelease(&latest);
        if(!m_cfg.getDefaultRelease().empty() && !m_cfg.getReleases().empty())
        {
            date::Date latestDate = date::Date::fromIsoDatetime(latest.ReleaseDate);
            for(const auto& rel : m_cfg.getReleases())
            {
                date::Date relDate = date::Date::fromIsoDatetime(rel.ReleaseDate);
                if(relDate >= latestDate)
                {
                    QObject *dialog = m_root->findChild<QObject*>("UpToDateDialog");
                    QMetaObject::invokeMethod(dialog, "open");
                    return;
                }
            }
        }

        QObject *dialog = m_root->findChild<QObject*>("NewReleaseAvailableDialog");
        dialog->setProperty("releaseName", QString(latest.Name.c_str()));
        connect(dialog, SIGNAL(yes()),
                SLOT(downloadLatestVersion()));
        QMetaObject::invokeMethod(dialog, "open");
    }

    void cancelDownload()
    {
        m_downloadThread->continueDownload = false;
    }

    void downloadComplete()
    {
        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        QMetaObject::invokeMethod(dialog, "close");

        if(m_downloadThread->data.empty() && m_downloadThread->continueDownload)
        {
            QObject *dialog = m_root->findChild<QObject*>("DownloadErrorDialog");
            dialog->setProperty("errorMessage", m_downloadThread->error());
            QMetaObject::invokeMethod(dialog, "open");
        }
        else
        {
            Release r = m_downloadThread->release();

            QDir home = QDir::home();
            home.cd(".REGoth");
            home.cd("releases");
            home.mkdir(r.Tag.c_str());
            home.cd(r.Tag.c_str());

            extractArchive(m_downloadThread->data, home.absolutePath().toStdString());
            m_cfg.getReleases().push_back(r);
            m_cfg.setDefaultRelease(r.Tag);
            createLinkToRelease(r.Tag.c_str());
        }
    }

    void updateProgress(double total, double current)
    {
        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        if (total == 0)
        {
            dialog->setProperty("isIndeterminate", true);
        }
        else
        {
            dialog->setProperty("isIndeterminate", false);
            dialog->setProperty("progress", (float)(current / total));
        }
    }

    void playGame(QString url)
    {
        QProcess proc;
        QDir home = QDir::home();
        home.cd(".REGoth");
        home.mkdir("logs");
        home.cd("logs");
        QString logname = QString("crashlog_") + QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss") + QString(".txt");
        QString logfilePath = home.absoluteFilePath(logname);
        home.cdUp();
        home.cd("current");
        proc.setWorkingDirectory(home.absolutePath());
        proc.start(home.absoluteFilePath("REGoth"), QStringList() << "-g" << url);
        proc.waitForFinished(-1);
        int ecode = proc.exitCode();
        if(proc.exitCode() != 0)
        {
            QFile logfile(logfilePath);
            logfile.open(QIODevice::WriteOnly);
            logfile.write("############ BEGIN STDOUT LOG ############\n");
            logfile.write(proc.readAllStandardOutput());
            logfile.write("############ BEGIN STDERR LOG ############\n");
            logfile.write(proc.readAllStandardError());
            logfile.close();

            QObject *dialog = m_root->findChild<QObject*>("CrashDialog");
            dialog->setProperty("logFile", logfilePath);
            QMetaObject::invokeMethod(dialog, "open");
        }
    }

private:
    CurlThread *m_downloadThread;
    QObject *m_root;
    QQmlContext *m_ctx;
    LauncherConfig& m_cfg;
    QList<QObject*> m_installationList;
    QQmlApplicationEngine& m_engine;
};

/// Open the config file in the home directory or creates it if it doesn't exist yet
static LauncherConfig openConfigFile()
{
    QDir home = QDir::home();
    if(!home.exists(".REGoth"))
    {
        home.mkdir(".REGoth");
    }

    if(!home.cd(".REGoth"))
    {
        throw std::runtime_error("Cannot access .regoth directory");
    }

    if(!home.exists("releases"))
    {
        home.mkdir("releases");
    }

    if(home.exists("launcher.json"))
    {
        std::ifstream stream(home.absoluteFilePath("launcher.json").toStdString().c_str());
        json j;
        stream >> j;
        stream.close();
        return LauncherConfig::deserialize(j);
    }
    else
    {
        LauncherConfig cfg;
        cfg.setShowPrereleases(true);
        cfg.setReleasesEndpoint("https://api.github.com/repos/REGoth-Project/REGoth/releases");

        std::ofstream stream(home.absoluteFilePath("launcher.json").toStdString().c_str());
        stream << cfg.serialize();
        stream.close();
        return cfg;
    }
}

/// Saves the launcher configuration before closing the app
static void saveConfig(const LauncherConfig& cfg)
{
    QDir home = QDir::home();
    home.cd(".REGoth");

    std::ofstream stream(home.absoluteFilePath("launcher.json").toStdString().c_str());
    stream << cfg.serialize();
    stream.close();
}

int main(int argc, char *argv[])
{
    LauncherConfig cfg = openConfigFile();

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    MainWindow window(engine, cfg);

    int retVal = app.exec();

    saveConfig(cfg);

    return retVal;
}

#include "main.moc"
