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

#ifndef RELEASE_FETCHER_H
#define RELEASE_FETCHER_H

#include <string>
#include "Release.h"

class ReleaseFetcher
{
public:
    ReleaseFetcher(const std::string& url);
    bool getLatestRelease(Release *rel);

private:
    std::string m_url;
    
};

#endif // RELEASE_FETCHER_H
