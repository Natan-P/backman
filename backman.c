#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "utils.h"
#include "argparser.h"

Imlib_Image img;
Pixmap pix;
Display *display;
int screen;
Imlib_Context context;
char* randdir;

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

// loads image into img global
int loadimage(char* state, const char* bgimg, Screen *screen) {
  Imlib_Image buffer = NULL;
  if (strcmp(bgimg, "--random") == 0) {
    DIR *dp;
    struct dirent *ep;     
    dp = opendir(randdir);
    int direntries = 0;
    if (dp != NULL) {
      while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0) {
          direntries++; }
      }
      char* ents[direntries];
      seekdir(dp, 0);
      direntries = 0;
      while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0) {
          ents[direntries] = ep->d_name;
          direntries++;
        }
      }
      char* select = ents[rand() % (direntries)]; // generate random number that gets modulo'd down to the number of files in the directory
      char final[256];
      strcpy(final, randdir);
      strcat(final, select);
      buffer = imlib_load_image(final);
      (void) closedir(dp);
    } else {
      perror("Couldn't open the directory!\n");
      exit(-1);
    }
  }
  if (buffer == NULL) {
    buffer = imlib_load_image(bgimg);
    //printf("%s\n", bgimg);
  }
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
  srand((unsigned) time(NULL));
  display = XOpenDisplay(NULL);
  if (! display) {
    printf("Can't open X display!");
    return 123;
  }

  char** args = getArgs(argc, argv);
  if (args == NULL) {
    return 0;
  }
  
  if (strcmp(args[1], "--random") == 0) strcpy(randdir, args[2]);
  printf("%s", randdir);

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
  
  free(args);
  XFree(display);
  return 0;
}