#include "TrackService.h"

#include "DomainEnums.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>

namespace
{
    constexpr std::size_t kMaxTracksInService = 512;

    bool IsNullOrEmpty(const char* text)
    {
        return text == nullptr || text[0] == '\0';
    }

    bool CopyText(const char* source, char* destination, std::size_t destination_size)
    {
        if (source == nullptr || destination == nullptr || destination_size == 0)
        {
            return false;
        }

        std::size_t i = 0;
        while (source[i] != '\0' && i + 1 < destination_size)
        {
            destination[i] = source[i];
            ++i;
        }

        destination[i] = '\0';
        return source[i] == '\0';
    }

    void EnsureRandomSeed()
    {
        static bool seeded = false;
        if (!seeded)
        {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            seeded = true;
        }
    }
}

namespace practic1
{
    TrackService::TrackService(const TrackStorage& track_storage)
        : track_storage_(track_storage)
    {
    }

    bool TrackService::UploadTrack(
        Id author_id,
        const char* title,
        const char* file_path,
        std::uint16_t genre,
        DurationSeconds duration,
        std::uint16_t bpm,
        Timestamp upload_date,
        Track& out_track) const
    {
        if (author_id <= 0 || IsNullOrEmpty(title) || IsNullOrEmpty(file_path))
        {
            return false;
        }

        if (duration == 0 || bpm == 0)
        {
            return false;
        }

        Track to_create{};
        if (!CopyText(title, to_create.title, sizeof(to_create.title)))
        {
            return false;
        }

        if (!CopyText(file_path, to_create.file_path, sizeof(to_create.file_path)))
        {
            return false;
        }

        to_create.genre = genre;
        to_create.duration = duration;
        to_create.upload_date = upload_date;
        to_create.author_id = author_id;
        to_create.average_rating = 0.0f;
        to_create.ratings_count = 0;
        to_create.status = TrackStatus::Active;
        to_create.bpm = bpm;

        if (!track_storage_.Add(to_create))
        {
            return false;
        }

        Track tracks[kMaxTracksInService]{};
        std::size_t track_count = 0;
        if (!track_storage_.ReadAll(tracks, kMaxTracksInService, track_count))
        {
            return false;
        }

        for (std::size_t i = track_count; i > 0; --i)
        {
            const Track& current = tracks[i - 1];
            if (current.author_id == author_id &&
                current.upload_date == upload_date &&
                std::strcmp(current.title, title) == 0 &&
                std::strcmp(current.file_path, file_path) == 0)
            {
                out_track = current;
                return true;
            }
        }

        return false;
    }

    bool TrackService::GetRandomTrackForUser(Id viewer_user_id, Track& out_track) const
    {
        if (viewer_user_id <= 0)
        {
            return false;
        }

        Track tracks[kMaxTracksInService]{};
        std::size_t track_count = 0;
        if (!track_storage_.ReadAll(tracks, kMaxTracksInService, track_count))
        {
            return false;
        }

        std::size_t active_count = 0;
        for (std::size_t i = 0; i < track_count; ++i)
        {
            if (tracks[i].status == TrackStatus::Active)
            {
                ++active_count;
            }
        }

        if (active_count == 0)
        {
            return false;
        }

        EnsureRandomSeed();
        const std::size_t target = static_cast<std::size_t>(std::rand()) % active_count;

        std::size_t cursor = 0;
        for (std::size_t i = 0; i < track_count; ++i)
        {
            if (tracks[i].status != TrackStatus::Active)
            {
                continue;
            }

            if (cursor == target)
            {
                out_track = tracks[i];
                return true;
            }

            ++cursor;
        }

        return false;
    }

    bool TrackService::GetNextRandomTrack(Id viewer_user_id, Id current_track_id, Track& out_track) const
    {
        if (viewer_user_id <= 0)
        {
            return false;
        }

        Track tracks[kMaxTracksInService]{};
        std::size_t track_count = 0;
        if (!track_storage_.ReadAll(tracks, kMaxTracksInService, track_count))
        {
            return false;
        }

        std::size_t eligible_count = 0;
        for (std::size_t i = 0; i < track_count; ++i)
        {
            if (tracks[i].status == TrackStatus::Active && tracks[i].id != current_track_id)
            {
                ++eligible_count;
            }
        }

        if (eligible_count == 0)
        {
            return false;
        }

        EnsureRandomSeed();
        const std::size_t target = static_cast<std::size_t>(std::rand()) % eligible_count;

        std::size_t cursor = 0;
        for (std::size_t i = 0; i < track_count; ++i)
        {
            if (tracks[i].status != TrackStatus::Active || tracks[i].id == current_track_id)
            {
                continue;
            }

            if (cursor == target)
            {
                out_track = tracks[i];
                return true;
            }

            ++cursor;
        }

        return false;
    }

    bool TrackService::GetTracksByAuthor(Id author_id, Track* out_tracks, std::size_t capacity, std::size_t& out_count) const
    {
        out_count = 0;
        if (author_id <= 0 || (out_tracks == nullptr && capacity > 0))
        {
            return false;
        }

        Track tracks[kMaxTracksInService]{};
        std::size_t track_count = 0;
        if (!track_storage_.ReadAll(tracks, kMaxTracksInService, track_count))
        {
            return false;
        }

        for (std::size_t i = 0; i < track_count; ++i)
        {
            if (tracks[i].author_id != author_id)
            {
                continue;
            }

            if (out_count >= capacity)
            {
                return false;
            }

            out_tracks[out_count] = tracks[i];
            ++out_count;
        }

        return true;
    }
}
