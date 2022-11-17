#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// if returns -1 then string is not in array, otherwise returns index in list
int valueinarray(char *val, char* arr[], int size) {
  for (int i = 0; i < size; i++) {
    if (strcmp(val, arr[i]) == 0) {
      return i;
    }
  }
  return -1;
}

int approxsqrt(int val) {
  int ctr = 1, root = 1;
  for (;root<val;) {
    ctr++;
    root = ctr*ctr;
  }
  return ctr;
}