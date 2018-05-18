/*  Copyright (C) 2018 The REGoth Team
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <QNetworkReply>
#include <QDebug>
#include "ReleaseFetcher.h"
#include "Release.h"

using json = nlohmann::json;

ReleaseFetcher::ReleaseFetcher(const std::string& url)
    : m_url(QUrl(url.c_str()))
{
    m_nam = std::make_unique<QNetworkAccessManager>();

    QObject::connect(m_nam.get(), &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error())
        {
            versionFetched({});
            return;
        }

        json data = json::parse(reply->readAll().toStdString());
        json compatibleRelease;
        json compatibleAsset;
        for (const json& release : data) {
            for (const json& asset : release["assets"]) {
                std::string assetName = asset["name"].get<std::string>();
                bool good = false;
#if defined (Q_OS_WIN32)
                good = assetName.find("linux") == std::string::npos &&
                    assetName.find("android") == std::string::npos &&
                    assetName.find("osx") == std::string::npos;
#elif defined (Q_OS_MACOS)
                good = assetName.find("osx") != std::string::npos;
#else
                good = assetName.find("linux") != std::string::npos;
#endif

                if (good) {
                    compatibleAsset = asset;
                    compatibleRelease = release;
                    goto foundRelease;
                }
            }
        }

        versionFetched({});
        return;

    foundRelease:
        Release rel;
        rel.Name = compatibleRelease["name"].get<std::string>();
        rel.Tag = compatibleRelease["tag_name"].get<std::string>();
        rel.Url = compatibleRelease["url"].get<std::string>();
        rel.ReleaseDate = compatibleRelease["published_at"].get<std::string>();
        rel.Prerelease = compatibleRelease["prerelease"].get<bool>();
        rel.DownloadUrl = compatibleAsset["browser_download_url"].get<std::string>();
        versionFetched(rel);
    });
}

void ReleaseFetcher::fetch()
{
    QNetworkRequest request;
    request.setUrl(m_url);
    m_nam->get(request);
}
