#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "argparser.h"

char* usage = 
"Backman - an app, that barely works, but just does. \n"
"Syntax: backman <--option arg>... \n"
"Fill options:\n"
"--stretch <img>: render the image stretched to fill the screen\n"
"Image options:\n"
"You may replace <img> with either:\n"
"A path to an image file, or\n"
"the --random option, followed by a directory, containing ONLY image files, no other directories or files.\n"
"";

void roundToPow2(int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
}

// return an int with bits filled from s (incl.) to e (excl.)
// please make sure that s <= e, anything else is untested behavior and may work???
int fillBitRange(short s, short e) {
  return ((1<<(s))-1)^((1<<(e))-1);
}

int size = 4;
char* opts[] = {"--stretch", "--center", "--cover", "--extend"};

int othersize = 1;
char* others[] = {"--random"};  // args that can come instead of an image after standard args

short mask = 0b000;  // bitmask of expected args; bits meaning [other image options, image, standard arg]
short masked = 0b000;  // bitmask for the parsed args, aka what was actually passed

data* getArgs(int argc, char **argv) {
  if (argc < 2) { printf("%s", usage); exit(1); }
  mask = 0b001;  // a standard arg is expected at the beginning
//masked = 0b000;  // reminder that this boye has been initiated at 000
  data* ret = (data*) calloc(2, sizeof(data));
  /*
  ret[0] is a string containing the path to the image to be 
  ret[1] is an integer bitmap, holding:
  */
  int x; // temp value for valueinarray bits
  for (int i = 1; i < argc; i++) {
    /* bitmask setters */
    if ((x = valueinarray(argv[i], opts, size)) > -1 && 
        !((ret[1].i & fillBitRange(8, 32)) > 0)){ masked |= 0b001; ret[1].i |= x << 8; }
    //    ^----------------------------------^
    // check if this option has already been defined
    if (valueinarray(argv[i], others, othersize) > -1 &&
        (ret[1].i & 1) > 0)                     { masked |= 0b010; ret[1].i |= 1; }
    if (access(argv[i], R_OK) > -1)             { masked |= 0b100; }
    if ((masked & 0b100) > 0 && (masked & 0b011) > 0) { 
      printf("Please don't name your images the same as potential arguments.\n");
      masked = 0; 
    } // if it is both an image name and an argument, raise error 
    /* bitmask testers */
    if ((masked & mask) > 0) {  // is this a valid entry?
      if ((masked & 0b001) > 0) { mask = 0b110; } // last was standard, next can be img or replacement
      if ((masked & 0b010) > 0) { mask = 0b100; } // last was replacement, next can be replacement
      if ((masked & 0b100) > 0) { mask = 0b001; } // last was image, can't set it to be 0b0 cause that breaks
      masked = 0b000;
    } else {  // if its not valid, then go away!
      printf("%s", usage);
      return NULL;
    }
  }
  return ret;
}