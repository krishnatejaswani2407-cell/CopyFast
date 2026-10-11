#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

typedef struct
{
    long long start;
    long long size;
} CopyTask;

int create_tasks(
    long long file_size,
    int num_threads,
    CopyTask *tasks
);

#endif
