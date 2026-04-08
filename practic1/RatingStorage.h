#pragma once

#include "Rating.h"
#include "Types.h"

#include <cstddef>

namespace practic1
{
    class RatingStorage
    {
    public:
        explicit RatingStorage(const char* file_path = "ratings.dat");

        bool Add(const Rating& rating) const;
        bool ReadAll(Rating* ratings, std::size_t capacity, std::size_t& out_count) const;
        bool FindById(Id id, Rating& out_rating) const;
        bool UpdateById(Id id, const Rating& updated_rating) const;
        bool DeleteById(Id id) const;

    private:
        char file_path_[kFilePathLength];
    };
}
