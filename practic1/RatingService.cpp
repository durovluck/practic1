#include "RatingService.h"

#include "DomainEnums.h"

#include <cstddef>

namespace
{
    constexpr std::size_t kMaxRatingsInService = 4096;
}

namespace practic1
{
    RatingService::RatingService(
        const UserStorage& user_storage,
        const TrackStorage& track_storage,
        const RatingStorage& rating_storage)
        : user_storage_(user_storage),
          track_storage_(track_storage),
          rating_storage_(rating_storage)
    {
    }

    bool RatingService::RateTrack(
        Id user_id,
        Id track_id,
        RatingValue value,
        Timestamp created_at,
        bool& out_is_new_rating) const
    {
        out_is_new_rating = false;

        if (user_id <= 0 || track_id <= 0 || value > 5)
        {
            return false;
        }

        User user{};
        if (!user_storage_.FindById(user_id, user))
        {
            return false;
        }

        if (user.is_blocked != 0)
        {
            return false;
        }

        Track track{};
        if (!track_storage_.FindById(track_id, track))
        {
            return false;
        }

        if (track.status != TrackStatus::Active)
        {
            return false;
        }

        Rating ratings[kMaxRatingsInService]{};
        std::size_t rating_count = 0;
        if (!rating_storage_.ReadAll(ratings, kMaxRatingsInService, rating_count))
        {
            return false;
        }

        Id existing_rating_id = -1;
        for (std::size_t i = 0; i < rating_count; ++i)
        {
            if (ratings[i].track_id == track_id && ratings[i].user_id == user_id)
            {
                existing_rating_id = ratings[i].id;
                break;
            }
        }

        if (existing_rating_id > 0)
        {
            Rating updated{};
            if (!rating_storage_.FindById(existing_rating_id, updated))
            {
                return false;
            }

            updated.value = value;
            updated.created_at = created_at;
            if (!rating_storage_.UpdateById(existing_rating_id, updated))
            {
                return false;
            }
        }
        else
        {
            Rating new_rating{};
            new_rating.track_id = track_id;
            new_rating.user_id = user_id;
            new_rating.value = value;
            new_rating.created_at = created_at;
            if (!rating_storage_.Add(new_rating))
            {
                return false;
            }

            out_is_new_rating = true;
        }

        if (!rating_storage_.ReadAll(ratings, kMaxRatingsInService, rating_count))
        {
            return false;
        }

        std::uint64_t sum = 0;
        std::uint32_t count = 0;
        for (std::size_t i = 0; i < rating_count; ++i)
        {
            if (ratings[i].track_id != track_id)
            {
                continue;
            }

            sum += ratings[i].value;
            ++count;
        }

        track.ratings_count = count;
        track.average_rating = (count == 0) ? 0.0f : static_cast<float>(sum) / static_cast<float>(count);

        return track_storage_.UpdateById(track_id, track);
    }
}
