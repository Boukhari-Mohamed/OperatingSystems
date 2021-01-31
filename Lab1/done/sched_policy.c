/**
 * @file sched_policy.c
 * @brief Implementations for custom schedulers
 *
 * @author Atri Bhattacharyya, Mark Sutherland
 */
#include <stdio.h>
#include <stdlib.h>
#include "sched_policy.h"
#include "schedule.h"
//========================================================================================
/** Round robin just returns the oldest thread in RUNNABLE state */
l1_thread_info *l1_round_robin_policy(l1_thread_info *prev, l1_thread_info *next)
{
  if (next != NULL)
  {
    return next;
  }
  l1_scheduler_info *scheduler = get_scheduler();
  return thread_list_rotate(&scheduler->thread_arrays[RUNNABLE]);
}
//========================================================================================
/** Find thread with smallest total run time and return it. NULL if list is empty*/
l1_thread_info *l1_find_smallest_cycle(l1_thread_list *list)
{
  if (list->head == NULL)
  {
    return NULL;
  }

  l1_thread_info *current = list->head;
  l1_thread_info *smallest_cycles = current;
  while (current->next != NULL)
  {
    current = current->next;
    if (l1_time_is_smaller(current->total_time, smallest_cycles->total_time))
    {
      smallest_cycles = current;
    }
  }
  return smallest_cycles;
}
//========================================================================================
/** Schedules the thread with the smallest amount of cycles so far */
l1_thread_info *l1_smallest_cycles_policy(l1_thread_info *prev, l1_thread_info *next)
{
  if (next != NULL)
  {
    return next;
  }

  l1_scheduler_info *scheduler = get_scheduler();
  return l1_find_smallest_cycle(&scheduler->thread_arrays[RUNNABLE]);
}
//========================================================================================
/**
 * Find next thread to run in RR and make sure it is at back of list before running
 * (To achieve Round Robin, need min at tail of list)
 */
l1_thread_info *l1_mlfq_next_thread(l1_thread_list *list)
{
  if (list == NULL)
  {
    return NULL;
  }

  l1_thread_info *max = list->head;
  l1_thread_info *current = list->head;

  const l1_thread_info *const tail = list->tail;

  //Search for min, until again in original disposition
  while (current != tail)
  {
    current = current->next;
    if (current->priority_level > max->priority_level)
    {
      max = current;
    }
  }
  //No need to set current at back: done in schedule after running
  return max;
}
//========================================================================================
/** increase priority of threads that dit not run at their current priority */
void l1_mlfq_boost(l1_thread_list *list)
{
  if (list == NULL)
  {
    return;
  }

  const l1_thread_info *const tail = list->tail;
  l1_thread_info *current = list->head;

  while (current != tail)
  {
    if (!current->got_scheduled)
    {
      l1_time_init(&current->total_time);
      l1_priority_increase(&current->priority_level);
    }
    current = current->next;
  }
}
//========================================================================================
/** Schedules threads according to mlfq policy */
l1_thread_info *l1_mlfq_policy(l1_thread_info *prev, l1_thread_info *next)
{
  l1_scheduler_info *scheduler = get_scheduler();
  l1_thread_list *runnable_list = &scheduler->thread_arrays[RUNNABLE];
  //Boost threads that didn't run when SCHED_PERDIOD wraps (i.e ran SCHED_PERIOD threads)
  if (scheduler->sched_ticks == 0)
  {
    l1_mlfq_boost(runnable_list);
  }

  //Check whole time slice use
  l1_time curr_slice_time;
  l1_time_init(&curr_slice_time);
  l1_time_diff(&curr_slice_time, prev->slice_end, prev->slice_start);
  const int used_whole_slice = l1_time_is_smaller(
      l1_priority_slice_size(prev->priority_level), curr_slice_time);

  //Check total run time use
  const int above_threshold = l1_time_is_smaller(TIME_PRIORITY_THRESHOLD, prev->total_time);

  //Demote thread if needed, update total_time, got_scheduled, ...
  if (used_whole_slice || above_threshold)
  {
    l1_priority_decrease(&prev->priority_level);
    l1_time_init(&prev->total_time);
    prev->got_scheduled = 0;
  }

  //No need to change sched_ticks -> done in schedule
  //No need to set got_schedule -> done in schedule
  return l1_mlfq_next_thread(runnable_list);
}