#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "thread_copy.h"

int main()
{
    struct timespec start, end;
    struct stat file_info;

    double time_taken;
    double throughput;

    printf("COPYFAST - MULTITHREADED COPY\n");
    printf("-----------------------------\n");

    if (stat("test_data/large_file.bin", &file_info) == -1)
    {
        perror("File information error");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (threaded_copy(
            "test_data/large_file.bin",
            "test_data/thread_backup.bin"))
    {
        clock_gettime(CLOCK_MONOTONIC, &end);

        printf("Threaded copy completed successfully!\n");

        time_taken =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1000000000.0;

        throughput =
            (file_info.st_size / (1024.0 * 1024.0))
            / time_taken;

        printf("\nPerformance\n");
        printf("-----------------------------\n");
        printf("File size  : %.2f MB\n",
               file_info.st_size / (1024.0 * 1024.0));

        printf("Time taken : %.6f seconds\n",
               time_taken);

        printf("Throughput : %.2f MB/s\n",
               throughput);
    }
    else
    {
        printf("Threaded copy failed!\n");
        return 1;
    }

    return 0;
}
