/*
 * @file seek.c
 * @brief Implementation of seek
 *
 * @author Matteo Rizzo
 */
#include "fs_api.h"
#include "open_file_table.h"

int lab3_seek(int fd, uint32_t offset)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        return -1;
    }

    struct lab3_open_file *file = &open_file_table[fd];
    if (file->inode == NULL || file->inode->file.size < offset)
    {
        return -1;
    }

    file->seek_offset = offset;
    return 0;
}
