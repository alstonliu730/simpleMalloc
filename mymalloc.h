#ifndef _MYMALLOC_H
#define _MYMALLOC_H

// Template of the implementation of malloc
#include <sys/mman.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// data structure for free list
typedef struct header header_t;

header_t *find_space(header_t **last, size_t s);

header_t *allocate_space(header_t *last, size_t s);

header_t *get_chunk(void *ptr);

header_t *split_chunk(header_t *last, size_t s);

// implementation of malloc
void *mymalloc(size_t s);

// implementation of calloc
void *mycalloc(size_t nmemb, size_t s);

// implementation of free
void myfree(void* ptr);

// Constants
#define HEADER_SIZE sizeof(header_t)
void *global_base = NULL;

#endif /* _MYMALLOC_H */
