#pragma once

#include "DomainEnums.h"
#include "Types.h"

#include <cstdint>

namespace practic1
{
    struct Track
    {
        Id id;
        char title[kTrackTitleLength];
        char file_path[kFilePathLength];
        std::uint16_t genre;
        DurationSeconds duration;
        Timestamp upload_date;
        Id author_id;
        float average_rating;
        std::uint32_t ratings_count;
        TrackStatus status;
        std::uint16_t bpm;
    };
}
