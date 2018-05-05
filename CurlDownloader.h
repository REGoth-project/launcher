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

#ifndef CURLDOWNLOADER_H
#define CURLDOWNLOADER_H

#include <string>
#include <functional>
#include <vector>
#include <cstdint>

class CurlDownloader
{
public:
    CurlDownloader(const std::string& url, std::function<bool(double, double)> progressCb = nullptr);
    ~CurlDownloader();

    std::vector<std::uint8_t>& get();
    const std::string& getError() const { return m_error; };

private:
    std::function<bool(double, double)> m_progress;
    void *m_curl;
    std::vector<std::uint8_t> m_buf;
    std::string m_error;
};

#endif
