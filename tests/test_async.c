#include <stdio.h>
#include "async_io.h"
#include "file_copy.h"

int main(void)
{
    const char *source = "test_data/large_file.bin";
const char *destination = "test_data/async_large_backup.bin";
    printf("COPYFAST - Async I/O Large File Test\n");
    printf("====================================\n");

    if (!async_copy_file(source, destination))
    {
        printf("Async copy failed!\n");
        return 1;
    }

    printf("Verifying copied file...\n");

    if (verify_file(source, destination))
    {
        printf("Verification successful!\n");
        return 0;
    }

    printf("Verification failed!\n");
    return 1;
}
