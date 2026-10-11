#include "copy_manager.h"
#include "file_copy.h"
#include "thread_copy.h"
#include "async_io.h"

int run_copy_manager(
    const char *source,
    const char *destination,
    int choice,
    int num_threads)
{
    if (choice == 1)
    {
        return copy_file(source, destination);
    }

    if (choice == 2)
    {
        return threaded_copy(
            source,
            destination,
            num_threads
        );
    }

    if (choice == 3)
    {
        return async_copy_file(
            source,
            destination
        );
    }

    return 0;
}
