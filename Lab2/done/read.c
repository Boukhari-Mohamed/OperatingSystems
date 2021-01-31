/*
 * @file read.c
 * @brief Implementation of read
 *
 * @author Matteo Rizzo, Mark Sutherland
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

int32_t lab3_read(int fd, void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || buffer == NULL)
    {
        return -1;
    }

    struct lab3_open_file *file = &open_file_table[fd];
    if (file->inode == NULL || file->inode->is_directory)
    {
        return -1;
    }

    if (file->seek_offset >= file->inode->file.size)
    {
        return 0;
    }

    uint32_t const available = file->inode->file.size - file->seek_offset;
    //Determing starting block

    // Compute how many bytes will actually be read
    uint32_t to_read;
    if (size <= available)
    {
        to_read = size;
    }
    else
    {
        to_read = available;
    }
    uint32_t current_blk = file->seek_offset / DISK_BLK_SIZE;
    uint32_t seek_off_offset = file->seek_offset % DISK_BLK_SIZE;
    uint32_t const available_in_curr_blk = DISK_BLK_SIZE - seek_off_offset;
    uint32_t current_offset = file->inode->file.data_block_offsets[current_blk];

    if (available_in_curr_blk >= to_read)
    {
        // NO NEED TO CROSS BLOCKS
        int err = read_from_disk(current_offset + seek_off_offset, buffer, to_read);
        if (err != 0)
            return -1;
    }
    else
    {
        //READ REMAINING DATA IN CURRENT BLOCK
        int err = read_from_disk(current_offset + seek_off_offset, buffer, available_in_curr_blk);
        if (err != 0)
            return -1;

        buffer = ((char *)buffer) + available_in_curr_blk;
        uint32_t rem = to_read - available_in_curr_blk;
        current_blk += 1;
        current_offset = file->inode->file.data_block_offsets[current_blk];

        //READ FULL BLOCKS AND COPY TO BUFFER
        while (rem >= DISK_BLK_SIZE && err == 0)
        {
            err = read_from_disk(current_offset, buffer, DISK_BLK_SIZE);
            if (err != 0)
                return -1;
            buffer = ((char *)buffer) + DISK_BLK_SIZE;
            rem -= DISK_BLK_SIZE;
            current_blk += 1;
            current_offset = file->inode->file.data_block_offsets[current_blk];
        }

        //HANDLE LAST PART
        if (rem > 0)
        {
            err = read_from_disk(current_offset, buffer, rem);
            if (err != 0)
                return -1;
        }
    }
    // In case of sucess, move offset and return read count
    file->seek_offset += to_read;
    return to_read;
}
