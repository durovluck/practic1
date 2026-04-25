#pragma once

#include "Types.h"
#include "User.h"
#include "UserStorage.h"

namespace practic1
{
    class AuthService
    {
    public:
        explicit AuthService(const UserStorage& user_storage);

        bool RegisterUser(
            const char* username,
            const char* email,
            const char* plain_password,
            Timestamp created_at,
            User& out_user) const;

        bool LoginByEmail(const char* email, const char* plain_password, User& out_user) const;

    private:
        const UserStorage& user_storage_;
    };
}
