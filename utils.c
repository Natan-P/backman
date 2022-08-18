#include <string.h>
#include "utils.h"

// if returns -1 then string is not in array, otherwise returns index in list
int valueinarray(char *val, char* arr[], int size) {
  for (int i = 0; i < size; i++) {
    if (strcmp(arr[i], val) == 0) {
      return i;
    }
  }
  return -1;
}
