#include "AuthService.h"

#include "DomainEnums.h"
#include "PasswordCrypto.h"

#include <cstddef>
#include <cstring>

namespace
{
    constexpr std::size_t kMaxUsersInService = 1024;

    bool IsNullOrEmpty(const char* text)
    {
        return text == nullptr || text[0] == '\0';
    }

    bool CopyText(const char* source, char* destination, std::size_t destination_size)
    {
        if (source == nullptr || destination == nullptr || destination_size == 0)
        {
            return false;
        }

        std::size_t i = 0;
        while (source[i] != '\0' && i + 1 < destination_size)
        {
            destination[i] = source[i];
            ++i;
        }

        destination[i] = '\0';
        return source[i] == '\0';
    }

    bool IsEmailBasicValid(const char* email)
    {
        if (IsNullOrEmpty(email))
        {
            return false;
        }

        const char* at = std::strchr(email, '@');
        if (at == nullptr || at == email)
        {
            return false;
        }

        const char* dot_after_at = std::strchr(at + 1, '.');
        return dot_after_at != nullptr && dot_after_at[1] != '\0';
    }
}

namespace practic1
{
    AuthService::AuthService(const UserStorage& user_storage)
        : user_storage_(user_storage)
    {
    }

    bool AuthService::RegisterUser(
        const char* username,
        const char* email,
        const char* plain_password,
        Timestamp created_at,
        User& out_user) const
    {
        if (IsNullOrEmpty(username) || !IsEmailBasicValid(email) || IsNullOrEmpty(plain_password))
        {
            return false;
        }

        User users[kMaxUsersInService]{};
        std::size_t user_count = 0;
        if (!user_storage_.ReadAll(users, kMaxUsersInService, user_count))
        {
            return false;
        }

        for (std::size_t i = 0; i < user_count; ++i)
        {
            if (std::strcmp(users[i].email, email) == 0)
            {
                return false;
            }

            if (std::strcmp(users[i].username, username) == 0)
            {
                return false;
            }
        }

        User to_create{};
        if (!CopyText(username, to_create.username, sizeof(to_create.username)))
        {
            return false;
        }

        if (!CopyText(email, to_create.email, sizeof(to_create.email)))
        {
            return false;
        }

        if (!CopyText(plain_password, to_create.password, sizeof(to_create.password)))
        {
            return false;
        }

        to_create.rank = UserRank::User;
        to_create.created_at = created_at;
        to_create.is_blocked = 0;
        to_create.is_verified = 0;

        if (!user_storage_.Add(to_create))
        {
            return false;
        }

        if (!user_storage_.ReadAll(users, kMaxUsersInService, user_count))
        {
            return false;
        }

        for (std::size_t i = 0; i < user_count; ++i)
        {
            if (std::strcmp(users[i].email, email) == 0)
            {
                out_user = users[i];
                return true;
            }
        }

        return false;
    }

    bool AuthService::LoginByEmail(const char* email, const char* plain_password, User& out_user) const
    {
        if (!IsEmailBasicValid(email) || IsNullOrEmpty(plain_password))
        {
            return false;
        }

        User users[kMaxUsersInService]{};
        std::size_t user_count = 0;
        if (!user_storage_.ReadAll(users, kMaxUsersInService, user_count))
        {
            return false;
        }

        for (std::size_t i = 0; i < user_count; ++i)
        {
            if (std::strcmp(users[i].email, email) != 0)
            {
                continue;
            }

            if (users[i].is_blocked != 0)
            {
                return false;
            }

            if (!security::VerifyPassword(plain_password, users[i].password))
            {
                return false;
            }

            out_user = users[i];
            return true;
        }

        return false;
    }
}
