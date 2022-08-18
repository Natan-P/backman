#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "argparser.h"

Imlib_Image img;
Pixmap pix;
Display *display;
int screen;
Imlib_Context context;

typedef struct screen {
  Screen *screen;
  int width;
  int height;
} Scr;

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

// loads image into 
int loadimage(char* state, const char* bgimg, Screen *screen) {
  Imlib_Image buffer;
  buffer = imlib_load_image(bgimg);
  imlib_context_set_image(buffer);
  if (! buffer) {
    printf("Failed to open image!\n");
    imlib_context_set_image(buffer);
    imlib_free_image();
    return 0;
  }
  int imgW = imlib_image_get_width(), imgH = imlib_image_get_height();
  int scrW = screen->width, scrH = screen->height;
  
  img = imlib_create_image(scrW, scrH);

  imlib_context_set_image(img);
  imlib_blend_image_onto_image(buffer, 0, 0, 0,
    imgW, imgH, 0, 0, scrW, scrH);

  imlib_context_set_image(buffer);
  imlib_free_image();
  imlib_context_set_image(img);
  return 1;
}

int main(int argc, char **argv) {
  display = XOpenDisplay(NULL);
  if (! display) {
    printf("Can't open X display!");
    return 123;
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
    
    // big argv parser boye
    char** args = getArgs(argc, argv);
    if (args == NULL) { 
      printf("Args are null.\n");
      return 0;
    }
    
    if (loadimage("stretch", args[1], scr) == 0) {
      printf("Bad image.\n");
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
  
  XFree(display);
  return 0;
}