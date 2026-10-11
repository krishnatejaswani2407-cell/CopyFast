#include "file_copy.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int copy_file(const char *source, const char *destination)
{
    int source_fd;
    int destination_fd;

    char buffer[8192];

    ssize_t bytes_read;
    ssize_t bytes_written;

    source_fd = open(source, O_RDONLY);

    if (source_fd == -1)
    {
        perror("Error opening source file");
        return 0;
    }

    destination_fd = open(
        destination,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (destination_fd == -1)
    {
        perror("Error opening destination file");
        close(source_fd);
        return 0;
    }

    while ((bytes_read = read(
        source_fd,
        buffer,
        sizeof(buffer))) > 0)
    {
        bytes_written = write(
            destination_fd,
            buffer,
            bytes_read
        );

        if (bytes_written != bytes_read)
        {
            perror("Error writing file");

            close(source_fd);
            close(destination_fd);

            return 0;
        }
    }

    if (bytes_read < 0)
    {
        perror("Error reading file");

        close(source_fd);
        close(destination_fd);

        return 0;
    }

    close(source_fd);
    close(destination_fd);

    return 1;
}
int verify_file(const char *source, const char *destination)
{
    int source_fd;
    int destination_fd;

    char source_buffer[8192];
    char destination_buffer[8192];

    ssize_t source_bytes;
    ssize_t destination_bytes;

    source_fd = open(source, O_RDONLY);

    if (source_fd == -1)
    {
        perror("Error opening source for verification");
        return 0;
    }

    destination_fd = open(destination, O_RDONLY);

    if (destination_fd == -1)
    {
        perror("Error opening destination for verification");
        close(source_fd);
        return 0;
    }

    while (1)
    {
        source_bytes = read(
            source_fd,
            source_buffer,
            sizeof(source_buffer)
        );

        destination_bytes = read(
            destination_fd,
            destination_buffer,
            sizeof(destination_buffer)
        );

        if (source_bytes != destination_bytes)
        {
            close(source_fd);
            close(destination_fd);
            return 0;
        }

        if (source_bytes == 0)
        {
            break;
        }

        if (memcmp(source_buffer, destination_buffer, source_bytes) != 0)
        {
            close(source_fd);
            close(destination_fd);
            return 0;
        }
    }

    close(source_fd);
    close(destination_fd);

    return 1;
}
