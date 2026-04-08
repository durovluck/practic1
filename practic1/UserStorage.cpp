#include "UserStorage.h"

#include "PasswordCrypto.h"
#include "StorageCommon.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace
{
    constexpr practic1::Id kUserIdPrefix = 1;
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

    bool UserIdExistsInFile(const char* file_path, practic1::Id id)
    {
        FILE* file = practic1::storage_common::OpenFile(file_path, "rb");
        if (file == nullptr)
        {
            return false;
        }

        practic1::User current{};
        while (std::fread(&current, sizeof(practic1::User), 1, file) == 1)
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

    bool GenerateUniqueUserId(const char* file_path, practic1::Id& out_id)
    {
        EnsureRandomSeed();
        const practic1::Id prefix_value = kUserIdPrefix * kIdBase;

        for (practic1::Id i = 0; i < kRandomAttempts; ++i)
        {
            const practic1::Id candidate = prefix_value + static_cast<practic1::Id>(RandomSuffix8());
            if (!UserIdExistsInFile(file_path, candidate))
            {
                out_id = candidate;
                return true;
            }
        }

        for (practic1::Id suffix = 0; suffix < kIdBase; ++suffix)
        {
            const practic1::Id candidate = prefix_value + suffix;
            if (!UserIdExistsInFile(file_path, candidate))
            {
                out_id = candidate;
                return true;
            }
        }

        return false;
    }

    bool NormalizePasswordField(char* password, std::size_t password_size)
    {
        if (password == nullptr || password_size == 0)
        {
            return false;
        }

        if (practic1::security::IsPasswordHashed(password))
        {
            return true;
        }

        char hashed[practic1::kPasswordLength]{};
        if (!practic1::security::HashPassword(password, hashed, sizeof(hashed)))
        {
            return false;
        }

        std::memset(password, 0, password_size);
        std::memcpy(password, hashed, std::strlen(hashed) + 1);
        return true;
    }
}

namespace practic1
{
    UserStorage::UserStorage(const char* file_path)
    {
        if (!storage_common::CopyPath(file_path, file_path_, sizeof(file_path_)))
        {
            storage_common::CopyPath("users.dat", file_path_, sizeof(file_path_));
        }
    }

    bool UserStorage::Add(const User& user) const
    {
        User record = user;
        if (!GenerateUniqueUserId(file_path_, record.id))
        {
            return false;
        }

        if (!NormalizePasswordField(record.password, sizeof(record.password)))
        {
            return false;
        }

        FILE* file = storage_common::OpenFile(file_path_, "ab");
        if (file == nullptr)
        {
            return false;
        }

        const bool write_ok = std::fwrite(&record, sizeof(User), 1, file) == 1;
        std::fclose(file);
        return write_ok;
    }

    bool UserStorage::ReadAll(User* users, std::size_t capacity, std::size_t& out_count) const
    {
        out_count = 0;
        if (users == nullptr && capacity > 0)
        {
            return false;
        }

        FILE* file = storage_common::OpenFile(file_path_, "rb");
        if (file == nullptr)
        {
            return true;
        }

        User current{};
        while (std::fread(&current, sizeof(User), 1, file) == 1)
        {
            if (out_count >= capacity)
            {
                std::fclose(file);
                return false;
            }

            users[out_count] = current;
            ++out_count;
        }

        const bool reached_eof = std::feof(file) != 0;
        std::fclose(file);
        return reached_eof;
    }

    bool UserStorage::FindById(Id id, User& out_user) const
    {
        FILE* file = storage_common::OpenFile(file_path_, "rb");
        if (file == nullptr)
        {
            return false;
        }

        User current{};
        while (std::fread(&current, sizeof(User), 1, file) == 1)
        {
            if (current.id == id)
            {
                out_user = current;
                std::fclose(file);
                return true;
            }
        }

        std::fclose(file);
        return false;
    }

    bool UserStorage::VerifyCredentials(Id id, const char* plain_password) const
    {
        FILE* file = storage_common::OpenFile(file_path_, "rb");
        if (file == nullptr)
        {
            return false;
        }

        User current{};
        while (std::fread(&current, sizeof(User), 1, file) == 1)
        {
            if (current.id == id)
            {
                const bool verified = security::VerifyPassword(plain_password, current.password);
                std::fclose(file);
                return verified;
            }
        }

        std::fclose(file);
        return false;
    }

    bool UserStorage::UpdateById(Id id, const User& updated_user) const
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
        User current{};
        User normalized_update = updated_user;
        if (!NormalizePasswordField(normalized_update.password, sizeof(normalized_update.password)))
        {
            std::fclose(source);
            std::fclose(temp);
            std::remove(temp_path);
            return false;
        }

        while (std::fread(&current, sizeof(User), 1, source) == 1)
        {
            const User* to_write = &current;
            if (current.id == id)
            {
                to_write = &normalized_update;
                found = true;
            }

            if (std::fwrite(to_write, sizeof(User), 1, temp) != 1)
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

    bool UserStorage::DeleteById(Id id) const
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
        User current{};

        while (std::fread(&current, sizeof(User), 1, source) == 1)
        {
            if (current.id == id)
            {
                found = true;
                continue;
            }

            if (std::fwrite(&current, sizeof(User), 1, temp) != 1)
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
