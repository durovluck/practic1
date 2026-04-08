#pragma once

#include "Types.h"
#include "User.h"

#include <cstddef>

namespace practic1
{
    class UserStorage
    {
    public:
        explicit UserStorage(const char* file_path = "users.dat");

        bool Add(const User& user) const;
        bool ReadAll(User* users, std::size_t capacity, std::size_t& out_count) const;
        bool FindById(Id id, User& out_user) const;
        bool VerifyCredentials(Id id, const char* plain_password) const;
        bool UpdateById(Id id, const User& updated_user) const;
        bool DeleteById(Id id) const;

    private:
        char file_path_[kFilePathLength];
    };
}
