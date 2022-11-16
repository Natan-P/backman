#ifndef _ARGPARSE_H_
  typedef union {
    char s[4096];
    int i;
  } data;
  #define _ARGPARSE_H_
  data* getArgs(int argc, char **argv);
#endif

