/**
 * @file test_malloc.c
 * @brief Unit-test suite for week 5
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#include <check.h>
#include <stdio.h>
#include <stdbool.h>
#include "malloc.h"

void *(*l1_malloc)(size_t) = libc_malloc;
l1_error (*l1_free)(void *) = libc_free;
void (*l1_init)(void) = NULL;
void (*l1_deinit)(void) = NULL;

typedef struct test_struct
{
  int value0;
  double vale1;
  struct test_struct *inner_struct;

} test_struct;

START_TEST(chunk_malloc_test_1)
{
  /* This will test the chunk allocator */
  l1_init = l1_chunk_init;
  l1_deinit = l1_chunk_deinit;
  l1_malloc = l1_chunk_malloc;
  l1_free = l1_chunk_free;

  l1_init();
  void *ptr = l1_malloc(0);
  ck_assert_ptr_eq(ptr, NULL);
  l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_2)
{
  /* This will test the chunk allocator */
  l1_init = l1_chunk_init;
  l1_deinit = l1_chunk_deinit;
  l1_malloc = l1_chunk_malloc;
  l1_free = l1_chunk_free;

  l1_init();

  l1_chunk_desc_t *meta = l1_chunk_meta;

  int *first_int = (int *)l1_malloc(sizeof(int));
  double *sec_double = (double *)l1_malloc(sizeof(double));
  void *empty = l1_malloc(3 * CHUNK_SIZE);
  test_struct *t0 = (test_struct *)l1_malloc(sizeof(test_struct));
  test_struct *t1 = (test_struct *)l1_malloc(sizeof(test_struct));

  *first_int = 99;
  *sec_double = 12.25;
  *((size_t *)empty) = 998;
  t0->inner_struct = t1;
  t0->vale1 = 99.123;
  t0->value0 = -99;

  t1->inner_struct = t0;
  t1->value0 = -999;
  t0->vale1 = 999.999;

  // Test allocation
  ck_assert_ptr_ne(NULL, first_int);
  ck_assert_ptr_ne(NULL, sec_double);
  ck_assert_ptr_ne(NULL, empty);
  ck_assert_ptr_ne(NULL, t0);
  ck_assert_ptr_ne(NULL, t1);

  //Check memory state

  for (size_t i = 0; i < CHUNK_ARENA_LENGTH / 2; i++)
  {
    if (i < 12 /*because of headers */)
    {
      ck_assert_int_eq(meta[i], 1);
    }
    else
    {
      ck_assert_int_eq(meta[i], 0);
    }
  }

  //Test free
  l1_error free_err;

  free_err = l1_free(empty);
  ck_assert_int_eq((int)SUCCESS, (int)free_err);

  for (size_t i = 0; i < CHUNK_ARENA_LENGTH / 2; i++)
  {
    if (i > 11 || (i < 8 && i >= 4))
    {
      ck_assert_int_eq(meta[i], 0);
    }
    else
    {
      ck_assert_int_eq(meta[i], 1);
    }
  }

  empty = l1_malloc(4 * CHUNK_SIZE);
  for (size_t i = 0; i < CHUNK_ARENA_LENGTH / 2; i++)
  {
    if (i > 16 || (i < 8 && i >= 4))
    {
      ck_assert_int_eq(meta[i], 0);
    }
    else
    {
      ck_assert_int_eq(meta[i], 1);
    }
  }

  free_err = l1_free(empty);
  ck_assert_int_eq((int)SUCCESS, (int)free_err);

  free_err = l1_free(first_int);
  ck_assert_int_eq((int)SUCCESS, (int)free_err);

  free_err = l1_free(sec_double);
  ck_assert_int_eq((int)SUCCESS, (int)free_err);

  free_err = l1_free(t0);
  ck_assert_int_eq((int)SUCCESS, (int)free_err);

  free_err = l1_free(t1);
  ck_assert_int_eq((int)SUCCESS, (int)free_err);

  for (size_t i = 0; i < CHUNK_ARENA_LENGTH / 2; i++)
  {
    ck_assert_int_eq(meta[i], 0);
  }

  l1_deinit();
}
END_TEST

