#include "StorageCommon.h"

#include <cstdio>
#include <cstring>

namespace practic1::storage_common
{
    namespace
    {
        bool BuildPathWithSuffix(
            const char* source_path,
            const char* suffix,
            char* output_path,
            std::size_t output_size)
        {
            if (source_path == nullptr || suffix == nullptr || output_path == nullptr || output_size == 0)
            {
                return false;
            }

            std::size_t length = 0;
            while (source_path[length] != '\0' && (length + 1) < output_size)
            {
                output_path[length] = source_path[length];
                ++length;
            }

            output_path[length] = '\0';
            if (source_path[length] != '\0')
            {
                return false;
            }

            std::size_t suffix_index = 0;

            while (suffix[suffix_index] != '\0')
            {
                if ((length + 1) >= output_size)
                {
                    return false;
                }

                output_path[length] = suffix[suffix_index];
                ++length;
                ++suffix_index;
            }

            output_path[length] = '\0';
            return true;
        }
    }

    FILE* OpenFile(const char* path, const char* mode)
    {
        if (path == nullptr || mode == nullptr)
        {
            return nullptr;
        }

        FILE* file = nullptr;
        if (fopen_s(&file, path, mode) != 0)
        {
            return nullptr;
        }

        return file;
    }

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
        static constexpr char kSuffix[] = ".tmp";
        return BuildPathWithSuffix(source_path, kSuffix, temp_path, temp_path_size);
    }

    bool ReplaceFileWithTemp(const char* original_path, const char* temp_path)
    {
        char backup_path[1024]{};
        if (!BuildPathWithSuffix(original_path, ".bak", backup_path, sizeof(backup_path)))
        {
            return false;
        }

        std::remove(backup_path);

        if (std::rename(original_path, backup_path) != 0)
        {
            // If original file is missing, allow replace attempt anyway.
            FILE* check = OpenFile(original_path, "rb");
            if (check != nullptr)
            {
                std::fclose(check);
                return false;
            }
        }

        if (std::rename(temp_path, original_path) == 0)
        {
            std::remove(backup_path);
            return true;
        }

        std::rename(backup_path, original_path);
        return false;
    }
}
