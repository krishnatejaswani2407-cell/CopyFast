#include "thread_copy.h"
#include "task_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#define BUFFER_SIZE 8192

typedef struct
{
    int source_fd;
    int destination_fd;
    CopyTask task;
} ThreadData;


void *copy_part(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    char buffer[BUFFER_SIZE];

    off_t position = data->task.start;
    off_t remaining = data->task.size;

    while (remaining > 0)
    {
        size_t chunk;

        if (remaining > BUFFER_SIZE)
            chunk = BUFFER_SIZE;
        else
            chunk = remaining;

        ssize_t bytes_read = pread(
            data->source_fd,
            buffer,
            chunk,
            position
        );

        if (bytes_read <= 0)
        {
            perror("Thread read error");
            return NULL;
        }

        ssize_t bytes_written = pwrite(
            data->destination_fd,
            buffer,
            bytes_read,
            position
        );

        if (bytes_written != bytes_read)
        {
            perror("Thread write error");
            return NULL;
        }

        position += bytes_read;
        remaining -= bytes_read;
    }

    return NULL;
}


int threaded_copy(const char *source,
                  const char *destination,
                  int num_threads)
{
    int source_fd;
    int destination_fd;

    struct stat file_info;

    source_fd = open(source, O_RDONLY);

    if (source_fd == -1)
    {
        perror("Error opening source");
        return 0;
    }

    if (fstat(source_fd, &file_info) == -1)
    {
        perror("Error getting file size");
        close(source_fd);
        return 0;
    }

    off_t file_size = file_info.st_size;

    destination_fd = open(
        destination,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (destination_fd == -1)
    {
        perror("Error opening destination");
        close(source_fd);
        return 0;
    }

    if (ftruncate(destination_fd, file_size) == -1)
    {
        perror("Error setting destination size");
        close(source_fd);
        close(destination_fd);
        return 0;
    }


    /* Create task list */

    CopyTask *tasks =
        malloc(num_threads * sizeof(CopyTask));

    pthread_t *threads =
        malloc(num_threads * sizeof(pthread_t));

    ThreadData *data =
        malloc(num_threads * sizeof(ThreadData));


    if (tasks == NULL ||
        threads == NULL ||
        data == NULL)
    {
        perror("Memory allocation error");

        free(tasks);
        free(threads);
        free(data);

        close(source_fd);
        close(destination_fd);

        return 0;
    }


    /* Task Manager creates tasks */

    if (!create_tasks(
            file_size,
            num_threads,
            tasks))
    {
        printf("Task Manager failed!\n");

        free(tasks);
        free(threads);
        free(data);

        close(source_fd);
        close(destination_fd);

        return 0;
    }


    printf("Task Manager: %d tasks created\n",
           num_threads);

    printf("Starting %d worker threads...\n",
           num_threads);


    /* Create worker threads */

    for (int i = 0; i < num_threads; i++)
    {
        data[i].source_fd = source_fd;
        data[i].destination_fd = destination_fd;

        data[i].task = tasks[i];

        pthread_create(
            &threads[i],
            NULL,
            copy_part,
            &data[i]
        );
    }


    /* Wait for workers */

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(
            threads[i],
            NULL
        );
    }


    free(tasks);
    free(threads);
    free(data);

    close(source_fd);
    close(destination_fd);

    return 1;
}
