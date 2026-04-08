#pragma once

#include <cstddef>
#include <cstdint>

namespace practic1
{
    using Id = std::int32_t;
    using Timestamp = std::int64_t;
    using DurationSeconds = std::uint32_t;
    using RatingValue = std::uint8_t;

    constexpr std::size_t kUsernameLength = 32;
    constexpr std::size_t kEmailLength = 64;
    constexpr std::size_t kPasswordLength = 64;
    constexpr std::size_t kTrackTitleLength = 64;
    constexpr std::size_t kFilePathLength = 260;
}
