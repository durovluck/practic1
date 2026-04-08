#include "StorageCommon.h"

#include <cstdio>
#include <cstring>

namespace practic1::storage_common
{
    bool CopyPath(const char* source, char* destination, std::size_t destination_size)
    {
        if (source == nullptr || destination == nullptr || destination_size == 0)
        {
            return false;
        }

        std::size_t index = 0;
        while (source[index] != '\0' && (index + 1) < destination_size)
        {
            destination[index] = source[index];
            ++index;
        }

        destination[index] = '\0';
        return source[index] == '\0';
    }

    bool BuildTempPath(const char* source_path, char* temp_path, std::size_t temp_path_size)
    {
        if (!CopyPath(source_path, temp_path, temp_path_size))
        {
            return false;
        }

        static constexpr char kSuffix[] = ".tmp";
        std::size_t length = std::strlen(temp_path);
        std::size_t suffix_index = 0;

        while (kSuffix[suffix_index] != '\0')
        {
            if ((length + 1) >= temp_path_size)
            {
                return false;
            }

            temp_path[length] = kSuffix[suffix_index];
            ++length;
            ++suffix_index;
        }

        temp_path[length] = '\0';
        return true;
    }

    bool ReplaceFileWithTemp(const char* original_path, const char* temp_path)
    {
        std::remove(original_path);
        return std::rename(temp_path, original_path) == 0;
    }
}
