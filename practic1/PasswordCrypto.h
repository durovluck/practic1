#pragma once

#include <cstddef>

namespace practic1::security
{
    bool IsPasswordHashed(const char* value);
    bool HashPassword(const char* plain_password, char* out_hash, std::size_t out_hash_size);
    bool VerifyPassword(const char* plain_password, const char* stored_hash);
}
