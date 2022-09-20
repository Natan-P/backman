typedef union {
  char s[4096];
  int i;
}
data;
#ifndef _ARGPARSE_H_
  #define _ARGPARSE_H_
  data* getArgs(int argc, char **argv);
#endif