/* Test malloc with 0 size returns NULL */
START_TEST(list_malloc_test1)
{
  /* This will test the listoc8r allocator */
  l1_init = l1_listoc8r_init;
  l1_deinit = l1_listoc8r_deinit;
  l1_malloc = l1_listoc8r_malloc;
  l1_free = l1_listoc8r_free;

  l1_init();
  ck_assert_ptr_eq(l1_malloc(0), NULL);
  l1_deinit();
}
END_TEST

size_t list_len()
{
  l1_listoc8r_meta *curr = l1_listoc8r_free_head;
  size_t len = 0;
  while (curr != NULL)
  {
    curr = curr->next;
    len++;
  }
  return len;
}

START_TEST(list_malloc_test2)
{
  l1_init = l1_listoc8r_init;
  l1_deinit = l1_listoc8r_deinit;
  l1_malloc = l1_listoc8r_malloc;
  l1_free = l1_listoc8r_free;

  l1_init();

  char *mem_start = (char *)l1_listoc8r_free_head;
  ck_assert_int_eq(l1_listoc8r_free_head->capacity,
                   ALLOC8R_HEAP_SIZE - offsetof(l1_listoc8r_meta, next));

  ck_assert_int_eq(1, list_len());

  int *ptr0 = l1_malloc(sizeof(int));                      //^32
  double *ptr1 = l1_malloc(5 * sizeof(double));            //^64
  test_struct *ptr2 = l1_malloc(2 * sizeof(test_struct));  //^64
  test_struct *ptr3 = l1_malloc(10 * sizeof(test_struct)); //^256

  *ptr0 = INT32_MAX;
  *ptr1 = -9999.123;
  ptr1[4] = 91923.99132;
  ptr2[0].value0 = 99;
  ptr2[0].vale1 = 9913.1239921;
  ptr2[0].inner_struct = &(ptr2[1]);

  ck_assert_int_eq(*ptr0, INT32_MAX);
  ck_assert_int_eq(*ptr1, -9999.123);
  ck_assert_int_eq(ptr1[4], 91923.99132);

  size_t alloc_len = (size_t)l1_listoc8r_free_head - (size_t)mem_start;
  ck_assert_int_eq(alloc_len, 416 + 4 * offsetof(l1_listoc8r_meta, next));
  ck_assert_int_eq(1, list_len());

  l1_error err;
  err = l1_free(ptr2);
  ck_assert_int_eq((int)SUCCESS, (int)err);
  ck_assert_int_eq(2, list_len());

  alloc_len = (size_t)l1_listoc8r_free_head - (size_t)mem_start;
  ck_assert_int_eq(alloc_len, 96 + 2 * offsetof(l1_listoc8r_meta, next));
  ptr2 = l1_malloc(64);

  alloc_len = (size_t)l1_listoc8r_free_head - (size_t)mem_start;
  ck_assert_int_eq(alloc_len, 416 + 4 * offsetof(l1_listoc8r_meta, next));
  l1_free(ptr2);
  ck_assert_int_eq((int)SUCCESS, (int)err);

  ptr2 = l1_malloc(65);
  alloc_len = (size_t)l1_listoc8r_free_head->next - (size_t)mem_start;
  ck_assert_int_eq(alloc_len, 512 + 5 * offsetof(l1_listoc8r_meta, next));
  l1_free(ptr2);
  ck_assert_int_eq((int)SUCCESS, (int)err);

  err = l1_free(ptr1);
  ck_assert_int_eq((int)SUCCESS, (int)err);
  ck_assert_int_eq(4, list_len());

  err = l1_free(ptr3);
  ck_assert_int_eq((int)SUCCESS, (int)err);
  ck_assert_int_eq(5, list_len());

  err = l1_free(ptr0);
  ck_assert_int_eq((int)SUCCESS, (int)err);
  ck_assert_int_eq(6, list_len());

  l1_deinit();
}
END_TEST

int main(int argc, char **argv)
{
  Suite *s = suite_create("Threading lab");
  TCase *tc1 = tcase_create("basic tests");
  suite_add_tcase(s, tc1);

  tcase_add_test(tc1, chunk_malloc_test_1);
  tcase_add_test(tc1, chunk_malloc_test_2);

  tcase_add_test(tc1, list_malloc_test1);
  tcase_add_test(tc1, list_malloc_test2);

  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_VERBOSE);

  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
