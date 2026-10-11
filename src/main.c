#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#include "file_copy.h"
#include "file_scanner.h"
#include "copy_manager.h"

int main(int argc, char *argv[])
{
    int choice;
    int num_threads = 0;

    struct timespec start, end;
    struct stat file_info;

    double time_taken;
    double throughput;

    if (argc != 3)
    {
        printf("Usage: ./copyfast <source> <destination>\n");
        return 1;
    }

    printf("\nCOPYFAST\n");
    printf("============================\n");
    printf("Source      : %s\n", argv[1]);
    printf("Destination : %s\n", argv[2]);

    /* File Scanner */

    if (!validate_source_file(argv[1]))
    {
        printf("Source file validation failed!\n");
        return 1;
    }

    printf("File scanner: source file validated successfully!\n");

    if (stat(argv[1], &file_info) == -1)
    {
        perror("Error getting file size");
        return 1;
    }

    /* Copy Manager */

    printf("\nChoose Copy Method\n");
    printf("----------------------------\n");
    printf("1. Normal Copy\n");
printf("2. Multithreaded Copy\n");
printf("3. Async I/O (io_uring)\n");
printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2)
    {
        printf("Enter number of threads (2/4/8): ");
        scanf("%d", &num_threads);

        if (num_threads != 2 &&
            num_threads != 4 &&
            num_threads != 8)
        {
            printf("Invalid number of threads!\n");
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (!run_copy_manager(
            argv[1],
            argv[2],
            choice,
            num_threads))
    {
        printf("Copy failed!\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("\nCopy completed successfully!\n");

    /* Verification */

    printf("Verifying files...\n");

    if (verify_file(argv[1], argv[2]))
    {
        printf("Verification successful!\n");
    }
    else
    {
        printf("Verification failed!\n");
        return 1;
    }

    /* Performance */

    time_taken =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec)
        / 1000000000.0;

    throughput =
        (file_info.st_size /
        (1024.0 * 1024.0))
        / time_taken;

    printf("\nPerformance\n");
    printf("----------------------------\n");

    printf("File size  : %.2f MB\n",
           file_info.st_size /
           (1024.0 * 1024.0));

    printf("Time taken : %.6f seconds\n",
           time_taken);

    printf("Throughput : %.2f MB/s\n",
           throughput);

    return 0;
}
