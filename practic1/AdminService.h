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

        bool GetAllTracks(Id admin_id, Track* out_tracks, std::size_t capacity, std::size_t& out_count) const;
        bool SetTrackStatus(Id admin_id, Id track_id, TrackStatus status) const;

        bool GetPlatformStats(Id admin_id, PlatformStats& out_stats) const;

    private:
        const UserStorage& user_storage_;
        const TrackStorage& track_storage_;
        const RatingStorage& rating_storage_;
    };
}
