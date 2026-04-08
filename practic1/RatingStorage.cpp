#include "RatingStorage.h"

#include "StorageCommon.h"

#include <cstdio>

namespace practic1
{
    RatingStorage::RatingStorage(const char* file_path)
    {
        if (!storage_common::CopyPath(file_path, file_path_, sizeof(file_path_)))
        {
            storage_common::CopyPath("ratings.dat", file_path_, sizeof(file_path_));
        }
    }

    bool RatingStorage::Add(const Rating& rating) const
    {
        FILE* file = std::fopen(file_path_, "ab");
        if (file == nullptr)
        {
            return false;
        }

        const bool write_ok = std::fwrite(&rating, sizeof(Rating), 1, file) == 1;
        std::fclose(file);
        return write_ok;
    }

    bool RatingStorage::ReadAll(Rating* ratings, std::size_t capacity, std::size_t& out_count) const
    {
        out_count = 0;
        if (ratings == nullptr && capacity > 0)
        {
            return false;
        }

        FILE* file = std::fopen(file_path_, "rb");
        if (file == nullptr)
        {
            return true;
        }

        Rating current{};
        while (std::fread(&current, sizeof(Rating), 1, file) == 1)
        {
            if (out_count >= capacity)
            {
                std::fclose(file);
                return false;
            }

            ratings[out_count] = current;
            ++out_count;
        }

        const bool reached_eof = std::feof(file) != 0;
        std::fclose(file);
        return reached_eof;
    }

    bool RatingStorage::FindById(Id id, Rating& out_rating) const
    {
        FILE* file = std::fopen(file_path_, "rb");
        if (file == nullptr)
        {
            return false;
        }

        Rating current{};
        while (std::fread(&current, sizeof(Rating), 1, file) == 1)
        {
            if (current.id == id)
            {
                out_rating = current;
                std::fclose(file);
                return true;
            }
        }

        std::fclose(file);
        return false;
    }

    bool RatingStorage::UpdateById(Id id, const Rating& updated_rating) const
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
        Rating current{};

        while (std::fread(&current, sizeof(Rating), 1, source) == 1)
        {
            const Rating* to_write = &current;
            if (current.id == id)
            {
                to_write = &updated_rating;
                found = true;
            }

            if (std::fwrite(to_write, sizeof(Rating), 1, temp) != 1)
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

    bool RatingStorage::DeleteById(Id id) const
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
        Rating current{};

        while (std::fread(&current, sizeof(Rating), 1, source) == 1)
        {
            if (current.id == id)
            {
                found = true;
                continue;
            }

            if (std::fwrite(&current, sizeof(Rating), 1, temp) != 1)
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
