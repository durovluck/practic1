#include "AuthService.h"
#include "UserStorage.h"

#include <cstdio>
#include <ctime>

namespace
{
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

    bool ReadTextInput(const char* prompt, char* buffer, unsigned buffer_size)
    {
        if (prompt == nullptr || buffer == nullptr || buffer_size == 0)
        {
            return false;
        }

        std::printf("%s", prompt);
        if (scanf_s("%63s", buffer, buffer_size) != 1)
        {
            return false;
        }
        return true;
    }
}

int main()
{
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
