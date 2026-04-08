#include "RatingStorage.h"
#include "TrackStorage.h"
#include "UserStorage.h"

#include <cstdio>
#include <cstring>

namespace
{
    constexpr std::size_t kMaxRows = 32;
    constexpr practic1::Id kIdBase = 100000000;

    bool HasIdPrefix(practic1::Id id, practic1::Id prefix)
    {
        return id >= (prefix * kIdBase) && id < ((prefix + 1) * kIdBase);
    }

    void PrintUsers(practic1::UserStorage& storage)
    {
        practic1::User users[kMaxRows]{};
        std::size_t count = 0;

        if (!storage.ReadAll(users, kMaxRows, count))
        {
            std::printf("Users: read error\n");
            return;
        }

        std::printf("Users (%zu):\n", count);
        for (std::size_t i = 0; i < count; ++i)
        {
            const practic1::User& u = users[i];
            std::printf(
                "  id=%d username=%s email=%s rank=%u blocked=%u verified=%u created_at=%lld\n",
                static_cast<int>(u.id),
                u.username,
                u.email,
                static_cast<unsigned>(u.rank),
                static_cast<unsigned>(u.is_blocked),
                static_cast<unsigned>(u.is_verified),
                static_cast<long long>(u.created_at));
        }
    }

    void PrintTracks(practic1::TrackStorage& storage)
    {
        practic1::Track tracks[kMaxRows]{};
        std::size_t count = 0;

        if (!storage.ReadAll(tracks, kMaxRows, count))
        {
            std::printf("Tracks: read error\n");
            return;
        }

        std::printf("Tracks (%zu):\n", count);
        for (std::size_t i = 0; i < count; ++i)
        {
            const practic1::Track& t = tracks[i];
            std::printf(
                "  id=%d title=%s author_id=%d status=%u bpm=%u avg=%.2f ratings=%u file=%s\n",
                static_cast<int>(t.id),
                t.title,
                static_cast<int>(t.author_id),
                static_cast<unsigned>(t.status),
                static_cast<unsigned>(t.bpm),
                t.average_rating,
                static_cast<unsigned>(t.ratings_count),
                t.file_path);
        }
    }

    void PrintRatings(practic1::RatingStorage& storage)
    {
        practic1::Rating ratings[kMaxRows]{};
        std::size_t count = 0;

        if (!storage.ReadAll(ratings, kMaxRows, count))
        {
            std::printf("Ratings: read error\n");
            return;
        }

        std::printf("Ratings (%zu):\n", count);
        for (std::size_t i = 0; i < count; ++i)
        {
            const practic1::Rating& r = ratings[i];
            std::printf(
                "  id=%d track_id=%d user_id=%d value=%u created_at=%lld\n",
                static_cast<int>(r.id),
                static_cast<int>(r.track_id),
                static_cast<int>(r.user_id),
                static_cast<unsigned>(r.value),
                static_cast<long long>(r.created_at));
        }
    }

    bool SeedUsers(practic1::UserStorage& users)
    {
        practic1::User u1{};
        strcpy_s(u1.username, sizeof(u1.username), "alice");
        strcpy_s(u1.email, sizeof(u1.email), "alice@test.com");
        strcpy_s(u1.password, sizeof(u1.password), "pass1");
        u1.rank = practic1::UserRank::User;
        u1.created_at = 1710000000;
        u1.is_blocked = 0;
        u1.is_verified = 1;

        practic1::User u2{};
        strcpy_s(u2.username, sizeof(u2.username), "admin");
        strcpy_s(u2.email, sizeof(u2.email), "admin@test.com");
        strcpy_s(u2.password, sizeof(u2.password), "adminpass");
        u2.rank = practic1::UserRank::Admin;
        u2.created_at = 1710000100;
        u2.is_blocked = 0;
        u2.is_verified = 1;

        return users.Add(u1) && users.Add(u2);
    }

    bool SeedTracks(practic1::TrackStorage& tracks, practic1::Id author_id)
    {
        practic1::Track t1{};
        strcpy_s(t1.title, sizeof(t1.title), "Neon Nights");
        strcpy_s(t1.file_path, sizeof(t1.file_path), "audio/neon_nights.mp3");
        t1.genre = 1;
        t1.duration = 210;
        t1.upload_date = 1710000200;
        t1.author_id = author_id;
        t1.average_rating = 4.5f;
        t1.ratings_count = 2;
        t1.status = practic1::TrackStatus::Active;
        t1.bpm = 124;

        practic1::Track t2{};
        strcpy_s(t2.title, sizeof(t2.title), "Lo-Fi Rain");
        strcpy_s(t2.file_path, sizeof(t2.file_path), "audio/lofi_rain.mp3");
        t2.genre = 4;
        t2.duration = 180;
        t2.upload_date = 1710000300;
        t2.author_id = author_id;
        t2.average_rating = 3.0f;
        t2.ratings_count = 1;
        t2.status = practic1::TrackStatus::Active;
        t2.bpm = 90;

        return tracks.Add(t1) && tracks.Add(t2);
    }

