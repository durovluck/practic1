#include "AdminService.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

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

    bool AdminService::CreateUser(Id admin_id, const User& new_user) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        if (new_user.username[0] == '\0' ||
            new_user.email[0] == '\0' ||
            new_user.password[0] == '\0')
        {
            return false;
        }

        if (new_user.rank != UserRank::User && new_user.rank != UserRank::Admin)
        {
            return false;
        }

        if ((new_user.is_blocked != 0 && new_user.is_blocked != 1) ||
            (new_user.is_verified != 0 && new_user.is_verified != 1))
        {
            return false;
        }

        User users[kMaxUsersInAdminService]{};
        std::size_t user_count = 0;
        if (!user_storage_.ReadAll(users, kMaxUsersInAdminService, user_count))
        {
            return false;
        }

        for (std::size_t i = 0; i < user_count; ++i)
        {
            if (std::strcmp(users[i].email, new_user.email) == 0 ||
                std::strcmp(users[i].username, new_user.username) == 0)
            {
                return false;
            }
        }

        User record = new_user;
        record.id = 0;
        return user_storage_.Add(record);
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

    bool AdminService::UpdateUser(Id admin_id, Id target_user_id, const User& updated_user) const
    {
        if (!IsAdmin(admin_id) || target_user_id <= 0)
        {
            return false;
        }

        if (updated_user.username[0] == '\0' ||
            updated_user.email[0] == '\0' ||
            updated_user.password[0] == '\0')
        {
            return false;
        }

        if (updated_user.rank != UserRank::User && updated_user.rank != UserRank::Admin)
        {
            return false;
        }

        if ((updated_user.is_blocked != 0 && updated_user.is_blocked != 1) ||
            (updated_user.is_verified != 0 && updated_user.is_verified != 1))
        {
            return false;
        }

        User existing{};
        if (!user_storage_.FindById(target_user_id, existing))
        {
            return false;
        }

        User users[kMaxUsersInAdminService]{};
        std::size_t user_count = 0;
        if (!user_storage_.ReadAll(users, kMaxUsersInAdminService, user_count))
        {
            return false;
        }

        for (std::size_t i = 0; i < user_count; ++i)
        {
            if (users[i].id == target_user_id)
            {
                continue;
            }

            if (std::strcmp(users[i].email, updated_user.email) == 0 ||
                std::strcmp(users[i].username, updated_user.username) == 0)
            {
                return false;
            }
        }

        User record = updated_user;
        record.id = target_user_id;
        return user_storage_.UpdateById(target_user_id, record);
    }

    bool AdminService::DeleteUser(Id admin_id, Id target_user_id) const
    {
        if (!IsAdmin(admin_id) || target_user_id <= 0 || target_user_id == admin_id)
        {
            return false;
        }

        return user_storage_.DeleteById(target_user_id);
    }

    bool AdminService::GetAllTracks(Id admin_id, Track* out_tracks, std::size_t capacity, std::size_t& out_count) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        return track_storage_.ReadAll(out_tracks, capacity, out_count);
    }

    bool AdminService::CreateTrack(Id admin_id, const Track& new_track) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        if (new_track.title[0] == '\0' ||
            new_track.file_path[0] == '\0' ||
            new_track.duration == 0 ||
            new_track.bpm == 0 ||
            new_track.author_id <= 0)
        {
            return false;
        }

        if (new_track.average_rating < 0.0f || new_track.average_rating > 5.0f)
        {
            return false;
        }

        if (new_track.status != TrackStatus::Active && new_track.status != TrackStatus::Blocked)
        {
            return false;
        }

        User author{};
        if (!user_storage_.FindById(new_track.author_id, author))
        {
            return false;
        }

        Track record = new_track;
        record.id = 0;
        return track_storage_.Add(record);
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

    bool AdminService::UpdateTrack(Id admin_id, Id track_id, const Track& updated_track) const
    {
        if (!IsAdmin(admin_id) || track_id <= 0)
        {
            return false;
        }

        if (updated_track.title[0] == '\0' ||
            updated_track.file_path[0] == '\0' ||
            updated_track.duration == 0 ||
            updated_track.bpm == 0 ||
            updated_track.author_id <= 0)
        {
            return false;
        }

        if (updated_track.average_rating < 0.0f || updated_track.average_rating > 5.0f)
        {
            return false;
        }

        if (updated_track.status != TrackStatus::Active && updated_track.status != TrackStatus::Blocked)
        {
            return false;
        }

        Track existing{};
        if (!track_storage_.FindById(track_id, existing))
        {
            return false;
        }

        User author{};
        if (!user_storage_.FindById(updated_track.author_id, author))
        {
            return false;
        }

        Track record = updated_track;
        record.id = track_id;
        return track_storage_.UpdateById(track_id, record);
    }

    bool AdminService::DeleteTrack(Id admin_id, Id track_id) const
    {
        if (!IsAdmin(admin_id) || track_id <= 0)
        {
            return false;
        }

        return track_storage_.DeleteById(track_id);
    }

    bool AdminService::GetAllRatings(Id admin_id, Rating* out_ratings, std::size_t capacity, std::size_t& out_count) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        return rating_storage_.ReadAll(out_ratings, capacity, out_count);
    }

    bool AdminService::CreateRating(Id admin_id, const Rating& new_rating) const
    {
        if (!IsAdmin(admin_id))
        {
            return false;
        }

        if (new_rating.track_id <= 0 ||
            new_rating.user_id <= 0 ||
            new_rating.value > 5)
        {
            return false;
        }

        Track track{};
        if (!track_storage_.FindById(new_rating.track_id, track))
        {
            return false;
        }

        User user{};
        if (!user_storage_.FindById(new_rating.user_id, user))
        {
            return false;
        }

        Rating record = new_rating;
        record.id = 0;
        if (!rating_storage_.Add(record))
        {
            return false;
        }

        return RecalculateTrackStats(new_rating.track_id);
    }

    bool AdminService::UpdateRating(Id admin_id, Id rating_id, const Rating& updated_rating) const
    {
        if (!IsAdmin(admin_id) || rating_id <= 0)
        {
            return false;
        }

        if (updated_rating.track_id <= 0 ||
            updated_rating.user_id <= 0 ||
            updated_rating.value > 5)
        {
            return false;
        }

        Rating existing{};
        if (!rating_storage_.FindById(rating_id, existing))
        {
            return false;
        }

        Track track{};
        if (!track_storage_.FindById(updated_rating.track_id, track))
        {
            return false;
        }

        User user{};
        if (!user_storage_.FindById(updated_rating.user_id, user))
        {
            return false;
        }

        Rating record = updated_rating;
        record.id = rating_id;
        if (!rating_storage_.UpdateById(rating_id, record))
        {
            return false;
        }

        if (!RecalculateTrackStats(existing.track_id))
        {
            return false;
        }

        if (existing.track_id != record.track_id && !RecalculateTrackStats(record.track_id))
        {
            return false;
        }

        return true;
    }

    bool AdminService::DeleteRating(Id admin_id, Id rating_id) const
    {
        if (!IsAdmin(admin_id) || rating_id <= 0)
        {
            return false;
        }

        Rating existing{};
        if (!rating_storage_.FindById(rating_id, existing))
        {
            return false;
        }

        if (!rating_storage_.DeleteById(rating_id))
        {
            return false;
        }

        return RecalculateTrackStats(existing.track_id);
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

    bool AdminService::RecalculateTrackStats(Id track_id) const
    {
        Track track{};
        if (!track_storage_.FindById(track_id, track))
        {
            return false;
        }

        Rating ratings[kMaxRatingsInAdminService]{};
        std::size_t rating_count = 0;
        if (!rating_storage_.ReadAll(ratings, kMaxRatingsInAdminService, rating_count))
        {
            return false;
        }

        std::uint64_t sum = 0;
        std::uint32_t count_for_track = 0;
        for (std::size_t i = 0; i < rating_count; ++i)
        {
            if (ratings[i].track_id == track_id)
            {
                sum += ratings[i].value;
                ++count_for_track;
            }
        }

        track.ratings_count = count_for_track;
        track.average_rating = count_for_track == 0
            ? 0.0f
            : static_cast<float>(sum) / static_cast<float>(count_for_track);

        return track_storage_.UpdateById(track_id, track);
    }
}
