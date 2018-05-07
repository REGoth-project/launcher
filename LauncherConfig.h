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

#ifndef LAUNCHERCONFIG_H
#define LAUNCHERCONFIG_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Release.h"
#include "Installation.h"

class LauncherConfig
{
public:
    std::vector<Release>& getReleases() { return m_releases; }
    const std::string& getDefaultRelease() const { return m_defaultRelease; }
    void setDefaultRelease(const std::string& tagName) { m_defaultRelease = tagName; }

    bool getShowPrereleases() const { return m_showPrereleases; }
    void setShowPrereleases(bool showPrereleases) { m_showPrereleases = showPrereleases; }

    const std::string& getReleasesEndpoint() const { return m_releasesEndpoint; }
    void setReleasesEndpoint(const std::string& endpoint) { m_releasesEndpoint = endpoint; }

    std::vector<Installation>& getGothicInstallations() { return m_gothicInstallations; }

    bool getCheckReleasesOnStartup() const { return m_checkReleasesOnStartup; }
    void setCheckReleasesOnStartup(bool check) { m_checkReleasesOnStartup = check; }

    nlohmann::json serialize() const;

    static LauncherConfig deserialize(const nlohmann::json& data);
private:
    std::vector<Release> m_releases;
    std::vector<Installation> m_gothicInstallations;
    std::string m_defaultRelease;
    bool m_showPrereleases;
    bool m_checkReleasesOnStartup;
    std::string m_releasesEndpoint;
};

#endif // LAUNCHERCONFIG_H
