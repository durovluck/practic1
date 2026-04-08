#include "PasswordCrypto.h"

#include "Types.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace
{
    constexpr char kHashPrefix[] = "H1$";
    constexpr std::size_t kHashHexLength = 16;
    constexpr std::size_t kHashTextLength = (sizeof(kHashPrefix) - 1) + kHashHexLength;
    constexpr std::uint64_t kFnvOffset = 14695981039346656037ull;
    constexpr std::uint64_t kFnvPrime = 1099511628211ull;
    constexpr char kPepper[] = "practic1_static_pepper_v1";

    void MixBytes(std::uint64_t& hash, const char* text)
    {
        std::size_t i = 0;
        while (text[i] != '\0')
        {
            hash ^= static_cast<unsigned char>(text[i]);
            hash *= kFnvPrime;
            ++i;
        }
    }
}

namespace practic1::security
{
    bool IsPasswordHashed(const char* value)
    {
        if (value == nullptr)
        {
            return false;
        }

        if (std::strncmp(value, kHashPrefix, sizeof(kHashPrefix) - 1) != 0)
        {
            return false;
        }

        return std::strlen(value) == kHashTextLength;
    }

    bool HashPassword(const char* plain_password, char* out_hash, std::size_t out_hash_size)
    {
        if (plain_password == nullptr || out_hash == nullptr)
        {
            return false;
        }

        if (out_hash_size <= kHashTextLength)
        {
            return false;
        }

        std::uint64_t hash = kFnvOffset;
        MixBytes(hash, kPepper);
        MixBytes(hash, plain_password);

        const int written = std::snprintf(
            out_hash,
            out_hash_size,
            "%s%016llX",
            kHashPrefix,
            static_cast<unsigned long long>(hash));

        return written == static_cast<int>(kHashTextLength);
    }

    bool VerifyPassword(const char* plain_password, const char* stored_hash)
    {
        if (plain_password == nullptr || stored_hash == nullptr)
        {
            return false;
        }

        char calculated[kPasswordLength]{};
        if (!HashPassword(plain_password, calculated, sizeof(calculated)))
        {
            return false;
        }

        return std::strcmp(calculated, stored_hash) == 0;
    }
}
