#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "blk_io.h"
#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

int list_contains(char **list, char *name, uint32_t const len)
{
    for (unsigned i = 0; i < len; i++)
    {
        if (strcmp(list[i], name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

START_TEST(test_find_inode_by_path)
{

    ck_assert_int_eq(open_emu_disk("vmem.dump"), 0);

    struct lab3_inode *node = find_inode_by_path("/");

    ck_assert_ptr_ne(node, NULL);
    ck_assert_uint_eq(node->id, 0);
    ck_assert_str_eq(node->name, "root");
    ck_assert(node->is_directory);
    ck_assert_uint_eq(node->directory.num_children, 6);

    free(node);
    node = find_inode_by_path("/key.hex/hidden");
    ck_assert_ptr_eq(node, NULL);
    node = find_inode_by_path("/unexisting_file");
    ck_assert_ptr_eq(node, NULL);
    node = find_inode_by_path("/add/grades.pdf/sub");
    ck_assert_ptr_eq(node, NULL);

    node = find_inode_by_path("/add/final_entry/endfile.exe");
    ck_assert_ptr_ne(node, NULL);
    ck_assert_str_eq(node->name, "endfile.exe");
    ck_assert(!node->is_directory);
    ck_assert_int_eq(node->file.size, 240);
    free(node);

    node = find_inode_by_path("/add/fun_img.png");
    ck_assert_ptr_ne(node, NULL);
    ck_assert_str_eq(node->name, "fun_img.png");
    ck_assert(!node->is_directory);
    ck_assert_int_eq(node->file.size, 39);
    free(node);

    node = find_inode_by_path("/add/grades.pdf");
    ck_assert_ptr_ne(node, NULL);
    ck_assert_str_eq(node->name, "grades.pdf");
    ck_assert(!node->is_directory);
    ck_assert_int_eq(node->file.size, 0);
    free(node);

    close_emu_disk();
}
END_TEST

START_TEST(readdir_test_files_all_present)
{
    ck_assert_int_eq(open_emu_disk("vmem.dump"), 0);

    char **out = NULL;
    uint32_t count = 0;
    lab3_readdir("/", &out, &count);

    ck_assert_ptr_ne(out, NULL);
    ck_assert_int_eq(count, 6);
    ck_assert_int_eq(list_contains(out, "add", count), 1);
    ck_assert_int_eq(list_contains(out, "key.hex", count), 1);
    ck_assert_int_eq(list_contains(out, "sub", count), 1);
    ck_assert_int_eq(list_contains(out, "text.txt", count), 1);
    ck_assert_int_eq(list_contains(out, "large.txt", count), 1);
    ck_assert_int_eq(list_contains(out, "very_large.txt", count), 1);
    ck_assert_int_eq(list_contains(out, "grades.pdf", count), 0);
    ck_assert_int_eq(list_contains(out, "final_entry", count), 0);

    for (unsigned i = 0; i < count; i++)
    {
        free(out[i]);
    }

    free(out);

    lab3_readdir("/add", &out, &count);
    ck_assert_ptr_ne(out, NULL);
    ck_assert_int_eq(count, 3);
    ck_assert_int_eq(list_contains(out, "final_entry", count), 1);
    ck_assert_int_eq(list_contains(out, "fun_img.png", count), 1);
    ck_assert_int_eq(list_contains(out, "grades.pdf", count), 1);
    ck_assert_int_eq(list_contains(out, "text.txt", count), 0);

    for (unsigned i = 0; i < count; i++)
    {
        free(out[i]);
    }
    free(out);

    lab3_readdir("/add/final_entry", &out, &count);
    ck_assert_ptr_ne(out, NULL);
    ck_assert_int_eq(count, 1);
    ck_assert_int_eq(list_contains(out, "endfile.exe", count), 1);
    ck_assert_int_eq(list_contains(out, "fun_img.png", count), 0);
    ck_assert_int_eq(list_contains(out, "grades.pdf", count), 0);
    ck_assert_int_eq(list_contains(out, "text.txt", count), 0);

    for (unsigned i = 0; i < count; i++)
    {
        free(out[i]);
    }
    free(out);

    lab3_readdir("/sub", &out, &count);
    ck_assert_ptr_ne(out, NULL);
    ck_assert_int_eq(count, 2);
    ck_assert_int_eq(list_contains(out, "exec", count), 1);
    ck_assert_int_eq(list_contains(out, "secret.txt", count), 1);
    ck_assert_int_eq(list_contains(out, "endfile.exe", count), 0);
    ck_assert_int_eq(list_contains(out, "fun_img.png", count), 0);
    ck_assert_int_eq(list_contains(out, "grades.pdf", count), 0);
    ck_assert_int_eq(list_contains(out, "text.txt", count), 0);

    for (unsigned i = 0; i < count; i++)
    {
        free(out[i]);
    }
    free(out);

    close_emu_disk();
}
END_TEST

START_TEST(fails_on_purpose)
{
    ck_assert_int_eq(open_emu_disk("vmem.dump"), 0);

    char **out = NULL;
    void *empty = NULL;
    uint32_t count = 0;
    // Fail to read files as directories
    ck_assert_int_eq(lab3_readdir("/key.hex", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/text.txt", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/add/final_entry/endfile.exe", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/sub/exec", &out, &count), -1);

    //Fail on non existing files
    ck_assert_int_eq(lab3_readdir("/sub/keys", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/sub/execc", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/asdf", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/lol.pdf", &out, &count), -1);

    //Null params
    ck_assert_int_eq(lab3_readdir(empty, &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("/sub/exec", empty, &count), -1);
    ck_assert_int_eq(lab3_readdir("/lol.pdf", &out, empty), -1);

    //relative path
    ck_assert_int_eq(lab3_readdir("key.hex", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("add/final_entry/endfile", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("lol.pdf", &out, &count), -1);
    ck_assert_int_eq(lab3_readdir("su/exec", &out, &count), -1);

    close_emu_disk();
}
END_TEST

START_TEST(lab3_open_multiple_files)
{

    ck_assert_int_eq(open_emu_disk("vmem.dump"), 0);

    // CHECK THAT FAILS WHEN PRECONDITIONS ARE NOT MET
    ck_assert_int_eq(lab3_open(NULL), -1);
    ck_assert_int_eq(lab3_open("/add"), -1);
    ck_assert_int_eq(lab3_open("/add/final_entry"), -1);
    ck_assert_int_eq(lab3_open("/sub"), -1);
    ck_assert_int_eq(lab3_open("/"), -1);

    ck_assert_int_eq(lab3_open("add"), -1);
    ck_assert_int_eq(lab3_open("add/final_entry"), -1);
    ck_assert_int_eq(lab3_open("sub"), -1);
    ck_assert_int_eq(lab3_open("key.hex"), -1);

    ck_assert_int_eq(lab3_open("/lol.pdf"), -1);
    ck_assert_int_eq(lab3_open("/sub/exex.exe"), -1);

    int fd0 = lab3_open("/add/final_entry/endfile.exe");
    int fd1 = lab3_open("/key.hex");
    int fd2 = lab3_open("/text.txt");
    int fd3 = lab3_open("/add/fun_img.png");
    int fd4 = lab3_open("/add/grades.pdf");
    int fd5 = lab3_open("/sub/exec");
    int fd6 = lab3_open("/sub/secret.txt");

    ck_assert_int_ne(fd0, -1);
    ck_assert_int_ne(fd1, -1);
    ck_assert_int_ne(fd2, -1);
    ck_assert_int_ne(fd3, -1);
    ck_assert_int_ne(fd4, -1);
    ck_assert_int_ne(fd5, -1);
    ck_assert_int_ne(fd6, -1);
    ck_assert_int_eq(lab3_open("/add/final_entry/endfile.exe"), -1);
    ck_assert_int_eq(lab3_open("/key.hex"), -1);
    ck_assert_int_eq(lab3_open("/text.txt"), -1);
    ck_assert_int_eq(lab3_open("/add/fun_img.png"), -1);
    ck_assert_int_eq(lab3_open("/add/grades.pdf"), -1);
    ck_assert_int_eq(lab3_open("/sub/exec"), -1);
    ck_assert_int_eq(lab3_open("/sub/secret.txt"), -1);

    unsigned count = 0;
    for (unsigned i = 0; i < MAX_OPEN_FILES; i++)
    {
        int res = lab3_close(i);
        if (res == 0)
        {
            count++;
        }
    }
    ck_assert_int_eq(count, 7);

    count = 0;
    for (unsigned i = MAX_OPEN_FILES; i != 0;)
    {
        i--;
        int res = lab3_close(i);
        if (res == 0)
        {
            count++;
        }
    }
    ck_assert_int_eq(count, 0);

    close_emu_disk();
}
END_TEST

START_TEST(read_single_block_file_and_seek)
{
    ck_assert_int_eq(open_emu_disk("vmem.dump"), 0);

    int fd0 = lab3_open("/add/final_entry/endfile.exe");
    char buffer[150];
    memset(&buffer, 0, sizeof(char) * 150);

    int read = lab3_read(fd0, &buffer, 6);
    ck_assert(strcmp(buffer, "Line 0") == 0);
    ck_assert_int_eq(read, 6);
    memset(buffer, 0, 150);
    read = lab3_read(fd0, &buffer, 7);
    ck_assert(strcmp(buffer, "\nLine 1") == 0);
    ck_assert_int_eq(read, 7);
    ck_assert_int_eq(open_file_table[fd0].seek_offset, 13);

    read = lab3_read(fd0, &buffer, 7);
    ck_assert_int_eq(read, 7);
    read = lab3_read(fd0, &buffer, 7);

    ck_assert_int_eq(read, 7);
    ck_assert_str_eq(buffer, "\nLine 3");
    ck_assert_int_eq(open_file_table[fd0].seek_offset, 13 + 14);
    ck_assert_int_eq(lab3_seek(fd0, 100000), -1);

    ck_assert_int_eq(lab3_seek(fd0, 70), 0);
    read = lab3_read(fd0, &buffer, 7);
    ck_assert_int_eq(read, 7);
    ck_assert_str_eq(buffer, "Line 10");

    close_emu_disk();
}
END_TEST

START_TEST(read_multiple_block_file_and_seek)
{
    ck_assert_int_eq(open_emu_disk("vmem.dump"), 0);

    int fd0 = lab3_open("/large.txt");

    ck_assert_int_eq(lab3_seek(fd0, 4096), 0);
    char buffer[150];
    memset(&buffer, 0, 150);
    int read = lab3_read(fd0, &buffer, 58);
    ck_assert_int_eq(read, 58);
    ck_assert_str_eq(buffer, "This is first data on second block: Hope it works to read!");

    //Cross blocks for one block
    ck_assert_int_eq(lab3_seek(fd0, 4088), 0);
    ck_assert_int_eq(lab3_read(fd0, buffer, 66), 66);
    ck_assert_str_eq(buffer, "********This is first data on second block: Hope it works to read!");

    //Try with larger file
    int fd1 = lab3_open("/very_large.txt");
    char large_buffer[12289];
    memset(&large_buffer, 0, 12289);
    read = lab3_read(fd1, &large_buffer, 12288);
    ck_assert_int_eq(read, 12288);

    for (unsigned i = 0; i < 12288; i++)
    {
        ck_assert(large_buffer[i] == '*');
    }

    memset(&large_buffer, 0, 12289);
    size_t to_read = 122 + 4096 * 2 + 25 + 78;
    lab3_seek(fd1, 4096 - 122);
    read = lab3_read(fd1, &large_buffer, to_read);

    for (unsigned i = 0; i < 122 + 4096 * 2; i++)
    {
        ck_assert(large_buffer[i] == '*');
    }
    ck_assert_str_eq(&(large_buffer[122 + 4096 * 2]), "hhhhhhhhhhhhhhhhhhhhhhhhhThere is exactly 25 h before this sentence in the 4th memory block of the file");

    lab3_close(fd0);
    lab3_close(fd1);
    close_emu_disk();
}
END_TEST

int main()
{
    Suite *s = suite_create("File System Custom Tests");
    TCase *tc1 = tcase_create("testcase");
    suite_add_tcase(s, tc1);

    tcase_add_test(tc1, test_find_inode_by_path);
    tcase_add_test(tc1, readdir_test_files_all_present);
    tcase_add_test(tc1, fails_on_purpose);
    tcase_add_test(tc1, lab3_open_multiple_files);
    tcase_add_test(tc1, read_single_block_file_and_seek);
    tcase_add_test(tc1, read_multiple_block_file_and_seek);

    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_VERBOSE);

    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
