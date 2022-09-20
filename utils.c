#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void **toFree;
void **tmpFree;
int mall = 2;
int malld = 0;
// if returns -1 then string is not in array, otherwise returns index in list
int valueinarray(void *val, void* arr[], int size) {
  for (int i = 0; i < size; i++) {
    if (memcmp(val, arr[i], sizeof(arr[0]))) {
      return i;
    }
  }
  return -1;
}

void initAlloc() {
  toFree = (void**) calloc(mall, sizeof(void*));
}
void checkAlloc() {
  malld++;
  if (mall < malld) toFree = (void**) realloc(toFree, mall * sizeof(void*));
}

// i will regret doing this shit later
void doFree(void* ptr) {
  int x = valueinarray(ptr, toFree, malld);
  if ((x > -1 ? toFree[x] = NULL : 0) == 0) { return; }
  free(ptr);
  tmpFree = (void**) malloc(sizeof(toFree));
  int j = 0;
  for (int i = 0; i < malld; i++) {
    if (toFree[i] != NULL) { tmpFree[j] = toFree[i]; j++; }
    else malld--;
  }
  toFree = (void**) realloc(toFree, j*sizeof(void*));
  toFree = tmpFree;
  free(tmpFree);
}
void* addCalloc(size_t nmemb, size_t size) {

}
void* addRealloc(void *ptr, size_t size) {

}
void freeAll() {}