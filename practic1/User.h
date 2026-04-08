#pragma once

#include "DomainEnums.h"
#include "Types.h"

#include <cstdint>

namespace practic1
{
    struct User
    {
        Id id;
        char username[kUsernameLength];
        char email[kEmailLength];
        char password[kPasswordLength];
        UserRank rank;
        Timestamp created_at;
        std::uint8_t is_blocked;
        std::uint8_t is_verified;
    };
}
