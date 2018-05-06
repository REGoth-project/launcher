#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <QVariant>
#include <QCloseEvent>
#include <QQmlContext>
#include "ReleaseFetcher.h"
#include "LauncherConfig.h"
#include "CurlDownloader.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>
#include <fstream>
#include <thread>

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

    void run() {
        data = m_downloader->get();
        return;
    }

signals:
    void progressChanged(double total, double current);

private:
    std::unique_ptr<CurlDownloader> m_downloader;
};

class InstallationEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    void setName(QString name)
    {
        m_name = name;
        emit nameChanged(name);
    }

    QString name() const {
        return m_name;
    }

signals:
    void nameChanged(QString);

private:
    QString m_name;
};

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
            entry->setName(QString(inst.c_str()));

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

        connect(m_root, SIGNAL(addInstallation(QUrl)),
            SLOT(addInstallation(QUrl)));
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

        workerThread = new CurlThread(rel.DownloadUrl);
        // Connect our signal and slot
        connect(workerThread, SIGNAL(progressChanged(double, double)),
            SLOT(updateProgress(double, double)));
        // Setup callback for cleanup when it finishes
        connect(workerThread, SIGNAL(finished()),
            SLOT(downloadComplete()));

        connect(dialog, SIGNAL(cancelDownload()),
            SLOT(cancelDownload()));

        // Run, Forest, run!
        workerThread->start();
    }

    void addInstallation(QUrl url)
    {
        m_cfg.getGothicInstallations().push_back(url.toString().toStdString());
        InstallationEntry *entry = new InstallationEntry();
        entry->setName(url.toString());
        m_installationList.push_back(entry);
        m_ctx->setContextProperty("installations", QVariant::fromValue(m_installationList));
    }

    void checkAvailableVersions()
    {
    }

    void cancelDownload()
    {
        workerThread->continueDownload = false;
    }

    void downloadComplete()
    {
        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        QMetaObject::invokeMethod(dialog, "close");
    }

    void updateProgress(double total, double current)
    {
        QObject *dialog = m_root->findChild<QObject*>("VersionDownloadDialog");
        if (total == 0)
        {
            dialog->setProperty("progress", (float)0);
        }
        else
        {
            dialog->setProperty("progress", (float)(current / total));
        }
    }

private:
    CurlThread *workerThread;
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
    if(!home.exists(".regoth"))
    {
        home.mkdir(".regoth");
    }

    if(!home.cd(".regoth"))
    {
        throw std::runtime_error("Cannot access .regoth directory");
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
    home.cd(".regoth");

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
