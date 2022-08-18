#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "argparser.h"

char* usage = 
"Backman - an app, that barely works, but just does. \n"
"Syntax: backman [--option1 arg1]... \n"
"Image options:\n"
"--stretch <img>: render the image stretched to fill the screen\n"
"--center <img>:  render the image centered on the screen, without scaling\n"
"--cover <img>:   render the image to fill the screen, maintaining its aspect ratio\n"
"--extend <img>:  render the image max aspect with borders\n";

int size = 4;
char* opts[] = {"--stretch", "--center", "--cover", "--extend"};

char** getArgs(int argc, char **argv) {
  int x;
  char** ret;
  ret = malloc(2 * sizeof(char[10]));
  int rargs = 0;
  for (int i = 1; i < argc; i++) {
    x = valueinarray(argv[i], opts, size);
    if (x >= 0) {
      if (access(argv[++i], R_OK) == -1) return NULL;
      rargs += 2;
      ret[rargs-2] = opts[x]; ret[rargs-1] = argv[i++];
    } else {
      printf("%s", usage);
      return NULL;
    }
  }
  return ret;
}