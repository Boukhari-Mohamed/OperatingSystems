/*
 * @file readdir.c
 * @brief Implementation of readdir
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"

void cleanup_list(char **list, uint32_t len)
{
    if (list == NULL)
    {
        return;
    }

    for (unsigned i = 0; i < len; i++)
    {
        free(list[i]);
    }
}

int lab3_readdir(const char *path, char ***out, uint32_t *out_size)
{
    if (path == NULL || out == NULL || out_size == NULL)
    {
        return -1;
    }

    struct lab3_inode *node = find_inode_by_path(path);
    if (node == NULL || !node->is_directory)
    {
        free(node);
        return -1;
    }

    // Allocate list
    const uint32_t num = node->directory.num_children;
    char **const list = calloc(num, sizeof(char *));

    // Allocate space for names
    for (unsigned i = 0; i < num; i++)
    {
        char *ptr = malloc(MAX_NAME_SIZE * sizeof(char));
        if (ptr == NULL)
        {
            free(node);
            cleanup_list(list, i);
            free(list);
            return -1;
        }
        list[i] = ptr;
    }

    struct lab3_inode current;
    for (unsigned i = 0; i < num; i++)
    {
        int err = read_from_disk(node->directory.children_offsets[i],
                                 &current,
                                 sizeof(struct lab3_inode));

        if (err)
        {
            free(node);
            cleanup_list(list, num);
            free(list);
            return -1;
        }

        strncpy(list[i], current.name, MAX_NAME_SIZE);
    }

    *out_size = num;
    *out = list;
    free(node);
    return 0;
}