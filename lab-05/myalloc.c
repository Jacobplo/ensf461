#include <stddef.h>
#include "myalloc.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

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

  memset(_arena_start, 0, _arena_size);

  if (_arena_start == MAP_FAILED) {
    statusno = ERR_SYSCALL_FAILED;
    return statusno;
  }

  head = _arena_start;
  head->size = 0;
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
  node_t *header = (node_t *)(ptr - sizeof(node_t));

  header->is_free = 1;
}

static int getfree(node_t **node, size_t size) {
  *node = NULL;

  // Head not yet initialized
  if (head->size == 0) {
    if (_arena_size - sizeof(node_t) >= size) {
      *node = head;
      (*node)->size = size;
      (*node)->is_free = 0;
      (*node)->fwd = NULL;
      (*node)->bwd = NULL;
      return 0;
    }

    statusno = ERR_OUT_OF_MEMORY;
    return ERR_OUT_OF_MEMORY;
  } 

  node_t *cur = head; 
  size_t total_size = 0;
  while (1) {
    total_size += cur->size + sizeof(node_t);

    if (cur->is_free && (cur->size >= size)) {
      *node = cur;
      return 0;
    }

    if (cur->fwd == NULL) break;

    cur = cur->fwd;
  }

  // Handle case where chunks are not yet created.
  if (_arena_size >= size + total_size + sizeof(node_t)) {
    *node = _arena_start + total_size; 
    (*node)->bwd = cur; 
    (*node)->size = size;
    (*node)->is_free = 0;
    (*node)->fwd = NULL;
    return 0;
  }

  statusno = ERR_OUT_OF_MEMORY;
  return ERR_OUT_OF_MEMORY;
}
