#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "myalloc.h"

int statusno;

static void *_arena_start;
static size_t _arena_size;
static bool initialized = false;
static node_t *head;

#define DIV_CEIL(x,y) (((x) + (y) - 1) / (y))

static int getfree(node_t **node, size_t size);

int myinit(size_t size) { 
  if (size == 0 || size > MAX_ARENA_SIZE) {
    statusno = ERR_BAD_ARGUMENTS;
    return statusno;
  }

  size_t page_size = getpagesize();
  _arena_size = DIV_CEIL(size, page_size) * page_size;
  _arena_start = mmap(NULL, _arena_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  if (_arena_start == MAP_FAILED) {
    statusno = ERR_SYSCALL_FAILED;
    return statusno;
  }

  head = _arena_start;
  head->size = _arena_size - sizeof(node_t);
  head->is_free = 1;
  head->bwd = NULL;
  head->fwd = NULL;

  initialized = true;

  return _arena_size;
}

int mydestroy() {
  if (!initialized) {
    statusno = ERR_UNINITIALIZED;
    return statusno;
  }

  int ret = munmap(_arena_start, _arena_size);

  if (ret < 0) {
    statusno = ERR_SYSCALL_FAILED;
    return statusno;
  }

  initialized = false;
  _arena_start = NULL;
  _arena_size = 0;
  head = NULL;

  return 0;
}

void* myalloc(size_t size) {
  if (!initialized) {
    statusno = ERR_UNINITIALIZED;
    return NULL;
  }

  node_t *node = NULL;
  if (getfree(&node, size) < 0) {
    return NULL;
  };
  
  void* data = (void *)node + sizeof(node_t);

  return data;
}

void myfree(void *ptr) {
  if(!initialized) {
    statusno = ERR_UNINITIALIZED;
    return;
  }
  if(ptr == NULL){
    statusno = ERR_BAD_ARGUMENTS;
    return;
  }
  node_t *header = (node_t *)(ptr - sizeof(node_t));

  // Check if the previous chunk can be coalesced.
  if (header->bwd != NULL) {
    node_t *prev_header = header->bwd;

    if (prev_header->is_free) {
      prev_header->size += header->size + sizeof(node_t);
      prev_header->fwd = header->fwd;

      header = prev_header;
    }
  }

  // Check if the next chunk can be coalesced.
  if (header->fwd != NULL) {
    node_t *next_header = header->fwd;
    
    // Set the next header's bwd field for if the previous chunk was coalesced.
    next_header->bwd = header;

    if (next_header->is_free) {
      header->size += next_header->size + sizeof(node_t);
      header->fwd = next_header->fwd;
      if(header->fwd != NULL) header->fwd->bwd = header;
    }
  }

  header->is_free = 1;
}

static int getfree(node_t **node, size_t size) {
  *node = NULL;

  node_t *cur = head; 
  while (1) {
    // Check if this chunk has enough space.
    if (cur->is_free && (cur->size >= size)) {
      *node = cur;
      size_t size_extra = (*node)->size - size;

      // Split the chunk into two chunks if possible.
      if (size_extra >= sizeof(node_t)) {
        node_t *next = (void *)(*node) + sizeof(node_t) + size;

        next->size = size_extra - sizeof(node_t);
        next->fwd = (*node)->fwd;
        if(next->fwd !=NULL) next->fwd->bwd = next;
        next->bwd = *node;
        next->is_free = 1;

        (*node)->size = size;
        (*node)->fwd = next;
      }
      
      (*node)->is_free = 0;

      return 0;
    }

    if (cur->fwd == NULL) break;

    cur = cur->fwd;
  }

  statusno = ERR_OUT_OF_MEMORY;
  return ERR_OUT_OF_MEMORY;
}
