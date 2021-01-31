/**
 * @file malloc.c
 * @brief Implementations of custom allocators
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "malloc.h"
#include "error.h"

/*********************** Standard GLIBC malloc ***********/
void *libc_malloc(size_t size)
{
  return malloc(size);
}

l1_error libc_free(void *ptr)
{
  free(ptr);

  return SUCCESS;
}
/**********************************************************/

/*********************** Chunk malloc *********************/
/**
 * Pointer to array of char, each "array" reprensenting a chunk.
 * This way, can access chunk i with l1_chunk_arena[i]
 */
char (*l1_chunk_arena)[CHUNK_SIZE];
l1_chunk_desc_t *l1_chunk_meta;
max_align_t l1_region_magic;
//=========================================================
void l1_chunk_init(void)
{
  /* Allocate chunk arena and metadata */
  l1_chunk_arena = malloc(ALLOC8R_HEAP_SIZE);
  /* Allocate space for metadata */
  l1_chunk_meta = calloc(sizeof(l1_chunk_desc_t), CHUNK_ARENA_LENGTH);
  if ((l1_chunk_arena == NULL) || (l1_chunk_meta == NULL))
  {
    //free before exiting
    if (l1_chunk_arena == NULL)
      free(l1_chunk_arena);
    else
      free(l1_chunk_meta);

    printf("Unable to allocate %d bytes for the chunk allocator\n", ALLOC8R_HEAP_SIZE);
    exit(1);
  }

  /* Generate random chunk magic */
  srand(time(NULL));
  for (unsigned i = 0; i < sizeof(max_align_t); ++i)
    *(((char *)&l1_region_magic) + i) = rand();
}
//=========================================================
void l1_chunk_deinit(void)
{
  free(l1_chunk_arena);
  free(l1_chunk_meta);
}
//=========================================================
/**
 * verify that chunks are free, and if not, tells position of last occupied position
 */
unsigned l1_verify_chunks_free(size_t from, size_t len, size_t *last_occupied)
{
  unsigned is_free = 1;
  size_t const bound = from + len;
  for (size_t i = from; i < bound; i++)
  {
    if (IS_CHUNK_TAKEN(i))
    {
      is_free = 0; /*This chunk cannot be used */
      *last_occupied = i;
    }
  }
  return is_free;
}
//=========================================================
/* Create header and copy it in alloc_start's chunk*/
void l1_set_hdr_chunk(size_t alloc_start, size_t size)
{
  l1_region_hdr_t hdr;
  hdr.magic0 = l1_region_magic;
  hdr.magic1 = l1_region_magic;
  hdr.size = size;

  //Instead of for loop
  memcpy((void *)l1_chunk_arena[alloc_start],
         (void *)&hdr,
         sizeof(l1_region_hdr_t));
}
//=========================================================
size_t l1_chunk_count(size_t byte_size)
{
  return byte_size / CHUNK_SIZE +
         /* up rounding */ (byte_size % CHUNK_SIZE != 0 ? 1 : 0)
         /*metadata header */
         + 1;
}
//=========================================================
void *l1_chunk_malloc(size_t size)
{
  if (size == 0)
  {
    return NULL;
  }
  if (size > ALLOC8R_HEAP_SIZE)
  {
    l1_errno = ERRNOMEM;
    return NULL;
  }

  size_t const req_chunk_count = l1_chunk_count(size);
  //Need signed type
  long chunk_bound = CHUNK_ARENA_LENGTH - req_chunk_count + 1;
  size_t alloc_start = 0;
  unsigned can_alloc = 0;

  while (!can_alloc && alloc_start < chunk_bound)
  {
    if (IS_CHUNK_FREE(alloc_start))
    {
      size_t last_occupied; //Set by function when chunks are not free
      if (l1_verify_chunks_free(alloc_start, req_chunk_count, &last_occupied))
      {
        can_alloc = 1;
      }
      else
      {
        //Could not alloc here, so jump to last occupied chunk
        alloc_start = last_occupied + 1;
      }
    }
    else
    {
      alloc_start++;
    }
  }

  if (can_alloc)
  {
    //lock metadata chunks
    size_t lock_bound = alloc_start + req_chunk_count;
    for (size_t s = alloc_start; s < lock_bound; s++)
    {
      SET_CHUNK_TAKEN(s);
    }
    l1_set_hdr_chunk(alloc_start, size);
    return &(l1_chunk_arena[alloc_start + 1]);
  }
  else
  {
    l1_errno = ERRNOMEM;
    return NULL;
  }
}
//=========================================================
/* Compares max_align_t structs */
unsigned l1_align_equals(max_align_t *a0, max_align_t *a1)
{
  //Since created byte per byte, compare it the same way.
  char *ptr0 = (char *)a0;
  char *ptr1 = (char *)a1;

  unsigned eq = 1;
  size_t i = 0;
  while (eq == 1 && i < sizeof(max_align_t))
  {
    if (ptr0[i] != ptr1[i])
    {
      eq = 0;
    }
    i++;
  }
  return eq;
}
//=========================================================
l1_error l1_chunk_free(void *ptr)
{
  if (ptr == NULL)
    return SUCCESS;

  if (ptr < (void *)l1_chunk_arena)
  {
    return ERRINVAL;
  }

  char(*chunks)[CHUNK_SIZE] = ptr; // cast
  char(*header_chunk)[CHUNK_SIZE] = &(chunks[-1]);
  l1_region_hdr_t hdr;
  memcpy(&hdr, header_chunk, sizeof(l1_region_hdr_t)); //Get header content

  if (l1_align_equals(&hdr.magic0, &l1_region_magic) &&
      l1_align_equals(&hdr.magic1, &l1_region_magic))
  {
    //Find position in chunk meta list
    //Chunk count between ptr and start of memory
    size_t start_offset = (header_chunk - l1_chunk_arena);
    size_t chunk_count = l1_chunk_count(hdr.size);
    for (size_t i = 0; i < chunk_count; i++)
    {
      SET_CHUNK_FREE(start_offset + i);
    }
    return SUCCESS;
  }
  else
  {
    return ERRINVAL;
  }
}
/**********************************************************/

