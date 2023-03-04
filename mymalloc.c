#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include <malloc.h> 
#include <stdio.h> 

// Include any other headers we need here
#include <sys/mman.h>
#include <unistd.h>
// NOTE: You should NOT include <stdlib.h> in your final implementation

#include <debug.h> // definition of debug_printf

void *mymalloc(size_t s) {
  void *p = sbrk(0);
  void *req = sbrk(s);

  if (p == MAP_FAILED) {
    return NULL;
  }
  debug_printf("malloc %zu bytes\n", s);
  assert(p == req);
  return p;
}

void *mycalloc(size_t nmemb, size_t s) {

  void *p = (void *) calloc(nmemb, s); // In your solution no calls to calloc should be
                                       // made! Determine how you will request memory :)

  if (!p) {
    // We are out of memory
    // if we get NULL back from malloc
  }
  debug_printf("calloc %zu bytes\n", s);

  return p;
}

void myfree(void *ptr) {
  debug_printf("Freed some memory\n");

  // Replace the free below with your own free implementation
  free(ptr);
}
