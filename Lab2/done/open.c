/*
 * @file open.c
 * @brief Implementation of open
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>

#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

int lab3_open(const char *path)
{
    if (path == NULL)
    {
        return -1;
    }

    struct lab3_inode *node = find_inode_by_path(path);
    //Catch unexisting files, open directories and relative paths
    if (node == NULL || node->is_directory || path[0] != '/')
    {
        free(node);
        return -1;
    }
    struct lab3_open_file *table_entry = NULL;
    struct lab3_open_file *free_spot = NULL;
    int fd = -1;
    for (unsigned i = 0; i < MAX_OPEN_FILES; i++)
    {
        table_entry = &open_file_table[i];

        if (table_entry->inode != NULL)
        {
            if (table_entry->inode->id == node->id)
            {
                free(node);
                return -1;
            }
        }
        else
        {
            fd = i;
            free_spot = table_entry;
        }
    }
    if (free_spot == NULL)
    {
        free(node);
        return -1;
    }

    free_spot->inode = node;
    free_spot->seek_offset = 0;
    return fd;
}
