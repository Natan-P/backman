#include <X11/Xlib.h>
#include <Imlib2.h>
#include "argparser.h"
#ifndef _IMGUTILS_H_
  #define _IMGUTILS_H_
  int loadimage(data* data, Imlib_Image img, Screen *screen);
  int setimage(const char* img, const int mask);
#endif