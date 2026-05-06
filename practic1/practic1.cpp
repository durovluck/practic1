#include "AdminService.h"
#include "AuthService.h"
#include "RatingService.h"
#include "TrackService.h"
#include "TrackStorage.h"
#include "UserStorage.h"
#include "RatingStorage.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <ctime>
#include <windows.h>

namespace
{
    constexpr std::size_t kMaxUsersInDbConsole = 1024;
    constexpr std::size_t kMaxTracksInUi = 1024;
    constexpr std::size_t kMaxUsersInAdminUi = 1024;
    constexpr std::size_t kMaxTracksInAdminUi = 2048;
    constexpr std::size_t kMaxRatingsInAdminUi = 8192;

    bool ReadTextInput(const char* prompt, char* buffer, unsigned buffer_size);
    bool ReadIntInput(const char* prompt, int& out_value);
    bool ReadLongLongInput(const char* prompt, long long& out_value);
    bool ReadFloatInput(const char* prompt, float& out_value);

    void PrintUserAccount(const practic1::User& user)
    {
        std::printf("\n=== ACCOUNT DATA ===\n");
        std::printf("id: %d\n", static_cast<int>(user.id));
        std::printf("username: %s\n", user.username);
        std::printf("email: %s\n", user.email);
        std::printf("rank: %u\n", static_cast<unsigned>(user.rank));
        std::printf("is_blocked: %u\n", static_cast<unsigned>(user.is_blocked));
        std::printf("is_verified: %u\n", static_cast<unsigned>(user.is_verified));
        std::printf("created_at (unix): %lld\n", static_cast<long long>(user.created_at));
    }

    void PrintTrack(const practic1::Track& track)
    {
        std::printf("\n=== TRACK ===\n");
        std::printf("id: %d\n", static_cast<int>(track.id));
        std::printf("title: %s\n", track.title);
        std::printf("file_path: %s\n", track.file_path);
        std::printf("genre: %u\n", static_cast<unsigned>(track.genre));
        std::printf("duration: %u\n", static_cast<unsigned>(track.duration));
        std::printf("author_id: %d\n", static_cast<int>(track.author_id));
        std::printf("average_rating: %.2f\n", track.average_rating);
        std::printf("ratings_count: %u\n", static_cast<unsigned>(track.ratings_count));
        std::printf("status: %u\n", static_cast<unsigned>(track.status));
        std::printf("bpm: %u\n", static_cast<unsigned>(track.bpm));
    }

    void PrintRating(const practic1::Rating& rating)
    {
        std::printf("\n=== RATING ===\n");
        std::printf("id: %d\n", static_cast<int>(rating.id));
        std::printf("track_id: %d\n", static_cast<int>(rating.track_id));
        std::printf("user_id: %d\n", static_cast<int>(rating.user_id));
        std::printf("value: %u\n", static_cast<unsigned>(rating.value));
        std::printf("created_at (unix): %lld\n", static_cast<long long>(rating.created_at));
    }

    void PrintMainMenu(bool is_logged_in, const practic1::User& current_user, bool has_current_track)
    {
        std::printf("\n================ MENU ================\n");
        std::printf("1 - Register\n");
        std::printf("2 - Login\n");
        std::printf("3 - View my account\n");
        std::printf("4 - Upload track\n");
        std::printf("5 - Show random track\n");
        std::printf("6 - Show next random track\n");
        std::printf("7 - Rate current track (0..5)\n");
        std::printf("8 - My tracks stats\n");
        std::printf("9 - Open DB console\n");
        std::printf("10 - Logout\n");
        std::printf("11 - Admin panel\n");
        std::printf("0 - Exit\n");
        std::printf("--------------------------------------\n");

        if (is_logged_in)
        {
            std::printf("Session: logged in as %s (id=%d)\n", current_user.username, static_cast<int>(current_user.id));
        }
        else
        {
            std::printf("Session: guest\n");
        }

        std::printf("Current track selected: %s\n", has_current_track ? "yes" : "no");
    }

