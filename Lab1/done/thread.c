/**
 * @file week03.c
 * @brief Outlines in C for student week 3
 *
 * @author Mark Sutherland
 */
#include <stdlib.h>
#include "schedule.h"
#include "stack.h"
#include "thread.h"
#include "thread_info.h"
#include "priority.h"

/* This is a function that calls a new thread's start_routine and stores the
 * return value in the function's info struct. This function is put on top
 * of the constructed stack for all new threads.
 */
void l1_start(void)
{
  l1_thread_info *cur = get_scheduler()->current;
  /* enter execution */
  void *ret = cur->thread_func(cur->thread_func_args);
  cur->retval = ret;
  cur->state = ZOMBIE;
  /* Let the scheduler do the cleanup */
  yield(-1);
}

l1_error l1_thread_create(l1_tid *thread, void *(*start_routine)(void *), void *arg)
{
  l1_tid new_tid = get_uniq_tid();
  /* Allocate l1_thread_info struct for new thread,
   * allocate stack for the thread. */
  l1_thread_info *fresh_thread = malloc(sizeof(l1_thread_info));
  if (fresh_thread == NULL)
  {
    return ERRNOMEM;
  }
  fresh_thread->id = new_tid;
  fresh_thread->state = RUNNABLE;
  l1_stack *new_stack = l1_stack_new();

  /* Setup stack for new task. At the bottom of the stack is a fake stack 
   * frame for l1_start, as described in the handout. This will allow the 
   * new thread to return to l1_start after it finished working. 
   * Hint: To figure out which registers need to be saved on the stack, and the 
   * order in which they need to be stored, explore the state of the stack in 
   * `switch_asm`. Also, what are the default values required by `l1_start`.
   * Binaries compiled by `gcc` use the System-V ABI which defines which registers
   * are saved by a function's caller and callee. 
   * See https://wiki.osdev.org/System_V_ABI
   * In `switch_asm`, we only save the registers are not already saved.
   */
  //If cannot allocate, set error and return
  if (new_stack == NULL)
  {
    free(fresh_thread);
    return ERRNOMEM;
  }

  //Push empty value on stack
  l1_stack_push(new_stack, (uint64_t)0);
  l1_stack_push(new_stack, (uint64_t)l1_start);

//Push rbp, r15, r14, r13, r12, rbx on stack (6 registers)
#define M_SAVED_REGS_COUNT 6
  for (unsigned i = 0; i < M_SAVED_REGS_COUNT; i++)
  {
    l1_stack_push(new_stack, (uint64_t)0);
  }
#undef M_SAVED_REGS_COUNT

  fresh_thread->thread_stack = new_stack;
  fresh_thread->thread_func = start_routine;
  fresh_thread->thread_func_args = arg;
  fresh_thread->joined_target = -1;
  fresh_thread->join_recv = NULL;

  //Week 4 initializations
  fresh_thread->priority_level = TOP_PRIORITY;
  fresh_thread->got_scheduled = 0;
  l1_time_init(&fresh_thread->total_time);
  l1_time_init(&fresh_thread->slice_start);
  l1_time_init(&fresh_thread->slice_end);

  //Set remaining fields to a default value
  fresh_thread->errno = SUCCESS;

  /* Add the new task for scheduling */
  add_to_scheduler(fresh_thread, RUNNABLE);

  /* Give the user the right thread id */
  *thread = new_tid;

  return SUCCESS;
}

l1_error l1_thread_join(l1_tid target, void **retval)
{
  //Check that thread with tid target exists
  /* Setup necessary metadata and block yourself */
  l1_scheduler_info *sched = get_scheduler();
  l1_thread_info *current_thread = sched->current;

  current_thread->state = BLOCKED;
  current_thread->joined_target = target;
  current_thread->join_recv = retval;
  current_thread->errno = SUCCESS;

  yield(-1);

  //Back when re-scheduled
  current_thread->join_recv = NULL;
  l1_error resulting_errno = current_thread->errno;
  current_thread->errno = SUCCESS;
  current_thread->joined_target = -1;

  return resulting_errno;
}
