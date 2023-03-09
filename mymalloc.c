#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include "mymalloc.h"

// Include any other headers we need here
#include <sys/mman.h>
#include <unistd.h>

// NOTE: You should NOT include <stdlib.h> in your final implementation
#include <debug.h> // definition of debug_printf

struct header {
  size_t size; // size of free bytes
  header_t *next; // next chunk of free bytes
  int free;
  int magic; // Dubugging only TODO: Delete after debugging!
};

// find free space in free-list
header_t *find_space(header_t **last, size_t s) {
  header_t *curr = global_base;
  while (curr != NULL && !(curr->free && (curr->size + HEADER_SIZE) >= s)) {
    *last = curr;
    curr = curr->next;
  }
  return curr;
}

// request space from the OS and add block to end of the list
header_t *allocate_space(header_t *last, size_t s) {
  header_t *head;
  head = sbrk(0);
  void *req = sbrk(s + HEADER_SIZE);
  if (req == (void *) -1) {
    // failure to request space
    return NULL;
  }
  debug_printf("Header size: %ld\n", HEADER_SIZE);
  if (last) {
    last->next = head;
  }
  head->size = s;
  head->next = NULL;
  head->free = 0;
  head->magic = 0x12345678;
  return head;
}

// gets the pointer of the meta data of the chunk of memory
header_t *get_header(void *ptr) {
  assert(ptr);
  return (header_t *) ptr - HEADER_SIZE;
}

// splits the free chunk into a chunk of size s
header_t *split_chunk(header_t *chunk, size_t s) {
  assert(chunk != NULL);
  assert(chunk->size + HEADER_SIZE > s);  // assumes the chunk is big enough for the given size
  assert(chunk->free == 1); // assumes the chunk is also free

  size_t remaining = chunk->size - (s + HEADER_SIZE);

  header_t *new_chunk = chunk;
  chunk = chunk + s + HEADER_SIZE; // change the chunk pointer to its new pointer
  memcpy(new_chunk
  
}

// allocates memory in the heap with the given number of bytes and returns the pointer
void *mymalloc(size_t s) {
  header_t *chunk;
  
  // checks if the size is valid
  if(s <= 0) { return NULL; }
  
  // first allocation call and initializes the free list
  if (!global_base) {
    // allocates memory of given size
    chunk = allocate_space(NULL, s);
    // returns NULL is OS cannot allocate enough space
    if (!chunk) { return NULL; }

    // sets the head of the free chunk to the global base
    global_base = chunk;
  }
  else {
    // go to the last place of space
    header_t *last = global_base;
    chunk = find_space(&last, s);
    if (!chunk) {
      chunk = allocate_space(last, s);
      if (!chunk) { return NULL; }
    } else {
      chunk->free = 0;
      chunk->magic = 0x77777777; // debug status: used mem
    }
  }

 // debug_printf("malloc %zu bytes\n", s);
  return (chunk + HEADER_SIZE); // returns the actual memory part
}

void *mycalloc(size_t nmemb, size_t s) {

  void *p = mymalloc(nmemb * s);
  
  if (!p) { return NULL; }
 // debug_printf("calloc %zu bytes\n", s);
  memset(p, 0, s); //fills array with 0
  return p;
}

void myfree(void *ptr) {
  assert(ptr);
  
  // TODO: merge blocks once splitting blocks is implemented.
  header_t *chunk_ptr = get_header(ptr);
  assert(chunk_ptr->free == 0);
  assert(chunk_ptr->magic == 0x77777777 || chunk_ptr->magic == 0x12345678);

 // debug_printf("Freed %zu bytes\n", chunk_ptr->size);
  chunk_ptr->free = 1;
  chunk_ptr->magic = 0x55555555;
}
