#pragma once

#include "Track.h"
#include "TrackStorage.h"
#include "Types.h"

#include <cstddef>

namespace practic1
{
    class TrackService
    {
    public:
        explicit TrackService(const TrackStorage& track_storage);

        bool UploadTrack(
            Id author_id,
            const char* title,
            const char* file_path,
            std::uint16_t genre,
            DurationSeconds duration,
            std::uint16_t bpm,
            Timestamp upload_date,
            Track& out_track) const;

        bool GetRandomTrackForUser(Id viewer_user_id, Track& out_track) const;
        bool GetNextRandomTrack(Id viewer_user_id, Id current_track_id, Track& out_track) const;
        bool GetTracksByAuthor(Id author_id, Track* out_tracks, std::size_t capacity, std::size_t& out_count) const;

    private:
        const TrackStorage& track_storage_;
    };
}
