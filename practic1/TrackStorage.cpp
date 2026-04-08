#include "TrackStorage.h"

#include "StorageCommon.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>

namespace
{
    constexpr practic1::Id kTrackIdPrefix = 2;
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

    bool TrackIdExistsInFile(const char* file_path, practic1::Id id)
    {
        FILE* file = practic1::storage_common::OpenFile(file_path, "rb");
        if (file == nullptr)
        {
            return false;
        }

        practic1::Track current{};
        while (std::fread(&current, sizeof(practic1::Track), 1, file) == 1)
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

    bool GenerateUniqueTrackId(const char* file_path, practic1::Id& out_id)
    {
        EnsureRandomSeed();
        const practic1::Id prefix_value = kTrackIdPrefix * kIdBase;

        for (practic1::Id i = 0; i < kRandomAttempts; ++i)
        {
            const practic1::Id candidate = prefix_value + static_cast<practic1::Id>(RandomSuffix8());
            if (!TrackIdExistsInFile(file_path, candidate))
            {
                out_id = candidate;
                return true;
            }
        }

        for (practic1::Id suffix = 0; suffix < kIdBase; ++suffix)
        {
            const practic1::Id candidate = prefix_value + suffix;
            if (!TrackIdExistsInFile(file_path, candidate))
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
    TrackStorage::TrackStorage(const char* file_path)
    {
        if (!storage_common::CopyPath(file_path, file_path_, sizeof(file_path_)))
        {
            storage_common::CopyPath("tracks.dat", file_path_, sizeof(file_path_));
        }
    }

    bool TrackStorage::Add(const Track& track) const
    {
        Track record = track;
        if (!GenerateUniqueTrackId(file_path_, record.id))
        {
            return false;
        }

        FILE* file = storage_common::OpenFile(file_path_, "ab");
        if (file == nullptr)
        {
            return false;
        }

        const bool write_ok = std::fwrite(&record, sizeof(Track), 1, file) == 1;
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

        FILE* file = storage_common::OpenFile(file_path_, "rb");
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
        FILE* file = storage_common::OpenFile(file_path_, "rb");
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