    bool IsDbConsoleMode(int argc, char* argv[])
    {
        return argc > 1 && argv != nullptr && std::strcmp(argv[1], "--db-console") == 0;
    }

    void PrintUsersDatabase()
    {
        practic1::UserStorage user_storage("users_app.dat");
        practic1::User users[kMaxUsersInDbConsole]{};
        std::size_t count = 0;

        if (!user_storage.ReadAll(users, kMaxUsersInDbConsole, count))
        {
            std::printf("Read error.\n");
            return;
        }

        std::printf("\n=== USERS DATABASE (users_app.dat) ===\n");
        std::printf("rows: %zu\n", count);
        for (std::size_t i = 0; i < count; ++i)
        {
            const practic1::User& u = users[i];
            std::printf(
                "id=%d username=%s email=%s password_hash=%s rank=%u blocked=%u verified=%u created_at=%lld\n",
                static_cast<int>(u.id),
                u.username,
                u.email,
                u.password,
                static_cast<unsigned>(u.rank),
                static_cast<unsigned>(u.is_blocked),
                static_cast<unsigned>(u.is_verified),
                static_cast<long long>(u.created_at));
        }
    }

    bool ClearUsersDatabase()
    {
        FILE* file = nullptr;
        if (fopen_s(&file, "users_app.dat", "rb") != 0 || file == nullptr)
        {
            return true;
        }

        std::fclose(file);
        return std::remove("users_app.dat") == 0;
    }

    int RunDatabaseConsole()
    {
        std::printf("=== DB CONSOLE ===\n");
        std::printf("Commands:\n");
        std::printf("1 - show database\n");
        std::printf("2 - clear database\n");
        std::printf("3 - exit db console\n");

        for (;;)
        {
            std::printf("\nDB command: ");
            int command = 0;
            if (!ReadIntInput("", command))
            {
                std::printf("Input error.\n");
                continue;
            }

            if (command == 1)
            {
                PrintUsersDatabase();
                continue;
            }

            if (command == 2)
            {
                if (ClearUsersDatabase())
                {
                    std::printf("Database cleared.\n");
                }
                else
                {
                    std::printf("Clear failed.\n");
                }
                continue;
            }

            if (command == 3)
            {
                return 0;
            }

            std::printf("Unknown command.\n");
        }
    }

    bool StartDatabaseConsoleProcess()
    {
        char exe_path[MAX_PATH]{};
        const DWORD path_len = GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
        if (path_len == 0 || path_len >= MAX_PATH)
        {
            return false;
        }

        char command_line[MAX_PATH + 64]{};
        if (sprintf_s(command_line, "\"%s\" --db-console", exe_path) <= 0)
        {
            return false;
        }

        STARTUPINFOA startup_info{};
        startup_info.cb = sizeof(startup_info);

        PROCESS_INFORMATION process_info{};
        const BOOL created = CreateProcessA(
            nullptr,
            command_line,
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &startup_info,
            &process_info);

        if (!created)
        {
            return false;
        }

        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
        return true;
    }

    bool ReadTextInput(const char* prompt, char* buffer, unsigned buffer_size)
    {
        if (prompt == nullptr || buffer == nullptr || buffer_size == 0)
        {
            return false;
        }

        std::printf("%s", prompt);
        if (std::fgets(buffer, static_cast<int>(buffer_size), stdin) == nullptr)
        {
            return false;
        }

        std::size_t len = std::strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
        else
        {
            int ch = 0;
            do
            {
                ch = std::getchar();
            }
            while (ch != '\n' && ch != EOF);
        }

        if (buffer[0] == '\0')
        {
            return false;
        }

        return true;
    }

