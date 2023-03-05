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

// implementation of malloc
void *mymalloc(size_t s);

// Constants
#define HEADER_SIZE sizeof(header_t)
void *global_base = NULL;

#endif /* _MYMALLOC_H */
