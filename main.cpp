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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "ReleaseFetcher.h"
#include "LauncherConfig.h"
#include "ArchiveExtractor.h"
#include <nlohmann/json.hpp>
#include <date.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>

using json = nlohmann::json;

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
    if (current.cd("current")) {
#ifdef Q_OS_WIN
        {
            QProcess proc;
            proc.setWorkingDirectory(home.absolutePath());
            auto path = current.absolutePath().replace('/', "\\");
            proc.start("cmd.exe", QStringList() << "/c" << "rmdir" << path);
            proc.waitForFinished();
        }
#else
        current.removeRecursively();
#endif
    }

    QDir version = QDir::home();
    version.cd(".REGoth");
    version.cd("releases");
    version.cd(tagName);

#ifdef Q_OS_WIN
    {
        QProcess proc;
        proc.setWorkingDirectory(home.absolutePath());
        auto path = version.absolutePath().replace('/', "\\");
        proc.start("cmd.exe", QStringList() << "/c" << "mklink" << "/J" << "current" << path);
        proc.waitForFinished();
    }
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
        m_fetcher = std::make_unique<ReleaseFetcher>(cfg.getReleasesEndpoint());
        m_downloadManager = std::make_unique<QNetworkAccessManager>();

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
                SLOT(checkNewReleases()));
            QMetaObject::invokeMethod(dialog, "open");
        }

        connect(m_root, SIGNAL(addInstallation(QString)),
            SLOT(addInstallation(QString)));

        connect(m_root, SIGNAL(playGame(QString)),
                SLOT(playGame(QString)));

        connect(m_root, SIGNAL(removeInstallation(int)),
                SLOT(removeInstallation(int)));

        connect(m_root, SIGNAL(checkNewReleases()),
                SLOT(checkNewReleases()));

        m_reply = nullptr;
        m_rel = {};
    }

public slots:
    void downloadLatestVersion(std::optional<Release> rel)
    {
        if (rel.has_value())
        {
            if (!m_cfg.getDefaultRelease().empty() && !m_cfg.getReleases().empty()) {
                date::Date latestDate = date::Date::fromIsoDatetime(rel.value().ReleaseDate);
                for (const auto& rel : m_cfg.getReleases()) {
                    date::Date relDate = date::Date::fromIsoDatetime(rel.ReleaseDate);
                    if (relDate >= latestDate) {
                        QObject *dialog = m_root->findChild<QObject*>("UpToDateDialog");
                        QMetaObject::invokeMethod(dialog, "open");
                        return;
                    }
                }
            }

            QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
            QMetaObject::invokeMethod(dialog, "show");
            dialog->setProperty("versionName", QString(rel.value().Name.c_str()));
            dialog->setProperty("fileName", QString(rel.value().DownloadUrl.c_str()));

            QNetworkRequest request;
            request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            request.setUrl(QUrl(rel.value().DownloadUrl.c_str()));
            m_reply = m_downloadManager->get(request);
            

            connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)),
                SLOT(updateProgress(qint64, qint64)));
            connect(m_downloadManager.get(), SIGNAL(finished(QNetworkReply *)),
                SLOT(downloadComplete(QNetworkReply *)));
            connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
                SLOT(downloadError(QNetworkReply::NetworkError)));

            connect(dialog, SIGNAL(cancelDownload()),
                SLOT(cancelDownload()));

            m_rel = rel;
        } 
        else
        {
            QObject *dialog = m_root->findChild<QObject*>("UpToDateDialog");
            QMetaObject::invokeMethod(dialog, "open");
        }
    }

    void addInstallation(QString urlString)
    {
        Installation inst;
        inst.Name = QDir(urlString).dirName().toStdString();
        inst.Url = urlString.toStdString();

        m_cfg.getGothicInstallations().push_back(inst);
        InstallationEntry *entry = new InstallationEntry();
        entry->setName(QDir(urlString).dirName());
#ifdef Q_OS_WIN
        urlString = urlString.right(urlString.size() - 1);
#endif
        entry->setUrl(urlString);
        m_installationList.push_back(entry);
        m_ctx->setContextProperty("installations", QVariant::fromValue(m_installationList));
    }

    void removeInstallation(int position)
    {
        auto& installations = m_cfg.getGothicInstallations();
        installations.erase(installations.begin() + position);

        m_installationList.erase(m_installationList.begin() + position);
        m_ctx->setContextProperty("installations", QVariant::fromValue(m_installationList));
    }

    void checkNewReleases()
    {
        connect(m_fetcher.get(), SIGNAL(versionFetched(std::optional<Release>)),
            SLOT(downloadLatestVersion(std::optional<Release>)));
        m_fetcher->fetch();
    }

    void cancelDownload()
    {
        if (m_reply != nullptr) {
            m_reply->abort();
        }
    }

    void downloadComplete(QNetworkReply *reply)
    {
        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        QMetaObject::invokeMethod(dialog, "close");

        if(!reply->error())
        {
            Release r = m_rel.value();

            QDir home = QDir::home();
            home.cd(".REGoth");
            home.cd("releases");
            home.mkdir(r.Tag.c_str());
            home.cd(r.Tag.c_str());

            auto buf = reply->readAll();
            std::vector<std::uint8_t> data(buf.begin(), buf.end());

            extractArchive(data, home.absolutePath().toStdString());
            m_cfg.getReleases().push_back(r);
            m_cfg.setDefaultRelease(r.Tag);
            createLinkToRelease(r.Tag.c_str());
        }
    }

    void downloadError(QNetworkReply::NetworkError code)
    {
        QObject *dialog = m_root->findChild<QObject*>("DownloadErrorDialog");
        dialog->setProperty("errorMessage", m_reply->errorString());
        QMetaObject::invokeMethod(dialog, "open");
    }

    void updateProgress(qint64 current, qint64 total)
    {
        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        if (total == 0)
        {
            dialog->setProperty("isIndeterminate", true);
        }
        else
        {
            dialog->setProperty("isIndeterminate", false);
            dialog->setProperty("progress", (float)current / (float)total);
        }
    }

    void playGame(QString url)
    {
        QProcess proc;
        QDir home = QDir::home();
        home.cd(".REGoth");
        home.mkdir("logs");
        home.cd("logs");
        QString logname = QString("crashlog_") + QDateTime::currentDateTime().toString("yyyy-MM-ddTHH_mm_ss") + QString(".txt");
        QString logfilePath = home.absoluteFilePath(logname);
        home.cdUp();
        home.cd("current");

#ifdef Q_OS_WIN
        QString executable = home.absoluteFilePath("REGoth.exe");
        logfilePath = logfilePath.replace('/', "\\");
#else
        QString executable = home.absoluteFilePath("REGoth");
#endif
        QString workingDirectory = home.absolutePath();
        std::string exec = executable.toStdString();
        proc.setWorkingDirectory(workingDirectory);
        proc.start(executable, QStringList() << "-g" << url);
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
    QObject *m_root;
    QQmlContext *m_ctx;
    LauncherConfig& m_cfg;
    QList<QObject*> m_installationList;
    QQmlApplicationEngine& m_engine;
    std::unique_ptr<ReleaseFetcher> m_fetcher;
    std::unique_ptr<QNetworkAccessManager> m_downloadManager;
    QNetworkReply *m_reply;
    std::optional<Release> m_rel;
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
