#pragma once

#include "Track.h"
#include "Types.h"

#include <cstddef>

namespace practic1
{
    class TrackStorage
    {
    public:
        explicit TrackStorage(const char* file_path = "tracks.dat");

        bool Add(const Track& track) const;
        bool ReadAll(Track* tracks, std::size_t capacity, std::size_t& out_count) const;
        bool FindById(Id id, Track& out_track) const;
        bool UpdateById(Id id, const Track& updated_track) const;
        bool DeleteById(Id id) const;

    private:
        char file_path_[kFilePathLength];
    };
}
