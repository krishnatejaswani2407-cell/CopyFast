#include "task_manager.h"
#include <stddef.h>

int create_tasks(
    long long file_size,
    int num_threads,
    CopyTask *tasks)
{
    long long part_size;
    int i;

    if (num_threads <= 0 || tasks == NULL)
    {
        return 0;
    }

    part_size = file_size / num_threads;

    for (i = 0; i < num_threads; i++)
    {
        tasks[i].start = i * part_size;

        if (i == num_threads - 1)
        {
            tasks[i].size =
                file_size - tasks[i].start;
        }
        else
        {
            tasks[i].size = part_size;
        }
    }

    return 1;
}
