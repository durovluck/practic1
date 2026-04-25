#include "AdminService.h"

#include <cstddef>

namespace
{
    constexpr std::size_t kMaxUsersInAdminService = 1024;
    constexpr std::size_t kMaxTracksInAdminService = 2048;
    constexpr std::size_t kMaxRatingsInAdminService = 8192;
}

namespace practic1
{
    AdminService::AdminService(
        const UserStorage& user_storage,
        const TrackStorage& track_storage,
        const RatingStorage& rating_storage)
        : user_storage_(user_storage),
          track_storage_(track_storage),
          rating_storage_(rating_storage)
    {
    }

    bool AdminService::IsAdmin(Id user_id) const
    {
        if (user_id <= 0)
        {
            return false;
        }

        User user{};
        if (!user_storage_.FindById(user_id, user))
        {
            return false;
        }

        return user.rank == UserRank::Admin && user.is_blocked == 0;
    }

    bool AdminService::GetAllUsers(Id admin_id, User* out_users, std::size_t capacity, std::size_t& out_count) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        return user_storage_.ReadAll(out_users, capacity, out_count);
    }

    bool AdminService::SetUserBlocked(Id admin_id, Id target_user_id, std::uint8_t is_blocked) const
    {
        if (!IsAdmin(admin_id) || target_user_id <= 0 || (is_blocked != 0 && is_blocked != 1))
        {
            return false;
        }

        User target{};
        if (!user_storage_.FindById(target_user_id, target))
        {
            return false;
        }

        target.is_blocked = is_blocked;
        return user_storage_.UpdateById(target_user_id, target);
    }

    bool AdminService::GetAllTracks(Id admin_id, Track* out_tracks, std::size_t capacity, std::size_t& out_count) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        return track_storage_.ReadAll(out_tracks, capacity, out_count);
    }

    bool AdminService::SetTrackStatus(Id admin_id, Id track_id, TrackStatus status) const
    {
        if (!IsAdmin(admin_id) || track_id <= 0)
        {
            return false;
        }

        Track track{};
        if (!track_storage_.FindById(track_id, track))
        {
            return false;
        }

        track.status = status;
        return track_storage_.UpdateById(track_id, track);
    }

    bool AdminService::GetPlatformStats(Id admin_id, PlatformStats& out_stats) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        static User users[kMaxUsersInAdminService]{};
        std::size_t user_count = 0;
        if (!user_storage_.ReadAll(users, kMaxUsersInAdminService, user_count))
        {
            return false;
        }

        static Track tracks[kMaxTracksInAdminService]{};
        std::size_t track_count = 0;
        if (!track_storage_.ReadAll(tracks, kMaxTracksInAdminService, track_count))
        {
            return false;
        }

        static Rating ratings[kMaxRatingsInAdminService]{};
        std::size_t rating_count = 0;
        if (!rating_storage_.ReadAll(ratings, kMaxRatingsInAdminService, rating_count))
        {
            return false;
        }

        out_stats = PlatformStats{};

        out_stats.users_total = static_cast<std::uint32_t>(user_count);
        out_stats.tracks_total = static_cast<std::uint32_t>(track_count);
        out_stats.ratings_total = static_cast<std::uint32_t>(rating_count);

        for (std::size_t i = 0; i < user_count; ++i)
        {
            if (users[i].rank == UserRank::Admin)
            {
                ++out_stats.admins_total;
            }

            if (users[i].is_blocked != 0)
            {
                ++out_stats.blocked_users_total;
            }
        }

        for (std::size_t i = 0; i < track_count; ++i)
        {
            if (tracks[i].status == TrackStatus::Active)
            {
                ++out_stats.active_tracks_total;
            }
            else if (tracks[i].status == TrackStatus::Blocked)
            {
                ++out_stats.blocked_tracks_total;
            }
        }

        std::uint64_t sum = 0;
        for (std::size_t i = 0; i < rating_count; ++i)
        {
            sum += ratings[i].value;
        }

        out_stats.average_rating_overall = rating_count == 0
            ? 0.0f
            : static_cast<float>(sum) / static_cast<float>(rating_count);

        return true;
    }
}
