#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <Imlib2.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

Imlib_Image img;
Pixmap pix;
Display *display;
Imlib_Context context;

typedef enum img_states { stretch, full, center } img_states;
typedef struct screen {
  Screen *screen;
  int width;
  int height;
} Scr;

int loadimage(img_states state, const char* bgimg, Imlib_Image rootimg, Screen *screen, Pixmap *pxm) {
  Imlib_Image buffer;
  buffer = imlib_load_image(bgimg);
  if (! buffer) {
    printf("Failed to open image!");
    return 0;
  }

  imlib_context_set_image(buffer);
  int imgW = imlib_image_get_width(), imgH = imlib_image_get_height();
  int scrW = WidthOfScreen(screen), scrH = HeightOfScreen(screen);
  
  imlib_context_set_image(rootimg);
  imlib_blend_image_onto_image(buffer, true, 0, 0,
    imgW, imgH, 0, 0, scrW, scrH);

  imlib_render_image_on_drawable(0, 0);
  imlib_context_set_image(buffer);
  imlib_free_image();
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

    int width = WidthOfScreen(ScreenOfDisplay(display, screen)), height = HeightOfScreen(ScreenOfDisplay(display, screen));

    pix = XCreatePixmap(display, RootWindow(display, screen), width, height, DefaultDepthOfScreen(ScreenOfDisplay(display, screen)));
    imlib_context_set_drawable(pix);

    loadimage(stretch, "/home/natan/dotstuff/MountN.png", img, ScreenOfDisplay(display, screen), &pix);
    
    XSetWindowBackgroundPixmap(display, RootWindow(display, screen), pix);
    XClearWindow(display, RootWindow(display, screen));

    imlib_context_set_image(img);
    imlib_free_image();
    imlib_context_pop();
    imlib_context_free(context);

    XSync(display, false);
  }
  XFree(display);
}