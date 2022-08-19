#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "argparser.h"

char* usage = 
"Backman - an app, that barely works, but just does. \n"
"Syntax: backman [--option1 arg1]... \n"
"Fill options:\n"
"--stretch <img>: render the image stretched to fill the screen\n"
"Image options:\n"
"You may replace <img> with either:\n"
"A path to an image file, or\n"
"the --random option, followed by a directory, containing ONLY image files, no other directories or files.\n"
"";

int size = 4;
char* opts[] = {"--stretch", "--center", "--cover", "--extend"};

int othersize = 1;
char* others[] = {"--random"};  // args that can come instead of an image after standard args

short mask = 0b000;  // bitmask for arg parsing; bits meaning [other image options, image, standard arg]
short masked = 0b000;  // bitmask for the parsed args

char** getArgs(int argc, char **argv) {
  if (argc < 2) { printf("%s", usage); exit(1); }
  mask = 0b001;  // a standard arg is expected at the beginning
//masked = 0b000;  // reminder that this man has been initiated at 000
  int mall = 4;
  char** ret = (char**) calloc(mall, sizeof(char[128]));
  if (ret == NULL) {
    printf("Uh oh, malloc failed!");
    exit(-1);
  }
  int rargs = 0;
  for (int i = 1; i < argc; i++) {
    /* bitmask setters */
    if (valueinarray(argv[i], opts, size) > -1)        masked |= 0b001;  // is standard arg;
    if (valueinarray(argv[i], others, othersize) > -1) masked |= 0b010;  // is replacement arg;
    if (access(argv[i], R_OK) > -1 && masked == 0)     masked |= 0b100;  // if none has been set, try if image;
    /* bitmask testers */
    if ((masked & mask) > 0) {  // is this a valid entry?
      ret[rargs] = argv[i];  // yeah, add it!
      rargs++;
      if ((masked & 0b001) > 0) mask = 0b110;  // last was standard, next can be img or replacement
      if ((masked & 0b010) > 0) mask = 0b100;  // last was replacement, next can be replacement
      if ((masked & 0b100) > 0) mask = 0b000;  // last was image, args are done, ignore the rest.
      masked = 0b000;
    } else if (mask == 0) {  // if mask has been set to 0 (after image?), do nothing with the rest of the args
    } else {  // if its not valid, then go away!
      printf("%s", usage);
      free(ret);
      return NULL;
    }
    if (rargs > mall) {
      mall *= 2;
      (void) realloc(&ret, mall * sizeof(char[20]));
    }
  }
  return ret;
}