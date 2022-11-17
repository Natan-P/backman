#include <X11/Xlib.h>
#include <asm-generic/errno-base.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define IMGOPTLISTSIZE 6
char* imgopts[] = {"", "stretch", "fill", "cover", "tile", "extend"};
char* shortimgs[] = {"", "S", "F", "C", "T", "E"};
#if (IMGOPTLISTSIZE > ((1 << IMAGE_BITS) - 1))
  #error "fuck"
#endif

char** imglists[] = {imgopts, shortimgs};

#define OTHEROPTLISTSIZE 3
char* others[] = {"", "angle", "scale"};  // 1st gets autofilled if "image" is a directory
char* shortothers[] = {"", "a", "s"};
#if (OTHEROPTLISTSIZE > ((1 << MAGIC_BITS) - 1))
  #error "fuck but different"
#endif

char** magiclists[] = {others, shortothers};

short mask = 0b000;  // bitmask of expected args; bits meaning [other image options, image, standard arg]
short masked = 0b000;  // bitmask for the parsed args, aka what was actually passed

char* getcharfromstring(char* dest, char* src, size_t num) {
  dest[0] = src[num];
  dest[1] = '\0';
  return dest;
}

data* getArgs(int argc, char **argv) {
  int whatlist = 2;
  int whatsmall = 0;
  char whichsmall[2] = "L";
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
    whatsmall = 0;
    whatlist = 2;
    /* bitmask setters */
    if (argv[i][0] == '-') { // is short arg?
      whatlist = 1;
      if (argv[i][1] == '-') whatlist = 0;
    }
    if (whatlist < 2) {
      //                            pretty much just slice a string
      //                                           vv
      if ((x = valueinarray(whatlist == 0 ? argv[i]+2 : getcharfromstring(whichsmall, argv[i], 1+(whatsmall++)), imglists[whatlist], IMGOPTLISTSIZE)) > -1 && 
          !((ret[1].i & fillBitRange(MAGIC_BITS, MAGIC_BITS+IMAGE_BITS)) > 0))
   //       ^----------------------------------------------------------^
   //             check if this option has already been defined
            { masked |= 0b001; ret[1].i |= x << MAGIC_BITS; } else if (whatlist == 1) whatsmall--; // if short arg failed, revert it
      else if ((x = valueinarray(whatlist == 0 ? argv[i]+2 : getcharfromstring(whichsmall, argv[i], 1+(whatsmall++)), magiclists[whatlist], OTHEROPTLISTSIZE)) > -1 &&
        (ret[1].i & (1 << x)) < 1) { masked |= 0b010; ret[1].i |= 1 << x; 
        if (strcmp(argv[i], "--scale") == 0 && i < argc) {
          if (SEPERATE_SIZING) {
            char* errptr;
            double dbl = (strtod(argv[++i], &errptr));
            ret[2].i = *((unsigned long *) &dbl);
            if (*errptr != 0 || errno == ERANGE) {
              puts("Bad scaling input!\n");
              __FREEALL_ARGPARSE
              return(NULL);
            }
            masked |= 0b010;
          } else {
            printf("fuck");
            __FREEALL_ARGPARSE
            return(NULL);
          }
        } else if (strcmp(argv[i], "--angle") == 0 && i < argc) {  // i won't even fucking bother with short args
          char* errptr;
          int angle = strtol(argv[++i], &errptr, 10);
          if (angle > 360 || angle < 0 || *errptr != 0 || errno == ERANGE || ret[1].i >> MAGIC_BITS != 4) {
            puts("Bad angle input, currently only supported with tiled.\n");
            __FREEALL_ARGPARSE
            return(NULL);
          }
          ret[1].i |= angle << (MAGIC_BITS + IMAGE_BITS);
          masked |= 0b010;
        }
      } else if (whatlist == 1) whatsmall--;
    }
    if (access(argv[i], R_OK) > -1) {
      masked |= 0b100;
      strcpy(ret[0].s, argv[i]);
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