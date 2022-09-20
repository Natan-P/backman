#include <X11/Xlib.h>
#include <Imlib2.h>
#ifndef _IMGUTILS_H_
  #define _IMGUTILS_H_
  int loadimage(const char* bgimg, const int mask, Imlib_Image img, Screen *screen);
  int setimage(const char* img, const int mask);
#endif