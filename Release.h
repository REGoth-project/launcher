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

#ifndef RELEASE_H
#define RELEASE_H

#include <string>
#include <nlohmann/json.hpp>

struct Release
{
    std::string Name;
    std::string Url;
    std::string DownloadUrl;
    std::string Tag;
    bool Prerelease;
    std::string ReleaseDate;

    nlohmann::json serialize() const
    {
        nlohmann::json j;
        j["name"] = Name;
        j["url"] = Url;
        j["download_url"] = DownloadUrl;
        j["tag"] = Tag;
        j["prerelease"] = Prerelease;
        j["release_date"] = ReleaseDate;

        return j;
    }

    static Release deserialize(const nlohmann::json& j)
    {
        Release r;
        r.Name = j["name"].get<std::string>();
        r.Url = j["url"].get<std::string>();
        r.DownloadUrl = j["download_url"].get<std::string>();
        r.Tag = j["tag"].get<std::string>();
        r.Prerelease = j["prerelease"].get<bool>();
        r.ReleaseDate = j["release_date"].get<std::string>();

        return r;
    }

};

#endif // RELEASE_H