    bool ReadIntInput(const char* prompt, int& out_value)
    {
        if (prompt == nullptr)
        {
            return false;
        }

        char input[64]{};
        if (!ReadTextInput(prompt, input, static_cast<unsigned>(sizeof(input))))
        {
            return false;
        }

        errno = 0;
        char* end_ptr = nullptr;
        const long parsed = std::strtol(input, &end_ptr, 10);
        if (errno != 0 || end_ptr == input || *end_ptr != '\0')
        {
            return false;
        }

        if (parsed < INT_MIN || parsed > INT_MAX)
        {
            return false;
        }

        out_value = static_cast<int>(parsed);
        return true;
    }

    bool ReadLongLongInput(const char* prompt, long long& out_value)
    {
        if (prompt == nullptr)
        {
            return false;
        }

        char input[64]{};
        if (!ReadTextInput(prompt, input, static_cast<unsigned>(sizeof(input))))
        {
            return false;
        }

        errno = 0;
        char* end_ptr = nullptr;
        const long long parsed = std::strtoll(input, &end_ptr, 10);
        if (errno != 0 || end_ptr == input || *end_ptr != '\0')
        {
            return false;
        }

        out_value = parsed;
        return true;
    }

    bool ReadFloatInput(const char* prompt, float& out_value)
    {
        if (prompt == nullptr)
        {
            return false;
        }

        char input[64]{};
        if (!ReadTextInput(prompt, input, static_cast<unsigned>(sizeof(input))))
        {
            return false;
        }

        errno = 0;
        char* end_ptr = nullptr;
        const float parsed = std::strtof(input, &end_ptr);
        if (errno != 0 || end_ptr == input || *end_ptr != '\0')
        {
            return false;
        }

        out_value = parsed;
        return true;
    }

    bool EnsureDefaultAdmin(practic1::UserStorage& user_storage)
    {
        static constexpr char kDefaultAdminUsername[] = "admin";
        static constexpr char kDefaultAdminEmail[] = "admin@local.com";
        static constexpr char kDefaultAdminPassword[] = "admin123";

        practic1::User users[kMaxUsersInDbConsole]{};
        std::size_t count = 0;
        if (!user_storage.ReadAll(users, kMaxUsersInDbConsole, count))
        {
            return false;
        }

        for (std::size_t i = 0; i < count; ++i)
        {
            if (std::strcmp(users[i].email, kDefaultAdminEmail) == 0)
            {
                users[i].rank = practic1::UserRank::Admin;
                users[i].is_blocked = 0;
                strcpy_s(users[i].password, sizeof(users[i].password), kDefaultAdminPassword);
                return user_storage.UpdateById(users[i].id, users[i]);
            }
        }

        practic1::User admin{};
        strcpy_s(admin.username, sizeof(admin.username), kDefaultAdminUsername);
        strcpy_s(admin.email, sizeof(admin.email), kDefaultAdminEmail);
        strcpy_s(admin.password, sizeof(admin.password), kDefaultAdminPassword);
        admin.rank = practic1::UserRank::Admin;
        admin.created_at = static_cast<practic1::Timestamp>(std::time(nullptr));
        admin.is_blocked = 0;
        admin.is_verified = 1;
        return user_storage.Add(admin);
    }

    void PrintAdminMenu()
    {
        std::printf("\n=========== ADMIN PANEL ===========\n");
        std::printf("1 - List users\n");
        std::printf("2 - Block/unblock user\n");
        std::printf("3 - List tracks\n");
        std::printf("4 - Change track status\n");
        std::printf("5 - Platform stats\n");
        std::printf("6 - List ratings\n");
        std::printf("7 - Full edit user\n");
        std::printf("8 - Full edit track\n");
        std::printf("9 - Full edit rating\n");
        std::printf("0 - Back\n");
        std::printf("-----------------------------------\n");
    }

