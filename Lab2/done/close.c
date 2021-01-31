/*
 * @file close.c
 * @brief Implementation of close
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>

#include "fs_api.h"
#include "open_file_table.h"

int lab3_close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        return -1;
    }

    struct lab3_open_file *entry = &open_file_table[fd];
    if (entry->inode == NULL)
    {
        //Trying to close an already closed file
        return -1;
    }

    free(entry->inode);
    entry->inode = NULL;
    entry->seek_offset = 0;
    return 0;
}
