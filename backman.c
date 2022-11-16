#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <bits/time.h>
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "utils.h"
#include "imgutils.h"

Imlib_Image img;
Pixmap pix;
Display *display;
int screen;
Imlib_Context context;
char* randdir;

int setRootAtoms(Pixmap);
// copy-paste from hsetroot, i have a minimal clue of what this thing does,
// but i do know that without it, this mess of a program wouldn't work...
// left formatting from source code so you can really see who wrote this code.
int
setRootAtoms(Pixmap pixmap)
{
  Atom atom_root, atom_eroot, type;
  unsigned char *data_root, *data_eroot;
  int format;
  unsigned long length, after;

  atom_root = XInternAtom(display, "_XROOTMAP_ID", True);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", True);

  // doing this to clean up after old background
  if (atom_root != None && atom_eroot != None) {
    XGetWindowProperty(display, RootWindow(display, screen), atom_root, 0L, 1L, False, AnyPropertyType, &type, &format, &length, &after, &data_root);

    if (type == XA_PIXMAP) {
      XGetWindowProperty(display, RootWindow(display, screen), atom_eroot, 0L, 1L, False, AnyPropertyType, &type, &format, &length, &after, &data_eroot);

      if (data_root && data_eroot && type == XA_PIXMAP && *((Pixmap *) data_root) == *((Pixmap *) data_eroot))
        XKillClient(display, *((Pixmap *) data_root));
    }
  }

  atom_root = XInternAtom(display, "_XROOTPMAP_ID", False);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", False);

  if (atom_root == None || atom_eroot == None)
    return 0;

  // setting new background atoms
  XChangeProperty(display, RootWindow(display, screen), atom_root, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pixmap, 1);
  XChangeProperty(display, RootWindow(display, screen), atom_eroot, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pixmap, 1);

  return 1;
}



int main(int argc, char* *argv) {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  srand(time.tv_nsec);
  display = XOpenDisplay(NULL);
  if (! display) {
    printf("Can't open X display!\n");
    return 123;
  }

  data* args = getArgs(argc, argv);
  if (args == NULL) {
    return 0;
  }

  if ((args->i & 1) == 1) {
    randdir = args[0].s;
  }

  for (int screen = 0; screen < ScreenCount(display); screen++) {
    context = imlib_context_new();
    imlib_context_push(context);
    imlib_context_set_display(display);
    imlib_context_set_visual(DefaultVisual(display, DefaultScreen(display)));
    imlib_context_set_colormap(DefaultColormap(display, DefaultScreen(display)));
    Screen* scr = ScreenOfDisplay(display, screen);

    int width = scr->width, height = scr->height;
     
    pix = XCreatePixmap(display, RootWindow(display, screen), width, height, scr->root_depth);
    imlib_context_set_drawable(pix);
    
    img = imlib_create_image(scr->width, scr->height);

    if (loadimage(args, img, scr) == 0) {
      printf("Bad image.\n");
      free(args);
      return 1;
    }
    
    imlib_context_set_image(img);
    imlib_render_image_on_drawable(0, 0);
    imlib_free_image();

    if (setRootAtoms(pix) == 0)
      fprintf(stderr, "Couldn't create atoms...\n");

    XKillClient(display, AllTemporary);
    XSetCloseDownMode(display, RetainTemporary);

    XSetWindowBackgroundPixmap(display, RootWindow(display, screen), pix);
    XClearWindow(display, RootWindow(display, screen));
    
    // i have no clue why to put both flush and sync but that was in hsetroot and i trust that project
    XFlush(display);
    XSync(display, False);

    imlib_context_pop();
    imlib_context_free(context);
  }
  
  free(args);
  XFree(display);
  return 0;
}