    void RunAdminPanel(const practic1::AdminService& admin_service, const practic1::User& current_user)
    {
        for (;;)
        {
            PrintAdminMenu();
            int command = -1;
            if (!ReadIntInput("Admin command: ", command))
            {
                std::printf("Input error.\n");
                continue;
            }

            if (command == 0)
            {
                return;
            }

            if (command == 1)
            {
                static practic1::User users[kMaxUsersInAdminUi]{};
                std::size_t count = 0;
                if (!admin_service.GetAllUsers(current_user.id, users, kMaxUsersInAdminUi, count))
                {
                    std::printf("Cannot read users.\n");
                    continue;
                }

                std::printf("\n=== USERS (%zu) ===\n", count);
                for (std::size_t i = 0; i < count; ++i)
                {
                    std::printf(
                        "id=%d username=%s email=%s password_hash=%s rank=%u blocked=%u verified=%u created_at=%lld\n",
                        static_cast<int>(users[i].id),
                        users[i].username,
                        users[i].email,
                        users[i].password,
                        static_cast<unsigned>(users[i].rank),
                        static_cast<unsigned>(users[i].is_blocked),
                        static_cast<unsigned>(users[i].is_verified),
                        static_cast<long long>(users[i].created_at));
                }
                continue;
            }

            if (command == 2)
            {
                int target_id = 0;
                int blocked_flag = 0;
                if (!ReadIntInput("Target user id: ", target_id) ||
                    !ReadIntInput("Blocked flag (0/1): ", blocked_flag))
                {
                    std::printf("Input error.\n");
                    continue;
                }

                if (target_id <= 0 || (blocked_flag != 0 && blocked_flag != 1))
                {
                    std::printf("Invalid values.\n");
                    continue;
                }

                if (!admin_service.SetUserBlocked(
                        current_user.id,
                        static_cast<practic1::Id>(target_id),
                        static_cast<std::uint8_t>(blocked_flag)))
                {
                    std::printf("User status update failed.\n");
                    continue;
                }

                std::printf("User status updated.\n");
                continue;
            }

            if (command == 3)
            {
                static practic1::Track tracks[kMaxTracksInAdminUi]{};
                std::size_t count = 0;
                if (!admin_service.GetAllTracks(current_user.id, tracks, kMaxTracksInAdminUi, count))
                {
                    std::printf("Cannot read tracks.\n");
                    continue;
                }

                std::printf("\n=== TRACKS (%zu) ===\n", count);
                for (std::size_t i = 0; i < count; ++i)
                {
                    std::printf(
                        "id=%d title=%s file_path=%s genre=%u duration=%u upload_date=%lld author=%d avg=%.2f ratings=%u status=%u bpm=%u\n",
                        static_cast<int>(tracks[i].id),
                        tracks[i].title,
                        tracks[i].file_path,
                        static_cast<unsigned>(tracks[i].genre),
                        static_cast<unsigned>(tracks[i].duration),
                        static_cast<long long>(tracks[i].upload_date),
                        static_cast<int>(tracks[i].author_id),
                        tracks[i].average_rating,
                        static_cast<unsigned>(tracks[i].ratings_count),
                        static_cast<unsigned>(tracks[i].status),
                        static_cast<unsigned>(tracks[i].bpm));
                }
                continue;
            }

            if (command == 4)
            {
                int track_id = 0;
                int status_code = 0;
                if (!ReadIntInput("Track id: ", track_id) ||
                    !ReadIntInput("Status (1=Active, 2=Blocked): ", status_code))
                {
                    std::printf("Input error.\n");
                    continue;
                }

                if (track_id <= 0 || (status_code != 1 && status_code != 2))
                {
                    std::printf("Invalid values.\n");
                    continue;
                }

                const auto status = status_code == 1
                    ? practic1::TrackStatus::Active
                    : practic1::TrackStatus::Blocked;

                if (!admin_service.SetTrackStatus(current_user.id, static_cast<practic1::Id>(track_id), status))
                {
                    std::printf("Track status update failed.\n");
                    continue;
                }

                std::printf("Track status updated.\n");
                continue;
            }

            if (command == 5)
            {
                practic1::PlatformStats stats{};
                if (!admin_service.GetPlatformStats(current_user.id, stats))
                {
                    std::printf("Cannot read platform stats.\n");
                    continue;
                }

                std::printf("\n=== PLATFORM STATS ===\n");
                std::printf("users_total: %u\n", static_cast<unsigned>(stats.users_total));
                std::printf("admins_total: %u\n", static_cast<unsigned>(stats.admins_total));
                std::printf("blocked_users_total: %u\n", static_cast<unsigned>(stats.blocked_users_total));
                std::printf("tracks_total: %u\n", static_cast<unsigned>(stats.tracks_total));
                std::printf("active_tracks_total: %u\n", static_cast<unsigned>(stats.active_tracks_total));
                std::printf("blocked_tracks_total: %u\n", static_cast<unsigned>(stats.blocked_tracks_total));
                std::printf("ratings_total: %u\n", static_cast<unsigned>(stats.ratings_total));
                std::printf("average_rating_overall: %.2f\n", stats.average_rating_overall);
                continue;
            }

            if (command == 6)
            {
                static practic1::Rating ratings[kMaxRatingsInAdminUi]{};
                std::size_t count = 0;
                if (!admin_service.GetAllRatings(current_user.id, ratings, kMaxRatingsInAdminUi, count))
                {
                    std::printf("Cannot read ratings.\n");
                    continue;
                }

                std::printf("\n=== RATINGS (%zu) ===\n", count);
                for (std::size_t i = 0; i < count; ++i)
                {
                    std::printf(
                        "id=%d track_id=%d user_id=%d value=%u created_at=%lld\n",
                        static_cast<int>(ratings[i].id),
                        static_cast<int>(ratings[i].track_id),
                        static_cast<int>(ratings[i].user_id),
                        static_cast<unsigned>(ratings[i].value),
                        static_cast<long long>(ratings[i].created_at));
                }
                continue;
            }

            if (command == 7)
            {
                int target_id = 0;
                int rank_code = 0;
                int blocked_flag = 0;
                int verified_flag = 0;
                long long created_at = 0;
                practic1::User updated_user{};

                if (!ReadIntInput("Target user id: ", target_id) ||
                    !ReadTextInput("New username: ", updated_user.username, static_cast<unsigned>(sizeof(updated_user.username))) ||
                    !ReadTextInput("New email: ", updated_user.email, static_cast<unsigned>(sizeof(updated_user.email))) ||
                    !ReadTextInput("New password or existing hash: ", updated_user.password, static_cast<unsigned>(sizeof(updated_user.password))) ||
                    !ReadIntInput("Rank (1=User, 2=Admin): ", rank_code) ||
                    !ReadIntInput("Blocked flag (0/1): ", blocked_flag) ||
                    !ReadIntInput("Verified flag (0/1): ", verified_flag) ||
                    !ReadLongLongInput("Created_at unix: ", created_at))
                {
                    std::printf("Input error.\n");
                    continue;
                }

                if (target_id <= 0 ||
                    (rank_code != 1 && rank_code != 2) ||
                    (blocked_flag != 0 && blocked_flag != 1) ||
                    (verified_flag != 0 && verified_flag != 1))
                {
                    std::printf("Invalid values.\n");
                    continue;
                }

                updated_user.id = static_cast<practic1::Id>(target_id);
                updated_user.rank = rank_code == 1 ? practic1::UserRank::User : practic1::UserRank::Admin;
                updated_user.is_blocked = static_cast<std::uint8_t>(blocked_flag);
                updated_user.is_verified = static_cast<std::uint8_t>(verified_flag);
                updated_user.created_at = static_cast<practic1::Timestamp>(created_at);

                if (!admin_service.UpdateUser(current_user.id, updated_user.id, updated_user))
                {
                    std::printf("Full user update failed.\n");
                    continue;
                }

                std::printf("User fully updated.\n");
                continue;
            }

            if (command == 8)
            {
                int track_id = 0;
                int genre = 0;
                int duration = 0;
                int author_id = 0;
                int ratings_count = 0;
                int status_code = 0;
                int bpm = 0;
                long long upload_date = 0;
                float average_rating = 0.0f;
                practic1::Track updated_track{};

                if (!ReadIntInput("Track id: ", track_id) ||
                    !ReadTextInput("New title: ", updated_track.title, static_cast<unsigned>(sizeof(updated_track.title))) ||
                    !ReadTextInput("New file_path: ", updated_track.file_path, static_cast<unsigned>(sizeof(updated_track.file_path))) ||
                    !ReadIntInput("Genre code (0..65535): ", genre) ||
                    !ReadIntInput("Duration seconds: ", duration) ||
                    !ReadLongLongInput("Upload_date unix: ", upload_date) ||
                    !ReadIntInput("Author user id: ", author_id) ||
                    !ReadFloatInput("Average rating (0..5): ", average_rating) ||
                    !ReadIntInput("Ratings count: ", ratings_count) ||
                    !ReadIntInput("Status (1=Active, 2=Blocked): ", status_code) ||
                    !ReadIntInput("BPM (1..65535): ", bpm))
                {
                    std::printf("Input error.\n");
                    continue;
                }

                if (track_id <= 0 ||
                    genre < 0 || genre > 65535 ||
                    duration <= 0 ||
                    author_id <= 0 ||
                    average_rating < 0.0f || average_rating > 5.0f ||
                    ratings_count < 0 ||
                    (status_code != 1 && status_code != 2) ||
                    bpm <= 0 || bpm > 65535)
                {
                    std::printf("Invalid values.\n");
                    continue;
                }

                updated_track.id = static_cast<practic1::Id>(track_id);
                updated_track.genre = static_cast<std::uint16_t>(genre);
                updated_track.duration = static_cast<practic1::DurationSeconds>(duration);
                updated_track.upload_date = static_cast<practic1::Timestamp>(upload_date);
                updated_track.author_id = static_cast<practic1::Id>(author_id);
                updated_track.average_rating = average_rating;
                updated_track.ratings_count = static_cast<std::uint32_t>(ratings_count);
                updated_track.status = status_code == 1 ? practic1::TrackStatus::Active : practic1::TrackStatus::Blocked;
                updated_track.bpm = static_cast<std::uint16_t>(bpm);

                if (!admin_service.UpdateTrack(current_user.id, updated_track.id, updated_track))
                {
                    std::printf("Full track update failed.\n");
                    continue;
                }

                std::printf("Track fully updated.\n");
                continue;
            }

            if (command == 9)
            {
                int rating_id = 0;
                int track_id = 0;
                int user_id = 0;
                int value = 0;
                long long created_at = 0;
                practic1::Rating updated_rating{};

                if (!ReadIntInput("Rating id: ", rating_id) ||
                    !ReadIntInput("Track id: ", track_id) ||
                    !ReadIntInput("User id: ", user_id) ||
                    !ReadIntInput("Value (0..5): ", value) ||
                    !ReadLongLongInput("Created_at unix: ", created_at))
                {
                    std::printf("Input error.\n");
                    continue;
                }

                if (rating_id <= 0 || track_id <= 0 || user_id <= 0 || value < 0 || value > 5)
                {
                    std::printf("Invalid values.\n");
                    continue;
                }

                updated_rating.id = static_cast<practic1::Id>(rating_id);
                updated_rating.track_id = static_cast<practic1::Id>(track_id);
                updated_rating.user_id = static_cast<practic1::Id>(user_id);
                updated_rating.value = static_cast<practic1::RatingValue>(value);
                updated_rating.created_at = static_cast<practic1::Timestamp>(created_at);

                if (!admin_service.UpdateRating(current_user.id, updated_rating.id, updated_rating))
                {
                    std::printf("Full rating update failed.\n");
                    continue;
                }

                std::printf("Rating fully updated. Track statistics recalculated.\n");
                continue;
            }

            std::printf("Unknown admin command.\n");
        }
    }
}

