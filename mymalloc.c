#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include "mymalloc.h"

// Include any other headers we need here
#include <sys/mman.h>
#include <unistd.h>

// NOTE: You should NOT include <stdlib.h> in your final implementation
#include <debug.h> // definition of debug_printf

// data structure for free list
struct header {
  size_t size; // size of free bytes
  header_t *next; // next chunk of free bytes
  header_t *prev; // previous chunk of free bytes
  int free; // status of chunk
  int magic; // Dubugging only
};

// attempts to find free space in the free list
header_t *find_space(size_t s) {
  assert(s);

  // gets the head of the free list
  header_t *curr = global_base;

  // find the first block that fits the size
  while(curr && !(curr->free && curr->size >= s)) {
    // advance to the next block
    curr = curr->next;
  }

  return curr;
}

// request space from the OS and add block to end of the list
header_t *allocate_space(header_t *last, size_t s) {
  header_t *head;
  void *req;

  // request OS for memory
  req = sbrk(s + HEADER_SIZE); 
  if (req == (void *) -1) {
    // failed to request space
    debug_printf("FAILED TO REQUEST SPACE!\n");
    return NULL;
  } else { head = (header_t *) req; }
  
  // if given a previous list, add it to end
  if(last) { 
    last->next = head; head->prev = last; 
  } else {
    head->prev = NULL; //head would not have a previous pointer
  }

  // Update this chunks meta data
  head->size = s;
  head->next = NULL;
  head->free = 1; 
  head->magic = 0x77777777; //debug: allocated memory from OS
  
  // update global end of list
  global_end = head;
  
  // return the pointer to the chunk
  return head;
}
  

// gets the pointer of the meta data of the chunk of memory
header_t *get_header(void *ptr) {
  assert(ptr);
  return (header_t *)ptr - 1;
}

// splits the free chunk into a chunk of size s and the remaining bytes
// returns the split chunk
header_t *split_chunk(header_t *chunk, size_t s) {
  assert(chunk);
  assert(chunk->free);
  
  // prepare size of the chunks
  size_t claim = s + HEADER_SIZE;
  size_t remaining = chunk->size - claim;

  // prepare chunk new addresses
  header_t *new_chunk = chunk; // new chunk becomes old chunk address
  header_t *old_chunk = (header_t *) ((char *) chunk + claim); // old chunk moves "claim" bytes ahead

  // debug information
  /*
  debug_printf("-----------\n");
  debug_printf("Splitting chunk from source: %p, claim: %zu, remaining: %zu\n", chunk, claim, remaining);
  debug_printf("New Chunk stats:\n - size: %zu\n - addr: %p\n", claim, new_chunk);
  debug_printf("Old Chunk stats:\n - size: %zu\n - addr: %p\n", remaining, old_chunk);
  debug_printf("-----------\n"); */

  // prepare the new chunk from the given chunk
  new_chunk->size = claim;
  new_chunk->free = 0;
  new_chunk->magic = 0x77771234; //debug: actual split chunk
  
  // update the chunk status
  old_chunk->size = remaining; 
  old_chunk->free = 1;
  old_chunk->magic = 0x12347777; //debug: source for split chunk

  // update positioning of chunks
  old_chunk->next = chunk->next; // old -> (old->next)
  if(old_chunk->next) { old_chunk->next->prev = old_chunk; }
  else {global_end = old_chunk;}

  new_chunk->next = old_chunk; // new -> old
  old_chunk->prev = new_chunk; // old -> new

  debug_printf("Split success!\n");
  return new_chunk;
}


// combines adjacent free chunks given previous chunk
void *coalesce(header_t *last) {
  assert(last);
  assert(last->free == 1);

  header_t *ans;
  // check if previous chunk is free
  if (last->prev && last->prev->free) {
    // combine chunk
    ans = combine(last->prev, last);
  } else {
    ans = last;
  }

  // check if next chunk is free
  if (ans->next && ans->next->free) {
    // combine chunk
    ans = combine(ans, ans->next);
  } 

  // update global pointers
  if (!ans->next) { global_end = ans; }
  return ans;
}

// combines the given chunks assuming that mem1 is before mem2 in free list
header_t *combine(header_t *mem1, header_t *mem2) {
  // Assumes both are free and are not null
  assert(mem1);
  assert(mem2);
  assert(mem1->free);
  assert(mem2->free);

  // assuming mem1 come before mem2
  size_t combined_s = mem1->size + mem2->size + HEADER_SIZE;
  debug_printf("Combining %zu bytes from mem2: %p (%zu bytes) into mem1: %p (%zu bytes). Total combined bytes: %zu\n", 
    (mem2->size + HEADER_SIZE), mem2, mem2->size, mem1, mem1->size, combined_s);
  // update the combined chunk by adding it to the first memory
  mem1->size = combined_s;
  mem1->next = mem2->next;

  // the case where mem2 was the last item in free list
  if(!mem2->next) {
    global_end = mem1;
  } else {
    mem2->next->prev = mem1;
  }
  
  mem1->magic = 0x55555555; //debug: combined chunk
  mem2->magic = 0x88888888; //debug: cannot be used (combined into a chunk)
  
  return mem1;
}

// allocates memory in the heap with the given number of bytes and returns the pointer
void *mymalloc(size_t s) {
  header_t *chunk;

  // first allocation call and initializes the free list
  if (!global_base) {
    // allocates memory of given size from OS
    chunk = allocate_space(NULL, s);
    // returns NULL if OS cannot allocate enough space
    if (!chunk) { debug_printf("OS cannot allocate space."); return NULL; }

    // set the head of the free list to the global base
    global_base = chunk;

  } else {
    chunk = find_space(s);

    // case 1: could not find a free block
    if (!chunk) {
      header_t *end = global_end;
      chunk = allocate_space(end, s);
      if (!chunk) { debug_printf("OS cannot allocate space."); return NULL; }
    } 

    // case 2: found a free block
    else {
      if(chunk->size > (s + HEADER_SIZE)) {
        chunk = split_chunk(chunk, s);
      }
    }
  }
  
  // update chunk status
  chunk->free = 0;
  chunk->magic = 0x66666666; //debug status: used memory (malloc)

  // return actual memory part
  return (chunk + 1);
}

// allocates memory in the heap as an array given number of items and size of each item
void *mycalloc(size_t nmemb, size_t s) {

  // get data pointer
  void *p = mymalloc(nmemb * s);
  
  if (!p) { return NULL; }
  // debug_printf("calloc %zu bytes\n", s);
  memset(p, 0, nmemb * s); //fills array with 0
  return p;
}

// implementation of free
void myfree(void *ptr) {
  assert(ptr);

  // set the block free
  header_t *chunk_ptr = get_header(ptr);
  assert(chunk_ptr);
  assert(chunk_ptr->free == 0); // makes sure that the chunk is being used
  assert(chunk_ptr->magic != 0x88888888); // make sure that the chunk can be used

  chunk_ptr->free = 1;
  chunk_ptr->magic = 0x11111111; //debug status: freed block;

  // try to coalesce the block of memory
  header_t *combined = coalesce(chunk_ptr);

  //debugging information
  debug_printf("Combined chunk information:\n");
  debug_printf(" - size: %zu bytes.\n", combined->size);
  debug_printf(" - debug: %08x.\n", combined->magic);
  debug_printf(" - address: %p.\n", combined);
  debug_printf("Start: %p. End: %p.\n", global_base, global_end);

  return;
}
