#include "TrackStorage.h"

#include "StorageCommon.h"

#include <cstdio>

namespace practic1
{
    TrackStorage::TrackStorage(const char* file_path)
    {
        if (!storage_common::CopyPath(file_path, file_path_, sizeof(file_path_)))
        {
            storage_common::CopyPath("tracks.dat", file_path_, sizeof(file_path_));
        }
    }

    bool TrackStorage::Add(const Track& track) const
    {
        FILE* file = std::fopen(file_path_, "ab");
        if (file == nullptr)
        {
            return false;
        }

        const bool write_ok = std::fwrite(&track, sizeof(Track), 1, file) == 1;
        std::fclose(file);
        return write_ok;
    }

    bool TrackStorage::ReadAll(Track* tracks, std::size_t capacity, std::size_t& out_count) const
    {
        out_count = 0;
        if (tracks == nullptr && capacity > 0)
        {
            return false;
        }

        FILE* file = std::fopen(file_path_, "rb");
        if (file == nullptr)
        {
            return true;
        }

        Track current{};
        while (std::fread(&current, sizeof(Track), 1, file) == 1)
        {
            if (out_count >= capacity)
            {
                std::fclose(file);
                return false;
            }

            tracks[out_count] = current;
            ++out_count;
        }

        const bool reached_eof = std::feof(file) != 0;
        std::fclose(file);
        return reached_eof;
    }

    bool TrackStorage::FindById(Id id, Track& out_track) const
    {
        FILE* file = std::fopen(file_path_, "rb");
        if (file == nullptr)
        {
            return false;
        }

        Track current{};
        while (std::fread(&current, sizeof(Track), 1, file) == 1)
        {
            if (current.id == id)
            {
                out_track = current;
                std::fclose(file);
                return true;
            }
        }

        std::fclose(file);
        return false;
    }

    bool TrackStorage::UpdateById(Id id, const Track& updated_track) const
    {
        FILE* source = std::fopen(file_path_, "rb");
        if (source == nullptr)
        {
            return false;
        }

        char temp_path[kFilePathLength]{};
        if (!storage_common::BuildTempPath(file_path_, temp_path, sizeof(temp_path)))
        {
            std::fclose(source);
            return false;
        }

        FILE* temp = std::fopen(temp_path, "wb");
        if (temp == nullptr)
        {
            std::fclose(source);
            return false;
        }

        bool found = false;
        bool write_ok = true;
        Track current{};

        while (std::fread(&current, sizeof(Track), 1, source) == 1)
        {
            const Track* to_write = &current;
            if (current.id == id)
            {
                to_write = &updated_track;
                found = true;
            }

            if (std::fwrite(to_write, sizeof(Track), 1, temp) != 1)
            {
                write_ok = false;
                break;
            }
        }

        const bool read_ok = std::feof(source) != 0;
        std::fclose(source);
        std::fclose(temp);

        if (!write_ok || !read_ok || !found)
        {
            std::remove(temp_path);
            return false;
        }

        if (!storage_common::ReplaceFileWithTemp(file_path_, temp_path))
        {
            std::remove(temp_path);
            return false;
        }

        return true;
    }

    bool TrackStorage::DeleteById(Id id) const
    {
        FILE* source = std::fopen(file_path_, "rb");
        if (source == nullptr)
        {
            return false;
        }

        char temp_path[kFilePathLength]{};
        if (!storage_common::BuildTempPath(file_path_, temp_path, sizeof(temp_path)))
        {
            std::fclose(source);
            return false;
        }

        FILE* temp = std::fopen(temp_path, "wb");
        if (temp == nullptr)
        {
            std::fclose(source);
            return false;
        }

        bool found = false;
        bool write_ok = true;
        Track current{};

        while (std::fread(&current, sizeof(Track), 1, source) == 1)
        {
            if (current.id == id)
            {
                found = true;
                continue;
            }

            if (std::fwrite(&current, sizeof(Track), 1, temp) != 1)
            {
                write_ok = false;
                break;
            }
        }

        const bool read_ok = std::feof(source) != 0;
        std::fclose(source);
        std::fclose(temp);

        if (!write_ok || !read_ok || !found)
        {
            std::remove(temp_path);
            return false;
        }

        if (!storage_common::ReplaceFileWithTemp(file_path_, temp_path))
        {
            std::remove(temp_path);
            return false;
        }

        return true;
    }
}
