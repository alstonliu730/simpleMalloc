#ifndef _MYMALLOC_H
#define _MYMALLOC_H

// Template of the implementation of malloc
#include <sys/mman.h>
#include "malloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// data structure for free list
typedef struct header header_t;

// attempts to find free space in the free list
header_t *find_space(size_t s);

// request space from the OS and add block to end of the list
header_t *allocate_space(header_t *last, size_t s);

// gets the pointer of the meta data of the memory chunk
header_t *get_header(void *ptr);

// splits the free chunk into a chunk of size s and the remaining bytes
// returns the split chunk 
header_t *split_chunk(header_t *last, size_t s);

// attempts to combine adjacent chunks of free memory 
void *coalesce(header_t *last);

// combines the given chunks assuming that mem1 is before mem2 in free list
header_t *combine(header_t *mem1, header_t *mem2);

// Constants
#define HEADER_SIZE sizeof(header_t) // size of the header
void *global_base = NULL; // head of the free list
void *global_end = NULL; // end of the free list
#endif /* _MYMALLOC_H */
