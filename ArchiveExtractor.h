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

#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include <cstdint>
#include <vector>
#include <string>

void extractArchive(const std::string& fileName, const std::string& path);
void extractArchive(const std::vector<std::uint8_t>& data, const std::string& path);

#endif // ARCHIVEEXTRACTOR_H
