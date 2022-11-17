#pragma once

#define MAGIC_BITS 6
#define IMAGE_BITS 3

typedef union {
  char s[4096];
  unsigned long i;
} data;
data* getArgs(int argc, char **argv);