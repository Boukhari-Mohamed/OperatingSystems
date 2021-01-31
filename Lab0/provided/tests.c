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
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "week01.h"
#include "week02.h"

/***** BEGIN: WEEK01 Tests *****/

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
START_TEST(w1_example_test) {

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
/***** END: WEEK01 Tests *****/

/***** BEGIN: WEEK02 Tests *****/

/* print_hello prints Hello World */
void print_hello() {
    printf("Hello world, %d\n", getpid());
}

START_TEST(w2_example_test) {
    /* This should call print_hello twice,
     * printing "Hello world, ____" with different PIDs */
    bool root[] = {true, true, false, false, false};
    w2_fork(root, 0, print_hello);
}
END_TEST
/***** END: WEEK02 Tests *****/

int main() 
{ 
    Suite* s = suite_create("Lab 0 tests");

    /***** BEGIN: WEEK01 *****/
    TCase *tc1 = tcase_create("Week 1 tests"); 
    suite_add_tcase(s, tc1);

    /* Add the tests */
    tcase_add_test(tc1, w1_example_test);
    /***** END: WEEK01 *****/

    /***** BEGIN: WEEK02 *****/
    TCase *tc2 = tcase_create("Week 2 tests"); 
    suite_add_tcase(s, tc2);

    /* Add the tests */
    tcase_add_test(tc2, w2_example_test);
    /***** END: WEEK02 *****/

    SRunner *sr = srunner_create(s); 
    srunner_run_all(sr, CK_VERBOSE); 
 
    int number_failed = srunner_ntests_failed(sr); 
    srunner_free(sr); 
 
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE; 
}
