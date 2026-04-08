#include "RatingStorage.h"

#include "StorageCommon.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>

namespace
{
    constexpr practic1::Id kRatingIdPrefix = 3;
    constexpr practic1::Id kIdBase = 100000000;
    constexpr practic1::Id kRandomAttempts = 256;

    void EnsureRandomSeed()
    {
        static bool seeded = false;
        if (!seeded)
        {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            seeded = true;
        }
    }

    std::uint32_t RandomSuffix8()
    {
        const std::uint32_t r1 = static_cast<std::uint32_t>(std::rand());
        const std::uint32_t r2 = static_cast<std::uint32_t>(std::rand());
        const std::uint32_t mixed = (r1 << 16) ^ r2;
        return mixed % static_cast<std::uint32_t>(kIdBase);
    }

    bool RatingIdExistsInFile(const char* file_path, practic1::Id id)
    {
        FILE* file = practic1::storage_common::OpenFile(file_path, "rb");
        if (file == nullptr)
        {
            return false;
        }

        practic1::Rating current{};
        while (std::fread(&current, sizeof(practic1::Rating), 1, file) == 1)
        {
            if (current.id == id)
            {
                std::fclose(file);
                return true;
            }
        }

        std::fclose(file);
        return false;
    }

    bool GenerateUniqueRatingId(const char* file_path, practic1::Id& out_id)
    {
        EnsureRandomSeed();
        const practic1::Id prefix_value = kRatingIdPrefix * kIdBase;

        for (practic1::Id i = 0; i < kRandomAttempts; ++i)
        {
            const practic1::Id candidate = prefix_value + static_cast<practic1::Id>(RandomSuffix8());
            if (!RatingIdExistsInFile(file_path, candidate))
            {
                out_id = candidate;
                return true;
            }
        }

        for (practic1::Id suffix = 0; suffix < kIdBase; ++suffix)
        {
            const practic1::Id candidate = prefix_value + suffix;
            if (!RatingIdExistsInFile(file_path, candidate))
            {
                out_id = candidate;
                return true;
            }
        }

        return false;
    }
}

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
        Rating record = rating;
        if (!GenerateUniqueRatingId(file_path_, record.id))
        {
            return false;
        }

        FILE* file = storage_common::OpenFile(file_path_, "ab");
        if (file == nullptr)
        {
            return false;
        }

        const bool write_ok = std::fwrite(&record, sizeof(Rating), 1, file) == 1;
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

        FILE* file = storage_common::OpenFile(file_path_, "rb");
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
        FILE* file = storage_common::OpenFile(file_path_, "rb");
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
        FILE* source = storage_common::OpenFile(file_path_, "rb");
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

        FILE* temp = storage_common::OpenFile(temp_path, "wb");
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
        FILE* source = storage_common::OpenFile(file_path_, "rb");
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

        FILE* temp = storage_common::OpenFile(temp_path, "wb");
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