    bool SeedRatings(
        practic1::RatingStorage& ratings,
        practic1::Id track_id_1,
        practic1::Id track_id_2,
        practic1::Id user_id_1,
        practic1::Id user_id_2)
    {
        practic1::Rating r1{};
        r1.track_id = track_id_1;
        r1.user_id = user_id_1;
        r1.value = 5;
        r1.created_at = 1710000400;

        practic1::Rating r2{};
        r2.track_id = track_id_1;
        r2.user_id = user_id_2;
        r2.value = 4;
        r2.created_at = 1710000500;

        practic1::Rating r3{};
        r3.track_id = track_id_2;
        r3.user_id = user_id_2;
        r3.value = 3;
        r3.created_at = 1710000600;

        return ratings.Add(r1) && ratings.Add(r2) && ratings.Add(r3);
    }
}

int main()
{
    std::remove("users_test.dat");
    std::remove("tracks_test.dat");
    std::remove("ratings_test.dat");

    practic1::UserStorage users("users_test.dat");
    practic1::TrackStorage tracks("tracks_test.dat");
    practic1::RatingStorage ratings("ratings_test.dat");

    if (!SeedUsers(users))
    {
        std::printf("Seed users error\n");
        return 1;
    }

    practic1::User user_rows[kMaxRows]{};
    std::size_t user_count = 0;
    if (!users.ReadAll(user_rows, kMaxRows, user_count) || user_count < 2)
    {
        std::printf("Read users error\n");
        return 2;
    }

    const practic1::Id user_id_1 = user_rows[0].id;
    const practic1::Id user_id_2 = user_rows[1].id;
    if (!HasIdPrefix(user_id_1, 1) || !HasIdPrefix(user_id_2, 1))
    {
        std::printf("User id prefix validation error\n");
        return 3;
    }

    if (!SeedTracks(tracks, user_id_1))
    {
        std::printf("Seed tracks error\n");
        return 4;
    }

    practic1::Track track_rows[kMaxRows]{};
    std::size_t track_count = 0;
    if (!tracks.ReadAll(track_rows, kMaxRows, track_count) || track_count < 2)
    {
        std::printf("Read tracks error\n");
        return 5;
    }

    const practic1::Id track_id_1 = track_rows[0].id;
    const practic1::Id track_id_2 = track_rows[1].id;
    if (!HasIdPrefix(track_id_1, 2) || !HasIdPrefix(track_id_2, 2))
    {
        std::printf("Track id prefix validation error\n");
        return 6;
    }

    if (!SeedRatings(ratings, track_id_1, track_id_2, user_id_1, user_id_2))
    {
        std::printf("Seed ratings error\n");
        return 7;
    }

    practic1::Rating rating_rows[kMaxRows]{};
    std::size_t rating_count = 0;
    if (!ratings.ReadAll(rating_rows, kMaxRows, rating_count) || rating_count < 3)
    {
        std::printf("Read ratings error\n");
        return 8;
    }

    if (!HasIdPrefix(rating_rows[0].id, 3) || !HasIdPrefix(rating_rows[1].id, 3) || !HasIdPrefix(rating_rows[2].id, 3))
    {
        std::printf("Rating id prefix validation error\n");
        return 9;
    }

    std::printf("=== DATABASE SNAPSHOT (AFTER SEED) ===\n");
    PrintUsers(users);
    PrintTracks(tracks);
    PrintRatings(ratings);

    practic1::User to_update{};
    if (!users.FindById(user_id_1, to_update))
    {
        std::printf("Find user error\n");
        return 10;
    }

    to_update.is_blocked = 1;
    if (!users.UpdateById(user_id_1, to_update))
    {
        std::printf("Update user error\n");
        return 11;
    }

    if (!ratings.DeleteById(rating_rows[2].id))
    {
        std::printf("Delete rating error\n");
        return 12;
    }

    std::printf("\n=== DATABASE SNAPSHOT (AFTER UPDATE + DELETE) ===\n");
    PrintUsers(users);
    PrintTracks(tracks);
    PrintRatings(ratings);

    return 0;
}
