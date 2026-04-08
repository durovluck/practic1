#pragma once

#include <cstdint>

namespace practic1
{
    enum class UserRank : std::uint8_t
    {
        User = 1,
        Admin = 2
    };

    enum class TrackStatus : std::uint8_t
    {
        Active = 1,
        Blocked = 2
    };
}
