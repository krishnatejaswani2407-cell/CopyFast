#include "file_scanner.h"

#include <stdio.h>
#include <sys/stat.h>

int validate_source_file(const char *path)
{
    struct stat file_info;

    if (stat(path, &file_info) == -1)
    {
        perror("File scanner: cannot access source file");
        return 0;
    }

    if (!S_ISREG(file_info.st_mode))
    {
        printf("File scanner: source is not a regular file\n");
        return 0;
    }

    return 1;
}

int get_file_size(const char *path, long long *size)
{
    struct stat file_info;

    if (stat(path, &file_info) == -1)
    {
        perror("File scanner: cannot get file size");
        return 0;
    }

    *size = (long long)file_info.st_size;

    return 1;
}
