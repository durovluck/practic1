#pragma once

#include "DomainEnums.h"
#include "RatingStorage.h"
#include "TrackStorage.h"
#include "Types.h"
#include "UserStorage.h"

#include <cstddef>
#include <cstdint>

namespace practic1
{
    struct PlatformStats
    {
        std::uint32_t users_total;
        std::uint32_t admins_total;
        std::uint32_t blocked_users_total;
        std::uint32_t tracks_total;
        std::uint32_t active_tracks_total;
        std::uint32_t blocked_tracks_total;
        std::uint32_t ratings_total;
        float average_rating_overall;
    };

    class AdminService
    {
    public:
        AdminService(
            const UserStorage& user_storage,
            const TrackStorage& track_storage,
            const RatingStorage& rating_storage);

        bool IsAdmin(Id user_id) const;

        bool GetAllUsers(Id admin_id, User* out_users, std::size_t capacity, std::size_t& out_count) const;
        bool SetUserBlocked(Id admin_id, Id target_user_id, std::uint8_t is_blocked) const;
        bool UpdateUser(Id admin_id, Id target_user_id, const User& updated_user) const;

        bool GetAllTracks(Id admin_id, Track* out_tracks, std::size_t capacity, std::size_t& out_count) const;
        bool SetTrackStatus(Id admin_id, Id track_id, TrackStatus status) const;
        bool UpdateTrack(Id admin_id, Id track_id, const Track& updated_track) const;

        bool GetAllRatings(Id admin_id, Rating* out_ratings, std::size_t capacity, std::size_t& out_count) const;
        bool UpdateRating(Id admin_id, Id rating_id, const Rating& updated_rating) const;

        bool GetPlatformStats(Id admin_id, PlatformStats& out_stats) const;

    private:
        bool RecalculateTrackStats(Id track_id) const;

        const UserStorage& user_storage_;
        const TrackStorage& track_storage_;
        const RatingStorage& rating_storage_;
    };
}
