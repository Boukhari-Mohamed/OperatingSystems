/*
 * @file fs_util.c
 * @brief Utility functions used to implement the file system
 *
 * @author Matteo Rizzo, Mark Sutherland
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "disk.h"
#include "fs_util.h"
#include "open_file_table.h"

int get_sub_inode(struct lab3_inode *parent, char *filename, struct lab3_inode *child_buffer, bool req_dir)
{
    if (parent == NULL || child_buffer == NULL || !parent->is_directory)
    {
        return -1;
    }
    assert(parent->directory.num_children <= MAX_DATA_BLOCKS_PER_INODE);
    disk_off_t child_offset = 0;
    uint32_t child_nbr = 0;
    struct lab3_inode current;
    uint32_t const num_children = parent->directory.num_children;
    int err = 0;
    while (child_nbr < num_children)
    {
        child_offset = parent->directory.children_offsets[child_nbr];
        err = read_from_disk(child_offset,
                             (void *)&current, sizeof(struct lab3_inode));
        if (err != 0)
        {
            return err;
        }
        if (((req_dir && current.is_directory) || !req_dir) &&
            strcmp(filename, current.name) == 0)
        {
            *child_buffer = current;
            return 0;
        }
        child_nbr += 1;
    }
    //Nothing found: error
    return -1;
}

struct lab3_inode *find_inode_by_path(const char *path)
{
    if (path == NULL)
        return NULL;

    struct lab3_superblock *blk = get_disk_superblock();
    disk_off_t first_inode_offset = (blk->first_dnode_bmap +
                                     blk->num_dnode_bmap_blocks) *
                                    DISK_BLK_SIZE;

    free(blk); //free since blk got allocated at call

    struct lab3_inode parent;

    int err = read_from_disk(first_inode_offset, &parent, sizeof(struct lab3_inode));

    if (err != 0)
        return NULL;

    if (strcmp(path, "/") == 0)
    {
        struct lab3_inode *result_inode = malloc(sizeof(struct lab3_inode));
        if (result_inode == NULL)
            return NULL;
        *result_inode = parent;
        return result_inode;
    }

    char pathname[MAX_NAME_SIZE];
    memset(&pathname, 0, MAX_NAME_SIZE);
    strncpy(pathname, path, MAX_NAME_SIZE - 1);
    //Iterate through the structure until we found the last part
    char *prev = NULL;

    char *current = strtok(pathname, "/");

    while (current != NULL)
    {
        prev = current;
        current = strtok(NULL, "/");

        if (current != NULL)
        {
            err = get_sub_inode(&parent, prev, &parent, true);
            if (err != 0)
                return NULL;
        }
    }
    // Either reached end of path, so prev contains filename,
    // or path was empty and prev is NULL

    if (prev == NULL)
        return NULL;
    else
    {
        struct lab3_inode *result_inode = malloc(sizeof(struct lab3_inode));
        if (result_inode == NULL)
            return NULL;
        err = get_sub_inode(&parent, prev, result_inode, false);
        if (err != 0)
        {
            free(result_inode);
            return NULL;
        }

        else
            return result_inode;
    }
}

int read_from_disk(disk_off_t disk_offset, void *buffer, size_t size)
{
    if (disk_offset >= DISK_CAPACITY_BYTES)
        return -1;

    disk_off_t in_blk_offset = disk_offset % DISK_BLK_SIZE;
    disk_off_t blk_ptr = (disk_offset / DISK_BLK_SIZE) * DISK_BLK_SIZE;

    if ((size > DISK_BLK_SIZE - in_blk_offset))
        return -1;

    char local_buffer[DISK_BLK_SIZE];
    int res = get_block(blk_ptr, local_buffer);

    if (res != 0)
        return res;

    char *target = local_buffer + in_blk_offset;
    memcpy(buffer, target, size);
    return 0;
}

/* Implementation of going to the disk, getting block 0, and returning a pointer to
 * a formatted superblock.
 */
struct lab3_superblock *get_disk_superblock(void)
{
    struct lab3_superblock *sblk = (struct lab3_superblock *)malloc(sizeof(struct lab3_superblock));

    /* Read block 0 from the disk */
    int rcode = read_from_disk(0, sblk, sizeof(struct lab3_superblock));
    if (rcode < 0)
    {
        free(sblk);
        return NULL;
    }

    return sblk;
}
int sanitize_fd_and_size(int fd, size_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        return -1;
    }

    /* Check that there is a file with this descriptor */
    if (open_file_table[fd].inode == NULL)
    {
        return -1;
    }

    /* size is never negative because it's unsigned */
    if (size >= MAX_DATA_BLOCKS_PER_INODE * DISK_BLK_SIZE)
    {
        return -1;
    }

    return 0;
}
