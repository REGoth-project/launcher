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
#include "ArchiveExtractor.h"
#include <archive.h>
#include <archive_entry.h>

static int copy_data(archive *ar, archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

static void extract(archive *a, const std::string& path)
{
    archive *ext;
    archive_entry *entry;
    int flags;
    int r;

    /* Select which attributes we want to restore. */
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            throw std::runtime_error(archive_error_string(a));
        if (r < ARCHIVE_WARN)
            throw std::runtime_error("Error while reading archive");
        std::string pathName = archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, (path + "/" + pathName).c_str());
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            throw std::runtime_error(archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                throw std::runtime_error(archive_error_string(ext));
            if (r < ARCHIVE_WARN)
                throw std::runtime_error("Error while reading archive");
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            throw std::runtime_error(archive_error_string(ext));
        if (r < ARCHIVE_WARN)
            throw std::runtime_error("Error while reading archive");
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}

void extractArchive(const std::string& filename, const std::string& path)
{
    int r;
    archive *a;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if ((r = archive_read_open_filename(a, filename.c_str(), 10240)))
        throw std::runtime_error("Cannot open archive");

    extract(a, path);
}


void extractArchive(const std::vector<std::uint8_t>& data, const std::string& path)
{
    int r;
    archive *a;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if ((r = archive_read_open_memory(a, data.data(), data.size())))
        throw std::runtime_error("Cannot open archive");

    extract(a, path);
}