#include <stddef.h>
#ifndef _UTILS_H_
  #define _UTILS_H_
  int valueinarray(void *val, void* arr[], int size);

  // heap shit
  void initAlloc();
  void checkAlloc();

  void addFree(void *ptr);
  void* addCalloc(size_t nmemb, size_t size);
  void* addRealloc(void *ptr, size_t size);
  void freeAll();
#endif
