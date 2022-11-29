#include <Imlib2.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <bsd/string.h>
#include <limits.h>
#include "argparser.h"
#include "imgutils.h"
#include "utils.h"

#define optbits data[1].i
#define imgbits ((data[1].i >> MAGIC_BITS) & ((1 << IMAGE_BITS) - 1))

#define __FREEALL_ENTS \
  if ((data[1].i & 1) == 1 || c) { \
    for (int i = 0; i < direntries; i++) { \
      if (ents[i] != 0) { \
        imlib_context_set_image(ents[i]); \
        imlib_free_image(); \
      } \
      else break; \
    } \
    free(ents); \
  }

extern char* randdir;
extern Imlib_Image img;
// loads image into img global
int loadimage(data* data, Imlib_Image imag, Screen *screen) {
  Imlib_Image buffer = NULL;
  Imlib_Image* ents;
  int c = ((imgbits == 4) && ((data[1].i & (1 << 3)) > 1));
  int direntries = 0;
  int minImgW = INT_MAX, minImgH = INT_MAX;
  if ((data[1].i & 1) == 1 || c) {
    Imlib_Image temp = NULL;
    DIR *dp;
    if ((dp = opendir(randdir)) == NULL) return 0;
    struct dirent *ep;     
    int mall = 4;  // memory allocated for dynamic array
    ents = (Imlib_Image*) calloc(mall, sizeof(Imlib_Image));
    char tempname[4096] = {0};
    while ((ep = readdir(dp)) != NULL) {
      if (direntries >= mall) { // lets make sure our array is large enough
        mall *= 2;
        ents = (Imlib_Image*) realloc(ents, mall * sizeof(Imlib_Image));
      }
      strcpy(tempname, "");
      strlcpy(tempname, randdir, 4096);
      strlcat(tempname, ep->d_name, 4096);
      temp = imlib_load_image(tempname);
      if (temp != NULL) {
        imlib_context_set_image(temp);
        if (imlib_image_get_width() < minImgW) minImgW = imlib_image_get_width();
        if (imlib_image_get_height() < minImgH) minImgH = imlib_image_get_height();
        ents[direntries] = imlib_load_image(tempname);
        imlib_free_image();
        direntries++;
      }
    }
    if (direntries == 0) { // yeah i could do !direntries but i want readability, you know?
      free(ents);
      return 0;
    }
    buffer = ents[rand() % (direntries)]; // generate random number that gets modulo'd down to the number of files in the directory
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
    __FREEALL_ENTS;
    return 0;
  }
  imlib_context_set_image(buffer);
  int imgW, imgH;
  if (c) imgW = minImgW, imgH = minImgH;
  else   imgW = imlib_image_get_width(), imgH = imlib_image_get_height();
  int scrW = screen->width, scrH = screen->height;

  if ((data[1].i & (1 << 2)) > 1 && (*((double*)(&data[2].i)) != 1)) {
    double scale = *((double*)(&data[2].i));
    if (c) {
      for (int i = 0; i < direntries; i++) {
        imlib_context_set_image(ents[i]);
        int currW = imlib_image_get_width(), currH = imlib_image_get_height();
        Imlib_Image temp = imlib_create_cropped_scaled_image(0, 0, currW, currH, currW*scale, currH*scale);
        imlib_context_set_image(temp);
        ents[i] = imlib_clone_image();
        imlib_free_image();
      }
    }
    else {
      Imlib_Image temp = imlib_create_cropped_scaled_image(0, 0, imgW, imgH, imgW*scale, imgH*scale);
      imlib_context_set_image(temp);
      buffer = imlib_clone_image();
      imlib_free_image();
    }
    imgW*=scale;
    imgH*=scale;
  }

  img = imlib_create_image(scrW, scrH);
  imlib_context_set_image(img);

  int angle = 0;
  const int imagenum = imgbits;
  
  if (imagenum == 4 && (data[1].i & (1 << 1)) > 1 && data[1].i >> (MAGIC_BITS + IMAGE_BITS) != 0) {
    angle = data[1].i >> (MAGIC_BITS + IMAGE_BITS);
    scrW = (int) sqrt((double) scrW * scrW + (double) scrH * scrH);
    scrH = scrW; // make sure that the whole rotated area is filled
  }
  int wasthereanerror = 0;

  switch (imagenum) {
    case 1:
      imlib_blend_image_onto_image(buffer, 0, 0, 0,
          imgW, imgH, 0, 0, scrW, scrH);
      break;
    case 2: case 3:;
      double aspect = ((double) scrW) / imgW;
      if (((int) (imgH * aspect) > scrH) != (imgbits == 3)) 
        aspect = (double) scrH / (double) imgH;
      int left = (scrW - ((int) (aspect * imgW))) / 2, top = (scrH - ((int) (aspect * imgH))) / 2;
      imlib_blend_image_onto_image(buffer, 0, 0, 0, imgW, imgH, left, top, imgW*aspect, imgH*aspect);

      break;
    case 4:
      left = scrW / 2 - imgW/2;
      top = scrH / 2 - imgH/2;
      for (; left > 0; left -= imgW);
      for (; top > 0; top -= imgH);
      Imlib_Image temp;
      if (data[1].i >> (MAGIC_BITS + IMAGE_BITS) != 0) {
        temp = imlib_create_image(scrW, scrH);
        imlib_context_set_image(temp);
      }
      if (c) {
        for (int y = left; y < scrH; y += imgH) {
          for (int x = top; x < scrW;) {
            buffer = ents[rand() % direntries];
            Imlib_Image currentimage = imlib_context_get_image();
            imlib_context_set_image(buffer);
            const int bufW = imlib_image_get_width(), bufH = imlib_image_get_height();
            imlib_context_set_image(currentimage);
            imlib_blend_image_onto_image(buffer, 0, 0, 0, bufW, bufH, x, y, bufW, bufH);
            x += imgW;
            if ((data[1].i & (1 << 4)) > 1) {
              Imlib_Image currentimage = imlib_context_get_image();
              imlib_context_set_image(buffer);
              x += (imlib_image_get_width() - imgW);
              imlib_context_set_image(currentimage);
            }
          }
        }
      } else {
      for (int x = left; x < scrW; x += imgW)
        for (int y = top; y < scrW; y += imgH)
          imlib_blend_image_onto_image(buffer, 0, 0, 0, imgW, imgH, x, y, imgW, imgH);
      }
      if (data[1].i >> (MAGIC_BITS + IMAGE_BITS) != 0) {
        imlib_context_set_image(temp);
        Imlib_Image rotated_temp = imlib_create_image(scrW, scrH);
        #define scl 0.707

        int topleft_x  = (scrW>>1) * ( sin((3.1415/180) * (angle-45))/scl) + (scrW>>1);
        int topleft_y  = (scrH>>1) * (-cos((3.1415/180) * (angle-45))/scl) + (scrH>>1);
        int topright_x = (scrW>>1) * ( sin((3.1415/180) * (angle+45))/scl) + (scrW>>1) - topleft_x;
        int topright_y = (scrH>>1) * (-cos((3.1415/180) * (angle+45))/scl) + (scrH>>1) - topleft_y;
        //                               ^----------------------^      ^
        //                     convert left and right to radians, see desmos for +-45 meaning
        //                                                            |
        //                                                  normalize distance to 1
        imlib_context_set_image(rotated_temp);
        Imlib_Image temp2 = imlib_create_image(scrW, scrH);
        imlib_context_set_image(temp2);
        imlib_blend_image_onto_image(temp, 0, 0, 0, scrW, scrH, 0, 0, scrW, scrH);
        imlib_context_set_image(rotated_temp);
        imlib_blend_image_onto_image_at_angle(temp2, 0, 0, 0, scrW, scrH,
                topleft_x, topleft_y, topright_x, topright_y);
        
        img = imlib_create_cropped_image((scrW - screen->width)/2, (scrH - screen->height)/2,
                screen->width, screen->height);
        imlib_context_set_image(temp);
        imlib_free_image();
        imlib_context_set_image(rotated_temp);
        imlib_free_image();
        imlib_context_set_image(temp2);
        imlib_free_image();
        imlib_context_set_image(img);
      }
      break;

    default:
      wasthereanerror = 1;
      break;
  }

  __FREEALL_ENTS

  imlib_context_set_image(buffer);
  imlib_free_image();
  imlib_context_set_image(img);

  if (wasthereanerror == 1) {
    return 0;
  }
  
  return 1;
}
