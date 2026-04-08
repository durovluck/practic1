#pragma once

#include "Types.h"

namespace practic1
{
    struct Rating
    {
        Id id;
        Id track_id;
        Id user_id;
        RatingValue value;
        Timestamp created_at;
    };
}
