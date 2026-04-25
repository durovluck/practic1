#include "AuthService.h"
#include "UserStorage.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <windows.h>

namespace
{
    constexpr std::size_t kMaxUsersInDbConsole = 4096;

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
            if (scanf_s("%d", &command) != 1)
            {
                std::printf("Input error.\n");
                return 1;
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

        char scan_format[16]{};
        if (std::snprintf(scan_format, sizeof(scan_format), "%%%us", buffer_size - 1) <= 0)
        {
            return false;
        }

        std::printf("%s", prompt);
        if (scanf_s(scan_format, buffer, buffer_size) != 1)
        {
            return false;
        }
        return true;
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
    practic1::AuthService auth_service(user_storage);

    char register_username[practic1::kUsernameLength]{};
    char register_email[practic1::kEmailLength]{};
    char register_password[practic1::kPasswordLength]{};

    std::printf("=== USE CASE: Registration -> Login -> Account View ===\n");
    std::printf("Step 1/3: Registration\n");

    if (!ReadTextInput("Enter username: ", register_username, static_cast<unsigned>(sizeof(register_username))))
    {
        std::printf("Input error: username\n");
        return 1;
    }

    if (!ReadTextInput("Enter email: ", register_email, static_cast<unsigned>(sizeof(register_email))))
    {
        std::printf("Input error: email\n");
        return 2;
    }

    if (!ReadTextInput("Enter password: ", register_password, static_cast<unsigned>(sizeof(register_password))))
    {
        std::printf("Input error: password\n");
        return 3;
    }

    practic1::User registered_user{};
    const auto now = static_cast<practic1::Timestamp>(std::time(nullptr));
    if (!auth_service.RegisterUser(register_username, register_email, register_password, now, registered_user))
    {
        std::printf("Registration failed (invalid data or email already exists).\n");
        return 4;
    }

    std::printf("Registration success. User saved to file users_app.dat with id=%d\n", static_cast<int>(registered_user.id));

    std::printf("\nStep 2/3: Login\n");
    char login_email[practic1::kEmailLength]{};
    char login_password[practic1::kPasswordLength]{};

    if (!ReadTextInput("Enter email: ", login_email, static_cast<unsigned>(sizeof(login_email))))
    {
        std::printf("Input error: login email\n");
        return 5;
    }

    if (!ReadTextInput("Enter password: ", login_password, static_cast<unsigned>(sizeof(login_password))))
    {
        std::printf("Input error: login password\n");
        return 6;
    }

    practic1::User logged_user{};
    if (!auth_service.LoginByEmail(login_email, login_password, logged_user))
    {
        std::printf("Login failed (wrong email/password or blocked user).\n");
        return 7;
    }

    std::printf("Login success.\n");
    std::printf("\nStep 3/3: View account data\n");
    PrintUserAccount(logged_user);

    return 0;
}