int main(int argc, char* argv[])
{
    if (IsDbConsoleMode(argc, argv))
    {
        return RunDatabaseConsole();
    }

    if (!StartDatabaseConsoleProcess())
    {
        std::printf("Warning: cannot start second DB console.\n");
    }

    practic1::UserStorage user_storage("users_app.dat");
    practic1::TrackStorage track_storage("tracks_app.dat");
    practic1::RatingStorage rating_storage("ratings_app.dat");

    practic1::AuthService auth_service(user_storage);
    practic1::TrackService track_service(track_storage);
    practic1::RatingService rating_service(user_storage, track_storage, rating_storage);
    practic1::AdminService admin_service(user_storage, track_storage, rating_storage);

    if (!EnsureDefaultAdmin(user_storage))
    {
        std::printf("Warning: cannot ensure default admin account.\n");
    }
    else
    {
        std::printf("Default admin credentials: email=admin@local.com password=admin123\n");
    }

    bool is_logged_in = false;
    practic1::User current_user{};
    bool has_current_track = false;
    practic1::Track current_track{};

    for (;;)
    {
        PrintMainMenu(is_logged_in, current_user, has_current_track);

        int menu_choice = -1;
        if (!ReadIntInput("Select action: ", menu_choice))
        {
            std::printf("Input error.\n");
            continue;
        }

        if (menu_choice == 0)
        {
            std::printf("Exit.\n");
            return 0;
        }

        if (menu_choice == 1)
        {
            char username[practic1::kUsernameLength]{};
            char email[practic1::kEmailLength]{};
            char password[practic1::kPasswordLength]{};

            if (!ReadTextInput("Register username: ", username, static_cast<unsigned>(sizeof(username))) ||
                !ReadTextInput("Register email: ", email, static_cast<unsigned>(sizeof(email))) ||
                !ReadTextInput("Register password: ", password, static_cast<unsigned>(sizeof(password))))
            {
                std::printf("Input error.\n");
                continue;
            }

            practic1::User created_user{};
            const auto now = static_cast<practic1::Timestamp>(std::time(nullptr));
            if (!auth_service.RegisterUser(username, email, password, now, created_user))
            {
                std::printf("Registration failed.\n");
                continue;
            }

            std::printf("Registration success. New user id=%d\n", static_cast<int>(created_user.id));
            continue;
        }

        if (menu_choice == 2)
        {
            char email[practic1::kEmailLength]{};
            char password[practic1::kPasswordLength]{};

            if (!ReadTextInput("Login email: ", email, static_cast<unsigned>(sizeof(email))) ||
                !ReadTextInput("Login password: ", password, static_cast<unsigned>(sizeof(password))))
            {
                std::printf("Input error.\n");
                continue;
            }

            practic1::User logged_user{};
            if (!auth_service.LoginByEmail(email, password, logged_user))
            {
                std::printf("Login failed.\n");
                continue;
            }

            is_logged_in = true;
            current_user = logged_user;
            std::printf("Login success.\n");
            continue;
        }

        if (menu_choice == 3)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            PrintUserAccount(current_user);
            continue;
        }

        if (menu_choice == 4)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            char title[practic1::kTrackTitleLength]{};
            char file_path[practic1::kFilePathLength]{};
            int genre = 0;
            int duration = 0;
            int bpm = 0;

            if (!ReadTextInput("Track title: ", title, static_cast<unsigned>(sizeof(title))) ||
                !ReadTextInput("Track file path: ", file_path, static_cast<unsigned>(sizeof(file_path))) ||
                !ReadIntInput("Genre code (number): ", genre) ||
                !ReadIntInput("Duration (seconds): ", duration) ||
                !ReadIntInput("BPM: ", bpm))
            {
                std::printf("Input error.\n");
                continue;
            }

            if (genre < 0 || genre > 65535 || duration <= 0 || bpm <= 0 || bpm > 65535)
            {
                std::printf("Invalid numeric values.\n");
                continue;
            }

            practic1::Track created_track{};
            const auto now = static_cast<practic1::Timestamp>(std::time(nullptr));
            if (!track_service.UploadTrack(
                    current_user.id,
                    title,
                    file_path,
                    static_cast<std::uint16_t>(genre),
                    static_cast<practic1::DurationSeconds>(duration),
                    static_cast<std::uint16_t>(bpm),
                    now,
                    created_track))
            {
                std::printf("Upload failed.\n");
                continue;
            }

            std::printf("Track uploaded. id=%d\n", static_cast<int>(created_track.id));
            continue;
        }

        if (menu_choice == 5)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            practic1::Track random_track{};
            if (!track_service.GetRandomTrackForUser(current_user.id, random_track))
            {
                std::printf("No active tracks available.\n");
                continue;
            }

            current_track = random_track;
            has_current_track = true;
            PrintTrack(current_track);
            continue;
        }

        if (menu_choice == 6)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            if (!has_current_track)
            {
                std::printf("Select a random track first (menu 5).\n");
                continue;
            }

            practic1::Track next_track{};
            if (!track_service.GetNextRandomTrack(current_user.id, current_track.id, next_track))
            {
                std::printf("No next track available.\n");
                continue;
            }

            current_track = next_track;
            PrintTrack(current_track);
            continue;
        }

        if (menu_choice == 7)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            if (!has_current_track)
            {
                std::printf("Select a track first (menu 5).\n");
                continue;
            }

            int value = 0;
            if (!ReadIntInput("Rating value (0..5): ", value))
            {
                std::printf("Input error.\n");
                continue;
            }

            if (value < 0 || value > 5)
            {
                std::printf("Invalid rating value.\n");
                continue;
            }

            bool is_new_rating = false;
            const auto now = static_cast<practic1::Timestamp>(std::time(nullptr));
            if (!rating_service.RateTrack(
                    current_user.id,
                    current_track.id,
                    static_cast<practic1::RatingValue>(value),
                    now,
                    is_new_rating))
            {
                std::printf("Rating failed.\n");
                continue;
            }

            practic1::Track refreshed_track{};
            if (track_storage.FindById(current_track.id, refreshed_track))
            {
                current_track = refreshed_track;
            }

            std::printf(is_new_rating ? "Rating added.\n" : "Rating updated.\n");
            PrintTrack(current_track);
            continue;
        }

        if (menu_choice == 8)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            practic1::Track tracks[kMaxTracksInUi]{};
            std::size_t track_count = 0;
            if (!track_service.GetTracksByAuthor(current_user.id, tracks, kMaxTracksInUi, track_count))
            {
                std::printf("Cannot read track stats.\n");
                continue;
            }

            std::printf("\n=== MY TRACKS (%zu) ===\n", track_count);
            for (std::size_t i = 0; i < track_count; ++i)
            {
                std::printf(
                    "id=%d title=%s avg=%.2f ratings=%u status=%u\n",
                    static_cast<int>(tracks[i].id),
                    tracks[i].title,
                    tracks[i].average_rating,
                    static_cast<unsigned>(tracks[i].ratings_count),
                    static_cast<unsigned>(tracks[i].status));
            }
            continue;
        }

        if (menu_choice == 9)
        {
            if (StartDatabaseConsoleProcess())
            {
                std::printf("DB console started.\n");
            }
            else
            {
                std::printf("Cannot start DB console.\n");
            }
            continue;
        }

        if (menu_choice == 10)
        {
            is_logged_in = false;
            has_current_track = false;
            std::memset(&current_user, 0, sizeof(current_user));
            std::printf("Logged out.\n");
            continue;
        }

        if (menu_choice == 11)
        {
            if (!is_logged_in)
            {
                std::printf("Login required.\n");
                continue;
            }

            if (current_user.rank != practic1::UserRank::Admin)
            {
                std::printf("Admin access required.\n");
                continue;
            }

            RunAdminPanel(admin_service, current_user);
            continue;
        }

        std::printf("Unknown menu command.\n");
    }

    return 0;
}
