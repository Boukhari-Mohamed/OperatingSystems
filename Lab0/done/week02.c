/**
 * @file week02.c
 * @brief Exercises in C for week 2
 *
 * TODO's:
 * 1. Complete the implementation of w2_bork()
 * 2. Implement w2_fork()
 * 3. Implement w2_clone()
 *
 * @author Ahmad Hazimeh
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "week02.h"

void w2_bork(unsigned int n, void (*verify)(void))
{
    if (n == 0)
        return;

    pid_t cpid = fork();
    if (cpid == -1)
    {
        fprintf(stderr, "fork(): errno %d %s\n", errno, strerror(errno));
        return;
    }
    if (cpid == 0)
    {
        if (verify != NULL)
        {
            verify();
        }
        exit(0);
    }
    else
    {
        wait(NULL);
        w2_bork(n - 1, verify);
    }
}

void w2_fork(const bool *root, int index, void (*verify)(void))
{
    if (root == NULL || index < 0)
    {
        return;
    }

    bool isNode = root[index];

    if (isNode)
    {
        pid_t cpid = fork();

        if (cpid == -1)
        {
            fprintf(stderr, "fork() failed: errno %d %s\n", errno, strerror(errno));
        }
        else if (cpid == 0)
        {
            verify();
            w2_fork(root, 2 * index + 1, verify);
            exit(0);
        }
        else
        {
            wait(NULL);
            w2_fork(root, 2 * index + 2, verify);
        }
    }
}

void w2_clone(const struct pnode *root, int index, void *(*verify_thread)(void *), void (*verify_fork)(void))
{
    if (root == NULL || index < 0)
    {
        return;
    }

    struct pnode fork_node = root[index];
    //Chose int rather than size_t, unsigned, ... because num_threads is of type int
    int thread_count = fork_node.num_threads;

    if (fork_node.must_fork)
    {
        if (thread_count < 0)
        {
            fprintf(stderr, "Cannot create threads. Process exited");
            exit(1);
            return;
        }

        pthread_t thread_ids[thread_count];
        if (verify_thread != NULL)
        {
            //Spawn threads
            for (unsigned i = 0; i < thread_count; i++)
            {
                //Create threads and record their ids
                pthread_create(&thread_ids[i], NULL, verify_thread, NULL);
            }

            //Before forking, call join on each threads (Could have done in one for loop)
            for (unsigned i = 0; i < thread_count; i++)
            {
                pthread_join(thread_ids[i], NULL);
            }
        }

        pid_t cpid = fork();

        if (cpid == -1)
        {
            fprintf(stderr, "fork() failed: errno %d %s\n", errno, strerror(errno));
        }
        else if (cpid == 0)
        {
            if (verify_fork != NULL)
            {
                verify_fork();
            }
            w2_clone(root, index * 2 + 1, verify_thread, verify_fork);

            //Proper cleanup, join each thread (even when already called before fork ???)
            if (verify_thread != NULL)
            {
                for (unsigned i = 0; i < thread_count; i++)
                {
                    pthread_join(thread_ids[i], NULL);
                }
            }
            exit(0);
        }
        else
        {
            wait(NULL);
            w2_clone(root, index * 2 + 2, verify_thread, verify_fork);
        }
    }
}
