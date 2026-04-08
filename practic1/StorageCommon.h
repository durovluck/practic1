#pragma once

#include <cstddef>

namespace practic1::storage_common
{
    bool CopyPath(const char* source, char* destination, std::size_t destination_size);
    bool BuildTempPath(const char* source_path, char* temp_path, std::size_t temp_path_size);
    bool ReplaceFileWithTemp(const char* original_path, const char* temp_path);
}
