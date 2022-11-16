#include <Imlib2.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imgutils.h"

extern char* randdir;
extern Imlib_Image img;
// loads image into img global
int loadimage(data* data, Imlib_Image imag, Screen *screen) {
  Imlib_Image buffer = NULL;
  if ((data[1].i & 1) == 1) {
    Imlib_Image temp = NULL;
    DIR *dp;
    if ((dp = opendir(randdir)) == NULL) return 0;
    struct dirent *ep;     
    int mall = 4;  // memory allocated for dynamic array
    char* *ents = (char**) calloc(mall, sizeof(char[2048]));
    int direntries = 0;
    char tempname[2048] = {0};
    while ((ep = readdir(dp)) != NULL) {
      if (direntries >= mall) { // lets make sure our array is large enough
        mall *= 2;
        ents = (char**) realloc(ents, mall * sizeof(char[2048]));
      }
      strcpy(tempname, "");
      strcpy(tempname, randdir);
      strcat(tempname, ep->d_name);
      temp = imlib_load_image(tempname);
      if (temp != NULL) {
        imlib_context_set_image(temp);
        imlib_free_image();
        ents[direntries] = ep->d_name;
        direntries++;
      }
    }
    char* select = ents[rand() % (direntries)]; // generate random number that gets modulo'd down to the number of files in the directory
    char final[2048] = {0};
    strcpy(final, randdir);
    strcat(final, select); // finally, set the end value as an absolute path to the image
    buffer = imlib_load_image(final);
    (void) closedir(dp);
  }
  if (buffer == NULL) {
    buffer = imlib_load_image(data[0].s);
    //printf("%s\n", bgimg);
  }
  if (! buffer) {
    printf("Failed to open image!\n");
    imlib_context_set_image(buffer);
    imlib_free_image();
    return 0;
  }
  imlib_context_set_image(buffer);
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