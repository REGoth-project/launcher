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

#include <stdexcept>
#include "LauncherConfig.h"

using json = nlohmann::json;

json LauncherConfig::serialize() const
{
    json j;
    j["default_release"] = m_defaultRelease;
    j["show_prereleases"] = m_showPrereleases;
    j["releases_endpoint"] = m_releasesEndpoint;
    j["check_releases_startup"] = m_checkReleasesOnStartup;

    j["releases"] = {};
    for(const auto& release : m_releases)
    {
        j["releases"].push_back(release.serialize());
    }

    j["gothic_installations"] = {};
    for(const auto& inst : m_gothicInstallations)
    {
        j["gothic_installations"].push_back(inst.serialize());
    }

    return j;
}

LauncherConfig LauncherConfig::deserialize(const json& j)
{
    LauncherConfig cfg;
    cfg.setReleasesEndpoint(j["releases_endpoint"].get<std::string>());
    cfg.setShowPrereleases(j["show_prereleases"].get<bool>());
    cfg.setShowPrereleases(j["check_releases_startup"].get<bool>());

    for(const auto& release : j["releases"])
    {
        cfg.m_releases.push_back(Release::deserialize(release));
    }

    for(const auto& inst : j["gothic_installations"])
    {
        cfg.m_gothicInstallations.push_back(Installation::deserialize(inst));
    }

    cfg.setDefaultRelease(j["default_release"].get<std::string>());

    return cfg;
}
