#pragma once

#include <cstddef>
#include <cstdio>

namespace practic1::storage_common
{
    FILE* OpenFile(const char* path, const char* mode);
    bool CopyPath(const char* source, char* destination, std::size_t destination_size);
    bool BuildTempPath(const char* source_path, char* temp_path, std::size_t temp_path_size);
    bool ReplaceFileWithTemp(const char* original_path, const char* temp_path);
}
