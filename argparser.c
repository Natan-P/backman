#include <X11/Xlib.h>
#include <asm-generic/errno-base.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <bsd/string.h>
#include <unistd.h>
#include <errno.h>
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

#define SEPERATE_SIZING 1
#define __FREEALL_ARGPARSE free(ret);

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
static inline int fillBitRange(short s, short e) {
  return ((1<<(s))-1)^((1<<(e))-1);
}

short mask = 0b000;  // bitmask of expected args; bits meaning [other image options, image, standard arg]
short masked = 0b000;  // bitmask for the parsed args, aka what was actually passed

char funcerror = 0;
void anglefunc(char** argv, int* i, data* ret) {
  char* errptr;
  int angle = strtol(argv[++(*i)], &errptr, 10);
  if (angle > 360 || angle < 0 || *errptr != 0 || errno == ERANGE || ret[1].i >> MAGIC_BITS != 4) {
    puts("Bad angle input, currently only supported with tiled.\n");
    __FREEALL_ARGPARSE
    funcerror = 'L';
  }
  else ret[1].i |= angle << (MAGIC_BITS + IMAGE_BITS);
  masked |= 0b010;
}

void scalefunc(char** argv, int* i, data* ret) {
  if (SEPERATE_SIZING) {
    char* errptr;
    double dbl = (strtod(argv[++(*i)], &errptr));
    ret[2].i = *((unsigned long *) &dbl);
    if (*errptr != 0 || errno == ERANGE) {
      puts("Bad scaling input!\n");
      __FREEALL_ARGPARSE
      funcerror = 'L';
    }
    masked |= 0b010;
  } else {
    printf("fuck");
    __FREEALL_ARGPARSE
    funcerror = 'L';
  }
}

void doNothing(char** a, int* b, data* c) {};  // can't just feed in NULL so this is the closest to that

#define IMGOPTLISTSIZE 6
char* imgopts[] = {"", "stretch", "fill", "cover", "tile", "extend"};
char* shortimgs[] = {"", "S", "F", "C", "T", "E"};
#if (IMGOPTLISTSIZE > ((1 << IMAGE_BITS) - 1))
  #error "fuck"
#endif

char** imglists[] = {imgopts, shortimgs};

#define OTHEROPTLISTSIZE 6
char* others[] = {"", "angle", "scale", "randomtiles", "hdiff", "vdiff"};  // 1st gets autofilled if "image" is a directory
char* shortothers[] = {"", "a", "s", "", "h", "v"};
void (*otherfunc[])(char**, int*, data*) = {doNothing, anglefunc, scalefunc, doNothing, doNothing, doNothing};  // what functions get called 
#if (OTHEROPTLISTSIZE > MAGIC_BITS)
  #error "fuck but different"
#endif

char** magiclists[] = {others, shortothers};

char* getcharfromstring(char* dest, char* src, size_t num) {
  dest[0] = src[num];
  dest[1] = '\0';
  return dest;
}

data* getArgs(int argc, char **argv) {
  int whatlist = 2;
  if (argc < 2) { printf("%s", usage); exit(1); }
  mask = 0b001;  // a standard arg is expected at the beginning
//masked = 0b000;  // reminder that this boye has been initiated at 000
  data* ret = (data*) calloc(2 + SEPERATE_SIZING, sizeof(data));
  /*
  ret[0] is a string containing the path to the image to be 
  ret[1] is an integer bitmap, holding:
    //todo this line
  ret[2] a "double" holding the angle at which to tile (if tiling)
    optional: 
  */
  int x; // temp value for valueinarray bits
  for (int i = 1; i < argc; i++) {
    whatlist = 2;
    /* bitmask setters */
    if (argv[i][0] == '-') { // is short arg?
      whatlist = 1;
      if (argv[i][1] == '-') whatlist = 0;
    }

    if (whatlist == 0) {
      if ((x = valueinarray(argv[i]+2, imglists[whatlist], IMGOPTLISTSIZE)) > -1 && 
          !((ret[1].i & fillBitRange(MAGIC_BITS, MAGIC_BITS+IMAGE_BITS)) > 0))
   //       ^----------------------------------------------------------^
   //             check if this option has already been defined
            { masked |= 0b001; ret[1].i |= x << MAGIC_BITS; }
      else if ((x = valueinarray(argv[i]+2, magiclists[0], OTHEROPTLISTSIZE)) > -1 &&
          (ret[1].i & (1 << x)) < 1) {
        masked |= 0b010;
        ret[1].i |= 1 << x; 
        (*otherfunc[x])(argv, &i, ret);
        if (funcerror != 0) return NULL;
      }
    } else if (whatlist == 1) {
      char whichsmall[2] = "L";
      for (int whatsmall = 1; whatsmall < strlen(argv[i]); whatsmall++) {
        getcharfromstring(whichsmall, argv[i], whatsmall);

        if ((x = valueinarray(whichsmall, shortimgs, IMGOPTLISTSIZE)) > 1 &&
            !((ret[1].i & fillBitRange(MAGIC_BITS, MAGIC_BITS+IMAGE_BITS)) > 0)) {
          masked |= 0b001; ret[1].i |= x << MAGIC_BITS;  
        } else if ((x = valueinarray(whichsmall, shortothers, OTHEROPTLISTSIZE)) > -1 &&
            (ret[1].i & (1 << x)) < 1) {
          masked |= 0b010;
          ret[1].i |= (1 << x);
          (*otherfunc[x])(argv, &i, ret);
          if (funcerror != 0) return NULL;
        }
      }
    }

    if (access(argv[i], R_OK) > -1) {
      masked |= 0b100;
      strlcpy(ret[0].s, argv[i], 4096);
      struct stat statbuf;
      stat(argv[i], &statbuf);
      if (S_ISDIR(statbuf.st_mode) == 1) ret[1].i |= 1;
    }
    if ((masked & 0b111) > 4) { // are image bit *and* other bits set?
      printf("Please escape image names that start with a dash with `./'\n");
      masked = 0; 
    } // if it is both an image name and an argument, raise error 
    /* bitmask testers */
    if ((masked & mask) > 0) {  // is this a valid entry?
      if      ((masked & 0b001) > 0) { mask = 0b110; } // last was standard, next can be img or replacement
      else if ((masked & 0b010) > 0) { mask = 0b110; } // last was replacement, next can be replacement
      else if ((masked & 0b100) > 0) { mask = 0b010; } // last was image, can't set it to be 0b0 cause that breaks
      masked = 0b000;
    } else {  // if its not valid, then go away!
      printf("%s", usage);
      __FREEALL_ARGPARSE
      return(NULL);
    }
  }
  return ret;
}