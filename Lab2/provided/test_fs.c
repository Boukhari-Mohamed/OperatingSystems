/**
 * @file test_fs.c
 * @brief Unit-test suite for the file system lab
 *
 * @author Matteo Rizzo
 */
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "blk_io.h"
#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

START_TEST(empty_fs_test)
{
    ck_assert_int_eq(open_emu_disk("../test_fs/empty_disk.fs"), 0);

    struct lab3_inode *root_inode = find_inode_by_path("/");

    /* Check that the root inode looks fine */
    ck_assert_ptr_ne(root_inode, NULL);
    ck_assert_uint_eq(root_inode->id, 0);
    ck_assert_str_eq(root_inode->name, "root");
    ck_assert(root_inode->is_directory);
    ck_assert_uint_eq(root_inode->directory.num_children, 0);

    for (int i = 0; i < MAX_DATA_BLOCKS_PER_INODE; i++) {
        ck_assert_uint_eq(root_inode->directory.children_offsets[i], 0);
    }

    ck_assert_ptr_eq(find_inode_by_path("/foo/bar"), NULL);

    free(root_inode);
}
END_TEST

START_TEST(open_test_1)
{
    ck_assert_int_eq(open_emu_disk("../test_fs/empty_disk.fs"), 0);

    /* Check that opening NULL fails */
    ck_assert_int_lt(lab3_open(NULL), 0);
}
END_TEST

START_TEST(open_test_2)
{
    ck_assert_int_eq(open_emu_disk("../test_fs/4_files.fs"), 0);

    /* Check that opening a relative path fails */
    ck_assert_int_lt(lab3_open("a"), 0);
}
END_TEST

START_TEST(open_test_3)
{
    ck_assert_int_eq(open_emu_disk("../test_fs/4_files.fs"), 0);

    /* Check that opening a non-existing path fails */
    ck_assert_int_lt(lab3_open("/z"), 0);
}
END_TEST

START_TEST(close_test_1)
{
    ck_assert_int_eq(open_emu_disk("../test_fs/empty_disk.fs"), 0);

    /* Check that we can't close an invalid file */
    ck_assert_int_lt(lab3_close(-1), 0);
    ck_assert_int_lt(lab3_close(1234), 0);
    ck_assert_int_lt(lab3_close(22), 0);
}
END_TEST /* STAFF */

void cleanup(void)
{
    if (the_disk != NULL) {
        close_emu_disk();
    }

    /* Reset the open file table */
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (open_file_table[i].inode != NULL) {
            free(open_file_table[i].inode);
            open_file_table[i].inode = NULL;
        }
    }
}

int main(void)
{
    Suite *s = suite_create("File System Tests");
    TCase *tc1 = tcase_create("testcase");
    tcase_add_checked_fixture(tc1, NULL, cleanup);
    suite_add_tcase(s, tc1);

    tcase_add_test(tc1, empty_fs_test);

    tcase_add_test(tc1, open_test_1);
    tcase_add_test(tc1, open_test_2);
    tcase_add_test(tc1, open_test_3);

    tcase_add_test(tc1, close_test_1);

    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_VERBOSE);

    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
