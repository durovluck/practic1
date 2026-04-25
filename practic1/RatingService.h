#pragma once

#include "RatingStorage.h"
#include "TrackStorage.h"
#include "Types.h"
#include "UserStorage.h"

namespace practic1
{
    class RatingService
    {
    public:
        RatingService(
            const UserStorage& user_storage,
            const TrackStorage& track_storage,
            const RatingStorage& rating_storage);

        bool RateTrack(Id user_id, Id track_id, RatingValue value, Timestamp created_at, bool& out_is_new_rating) const;

    private:
        const UserStorage& user_storage_;
        const TrackStorage& track_storage_;
        const RatingStorage& rating_storage_;
    };
}
