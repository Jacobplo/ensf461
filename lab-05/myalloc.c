#include <stddef.h>
#include "myalloc.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int statusno;

static void *_arena_start;
static size_t _arena_size;
static bool initialized = false;

#define DIV_CEIL(x,y) (((x) + (y) - 1) / (y))

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
  (void)size;

  return NULL;
}

void myfree(void *ptr) {
  (void)ptr;
}