/****************** Free list based malloc ****************/
l1_listoc8r_meta *l1_listoc8r_free_head = NULL;
void *l1_listoc8r_heap = NULL;
max_align_t l1_listoc8r_magic;
//=========================================================
void l1_listoc8r_init()
{
  l1_listoc8r_heap = malloc(ALLOC8R_HEAP_SIZE);
  if (l1_listoc8r_heap == NULL)
  {
    printf("Unable to allocate %d bytes for the listoc8r\n", ALLOC8R_HEAP_SIZE);
    exit(1);
  }

  /* Generate random listoc8r magic */
  srand(time(NULL));
  for (unsigned i = 0; i < sizeof(max_align_t); i++)
    *(((char *)&l1_listoc8r_magic) + i) = rand();

  l1_listoc8r_meta first_meta;

  first_meta.capacity = ALLOC8R_HEAP_SIZE - offsetof(l1_listoc8r_meta, next);
  first_meta.magic0 = l1_listoc8r_magic;
  first_meta.magic1 = l1_listoc8r_magic;
  first_meta.next = NULL;
  memcpy(l1_listoc8r_heap, &first_meta, sizeof(l1_listoc8r_meta));
  l1_listoc8r_free_head = l1_listoc8r_heap;
}
//=========================================================
void l1_listoc8r_deinit()
{
  free(l1_listoc8r_heap);
}
//=========================================================
/* rewire list */
void l1_rewire_list(l1_listoc8r_meta *prev, l1_listoc8r_meta *current,
                    l1_listoc8r_meta *new_meta_pos)
{
  if (current == l1_listoc8r_free_head)
  {

    l1_listoc8r_free_head = new_meta_pos;
  }
  else
  {
    prev->next = new_meta_pos;
  }
}
//=========================================================
void *l1_listoc8r_malloc(size_t req_size)
{
  if (req_size == 0)
    return NULL;

  //Round up, no need to take metadata in consideration since its aligned due to its size
  size_t req_chunks = req_size / sizeof(max_align_t) + ((req_size % sizeof(max_align_t) == 0) ? 0 : 1);
  size_t aligned_sz = req_chunks * sizeof(max_align_t);
  //Find next available chunk and its predecessor
  l1_listoc8r_meta *current = l1_listoc8r_free_head;
  l1_listoc8r_meta *prev = NULL;
  while (current != NULL && current->capacity < aligned_sz)
  {
    prev = current;
    current = current->next;
  }

  if (current == NULL)
  {
    return NULL;
  }

  //Cannot fit an other metadata in remaining space
  if (aligned_sz + sizeof(l1_listoc8r_meta) > current->capacity)
  {
    //Rewire list
    l1_rewire_list(prev, current, current->next);
  }
  else
  {
    l1_listoc8r_meta new_meta;
    new_meta.magic1 = l1_listoc8r_magic;
    new_meta.magic0 = l1_listoc8r_magic;
    new_meta.next = current->next;
    new_meta.capacity = current->capacity - aligned_sz - offsetof(l1_listoc8r_meta, next);
    current->capacity = aligned_sz;
    //Compute position of new metadata and copy to mem
    l1_listoc8r_meta *new_meta_pos =
        (l1_listoc8r_meta *)(((char *)current) + offsetof(l1_listoc8r_meta, next) + aligned_sz);

    memcpy(new_meta_pos, &new_meta, sizeof(l1_listoc8r_meta));
    l1_rewire_list(prev, current, new_meta_pos);
  }

  current->next = NULL;
  return (void *)(((char *)current) + offsetof(l1_listoc8r_meta, next));
}
//=========================================================
l1_error l1_listoc8r_free(void *ptr)
{
  if (ptr == NULL)
    return SUCCESS;

  l1_listoc8r_meta *ptr_meta = (l1_listoc8r_meta *)(((char *)ptr) - offsetof(l1_listoc8r_meta, next));
  //check magic values
  if (!l1_align_equals(&ptr_meta->magic0, &l1_listoc8r_magic) ||
      !l1_align_equals(&ptr_meta->magic1, &l1_listoc8r_magic))
  {
    return ERRINVAL;
  }

  //Only need to add in free list
  ptr_meta->next = l1_listoc8r_free_head;
  l1_listoc8r_free_head = ptr_meta;
  return SUCCESS;
}
