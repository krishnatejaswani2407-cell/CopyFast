#include "async_io.h"

#include <liburing.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define BUFFER_SIZE 8192
#define QUEUE_DEPTH 1

int async_copy_file(
    const char *source,
    const char *destination)
{
    int src_fd = -1;
    int dst_fd = -1;
    int result = 0;

    struct stat file_info;
    struct io_uring ring;

    char *buffer = NULL;
    off_t offset = 0;

    src_fd = open(source, O_RDONLY);

    if (src_fd < 0)
    {
        perror("Cannot open source");
        goto cleanup;
    }

    if (fstat(src_fd, &file_info) < 0)
    {
        perror("Cannot get source information");
        goto cleanup;
    }

    if (!S_ISREG(file_info.st_mode))
    {
        fprintf(stderr, "Source must be a regular file\n");
        goto cleanup;
    }

    dst_fd = open(
        destination,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644);

    if (dst_fd < 0)
    {
        perror("Cannot open destination");
        goto cleanup;
    }

    if (file_info.st_size == 0)
    {
        result = 1;
        goto cleanup;
    }

    if (ftruncate(dst_fd, file_info.st_size) < 0)
    {
        perror("Cannot set destination size");
        goto cleanup;
    }

    buffer = malloc(BUFFER_SIZE);

    if (buffer == NULL)
    {
        perror("Cannot allocate buffer");
        goto cleanup;
    }

    int ret = io_uring_queue_init(
        QUEUE_DEPTH,
        &ring,
        0);

    if (ret < 0)
    {
        fprintf(stderr,
                "io_uring initialization failed: %s\n",
                strerror(-ret));
        goto cleanup;
    }

    printf("Async I/O: io_uring initialized\n");

    while (offset < file_info.st_size)
    {
        size_t amount = BUFFER_SIZE;

        if (file_info.st_size - offset < BUFFER_SIZE)
            amount = file_info.st_size - offset;

        struct io_uring_sqe *sqe =
            io_uring_get_sqe(&ring);

        if (sqe == NULL)
        {
            fprintf(stderr, "Cannot obtain I/O request\n");
            break;
        }

        io_uring_prep_read(
            sqe,
            src_fd,
            buffer,
            amount,
            offset);

        ret = io_uring_submit(&ring);

        if (ret < 0)
        {
            fprintf(stderr, "Read submission failed: %s\n",
                    strerror(-ret));
            break;
        }

        struct io_uring_cqe *cqe;

        ret = io_uring_wait_cqe(&ring, &cqe);

        if (ret < 0)
        {
            fprintf(stderr, "Read completion failed: %s\n",
                    strerror(-ret));
            break;
        }

        int bytes_read = cqe->res;

        io_uring_cqe_seen(&ring, cqe);

        if (bytes_read <= 0)
        {
            fprintf(stderr, "Read failed or reached EOF\n");
            break;
        }

        sqe = io_uring_get_sqe(&ring);

        if (sqe == NULL)
        {
            fprintf(stderr, "Cannot obtain write request\n");
            break;
        }

        io_uring_prep_write(
            sqe,
            dst_fd,
            buffer,
            bytes_read,
            offset);

        ret = io_uring_submit(&ring);

        if (ret < 0)
        {
            fprintf(stderr, "Write submission failed: %s\n",
                    strerror(-ret));
            break;
        }

        ret = io_uring_wait_cqe(&ring, &cqe);

        if (ret < 0)
        {
            fprintf(stderr, "Write completion failed: %s\n",
                    strerror(-ret));
            break;
        }

        int bytes_written = cqe->res;

        io_uring_cqe_seen(&ring, cqe);

        if (bytes_written != bytes_read)
        {
            fprintf(stderr, "Incomplete write\n");
            break;
        }

        offset += bytes_read;
    }

    if (offset == file_info.st_size)
    {
        result = 1;
        printf("Async I/O: copying completed\n");
    }

    io_uring_queue_exit(&ring);

cleanup:

    free(buffer);

    if (src_fd >= 0)
        close(src_fd);

    if (dst_fd >= 0)
        close(dst_fd);

    return result;
}
