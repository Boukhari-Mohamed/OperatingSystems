/**
 * @file tests.c
 * @brief Unit-test suite for week 1 exercises
 *
 * A simple test suite for week 1 exercises. Provides a template which
 * students should add tests to.
 *
 * @author Atri Bhattacharyya, Adrien Ghosn
 */
#include <check.h>
#include <stdlib.h>
#include "week01.h"
#include <string.h>

/* This is an example of using the unit testing framework `check`.
 * There are two components to each test:
 * - The test definition: This contains the work to be done during the test
 *   and contains `assert`s to check that specific conditions hold at runtime.
 *   Test definitions are written using the `START_TEST()` and `END_TEST` 
 *   macros. An example is shown below (`example_test`).
 * - Adding the test to the test suite: This tells the framework to which 
 *   tests to run. For example, the line `tcase_add_test(tc1, example_test);`
 *   tells the framework to run the example test.
 *
 * You are strongly encouraged to use the `check` framework to test your code.
 * Comprehensive documentation for the framework is available at
 * `https://libcheck.github.io/check/`.
 * Define further tests using the `START_TEST`, `END_TEST` macros and add them
 * to the test suite in `main()`.
 */

/* This test fails if ANY of the asserts fails */

int same_sign(int i, int j)
{
    if ((i > 0 && j > 0) || (i < 0 && j < 0) || (i == 0 && j == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

START_TEST(example_test)
{

    /* Example integer test. 
     * This checks whether the variable `num` is less than 1000 at runtime. */
    int num = 1;
    ck_assert_int_le(num, 1000);

    /* Example pointer check.
     * This checks whether the variable `new_node` is NULL at runtime. */
    w1_node *new_node = w1_create_node(0);
    ck_assert_ptr_ne(new_node, NULL);

    /* Testing insertion into linked list.
     * A `NULL` head pointer signifies an empty node. 
     * Inserting a node into an empty list at position 0 makes the 
     * new node the head. This is checked by the assert statement. */
    w1_node *head = NULL, *new_head;
    new_head = w1_insert_node(head, new_node, 0);
    ck_assert_ptr_eq(new_head, new_node);

    /* Clean-up */
    w1_delete_node(new_node);
}
END_TEST

START_TEST(strcmp_test0)
{
    char const *s1 = "abcd";
    char const *s2 = "abcd";

    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test1)
{
    char const *s1 = "abb";
    char const *s2 = "abcd";

    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test2)
{
    char const *s1 = "jkjkjjqkjsakjkj";
    char const *s2 = "123dnmnmmn3";
    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test3)
{
    char const *s1 = "AJanamdsKJAW";
    char const *s2 = "admWM,madsnmam,we";
    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test4)
{
    char const *s1 = "";
    char const *s2 = "abcd";
    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test5)
{
    char const *s1 = "ABCD";
    char const *s2 = "abcd";
    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test6)
{
    char const *s1 = "";
    char const *s2 = "";
    ck_assert_int_eq(strcmp(s1, s2), w1_strcmp(s1, s2));
}
END_TEST

START_TEST(strcmp_test7)
{
    char const *s1 = "ABC";
    char const *s2 = "bcd";
    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST

START_TEST(strcmp_test8)
{
    char const *s1 = "ABC";
    char const *s2 = "";
    ck_assert_int_eq(same_sign(strcmp(s1, s2), w1_strcmp(s1, s2)), 1);
}
END_TEST


START_TEST(test_list)
{
    w1_node *n0 = w1_create_node(12);
    w1_node *n1 = w1_create_node(9);
    w1_node *n2 = w1_create_node(4);
    w1_node *n3 = w1_create_node(6);
    w1_insert_node(n0, n1, 1);
    ck_assert_ptr_eq(n0->next, n1);

    w1_insert_node(n0, n2, 2);
    ck_assert_ptr_eq(n0->next->next, n2);

    w1_insert_node(n0, n3, 3);
    ck_assert_ptr_eq(n0->next->next->next, n3);

    ck_assert_int_eq(w1_size_list(n0), 4);

    free(n0);
    free(n1);
    free(n2);
    free(n3);
}
END_TEST

START_TEST(test_tree)
{
    Node nodes[10];
    nodes[0].data = 7;
    nodes[1].data = 3;
    nodes[2].data = 2;
    nodes[3].data = 12;
    nodes[4].data = 1;
    nodes[5].data = 5;
    nodes[6].data = 10;
    nodes[7].data = 15;
    nodes[8].data = 77;
    nodes[9].data = 9;

    nodes[0].left = NULL;
    nodes[0].right = NULL;

    nodes[1].left = NULL;
    nodes[1].right = NULL;

    nodes[2].left = &nodes[0];
    nodes[2].right = &nodes[1];

    nodes[3].left = &nodes[2];
    nodes[3].right = &nodes[4];

    nodes[4].left = &nodes[5];
    nodes[4].right = &nodes[6];

    nodes[5].left = NULL;
    nodes[5].right = NULL;

    nodes[6].left = NULL;
    nodes[6].right = NULL;

    nodes[7].left = &nodes[3];
    nodes[7].right = &nodes[8];

    nodes[8].left = NULL;
    nodes[8].right = &nodes[9];

    nodes[9].left = NULL;
    nodes[9].right = NULL;

    print_in_order(&nodes[7], stdout);
    printf("\n");
    printf("Expect 7 2 3 12 5 1 10 15 77 9\n");
    print_post_order(&nodes[7], stdout);
    printf("\n");
    printf("Expect 7 3 2 5 10 1 12 9 72\n");
    print_pre_order(&nodes[7], stdout);
    printf("\n");
    printf("Expect 15 12 2 7 3 1 5 7 77 9\n");
}
END_TEST

int main()
{
    Suite *s = suite_create("Week 01 tests");
    TCase *tc1 = tcase_create("basic tests");
    suite_add_tcase(s, tc1);

    TCase *tc2 = tcase_create("Strcmp test");
    suite_add_tcase(s, tc2);

    /* Add the  */
    tcase_add_test(tc1, example_test);
    tcase_add_test(tc2, strcmp_test0);
    tcase_add_test(tc2, strcmp_test1);
    tcase_add_test(tc2, strcmp_test2);
    tcase_add_test(tc2, strcmp_test3);
    tcase_add_test(tc2, strcmp_test4);
    tcase_add_test(tc2, strcmp_test5);
    tcase_add_test(tc2, strcmp_test6);
    tcase_add_test(tc2, strcmp_test7);
    tcase_add_test(tc2, strcmp_test8);

    TCase *tc3 = tcase_create("node test");
    suite_add_tcase(s, tc3);

    tcase_add_test(tc3, test_list);

    TCase *tc4 = tcase_create("Tree tests");
    suite_add_tcase(s, tc4);
    tcase_add_test(tc4, test_tree);

    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_VERBOSE);

    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
