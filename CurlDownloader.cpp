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

#include <string.h>
#include <curl/curl.h>
#include "CurlDownloader.h"

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    std::vector<std::uint8_t> *mem = (std::vector<std::uint8_t> *)userp;
    std::uint8_t *data = (std::uint8_t *)contents;

    for(size_t i = 0; i < realsize; i++)
    {
        mem->push_back(data[i]);
    }
 
    return realsize;
}

static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    std::function<bool(double,double)> *cb = (std::function<bool(double,double)>*)clientp;
    return (*cb)(dltotal, dlnow) ? 0 : 1;
}

CurlDownloader::CurlDownloader(const std::string& url, std::function<bool(double, double)> cb)
    : m_progress(cb)
{
    CURL *curl = curl_easy_init();
    m_curl = curl;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}

CurlDownloader::~CurlDownloader()
{
    curl_easy_cleanup((CURL *)m_curl);
}

std::vector<std::uint8_t>& CurlDownloader::get()
{
    CURL *curl = (CURL *)m_curl;
    char errbuf[CURL_ERROR_SIZE];
    memset(errbuf, 0, CURL_ERROR_SIZE);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&m_buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if(m_progress != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void *)&m_progress);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    }
 
    /* some servers don't like requests that are made without a user-agent
        field, so we provide one */ 
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    CURLcode res = curl_easy_perform(curl);

    m_error = std::string(errbuf);

    if (res != CURLE_OK)
    {
        m_buf.clear();
    }

    return m_buf;
}
