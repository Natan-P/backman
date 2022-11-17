#include <Imlib2.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argparser.h"
#include "imgutils.h"
#include "utils.h"

#define optbits data[1].i
#define imgbits ((data[1].i >> MAGIC_BITS) & ((1 << IMAGE_BITS) - 1))

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

  if ((data[1].i & (1 << 2)) > 1 && (*((double*)(&data[2].i)) == 1)) {
    double scale = *((double*)(&data[2].i));
    Imlib_Image temp = imlib_create_cropped_scaled_image(0, 0, imgW, imgH, imgW*scale, imgH*scale);
    imlib_context_set_image(temp);
    buffer = imlib_clone_image();
    imgW*=scale;
    imgH*=scale;
  }

  img = imlib_create_image(scrW, scrH);
  imlib_context_set_image(img);

  int angle = 0;
  int imagenum = imgbits;
  
  if (imagenum == 4 && (data[1].i & (1 << 1)) > 1 && data[1].i >> (MAGIC_BITS + IMAGE_BITS) != 0) {
    angle = data[1].i >> (MAGIC_BITS + IMAGE_BITS);
    scrW = (int) sqrt((double) scrW * scrW + (double) scrH * scrH);
    scrH = scrW; // make sure that the whole rotated area is filled
  }

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
      for (int x = left; x < scrW; x += imgW)
        for (int y = top; y < scrW; y += imgH)
          imlib_blend_image_onto_image(buffer, 0, 0, 0, imgW, imgH, x, y, imgW, imgH);
      if (data[1].i >> (MAGIC_BITS + IMAGE_BITS) != 0) {
        imlib_context_set_image(temp);
        Imlib_Image rotated_temp = imlib_create_image(scrW, scrH);
        double scl = sqrt(0.5);

        int topleft_x  = (scrW>>1) * ( sin((3.1415/180) * (angle-45))/scl) + (scrW>>1);
        int topleft_y  = (scrH>>1) * (-cos((3.1415/180) * (angle-45))/scl) + (scrH>>1);
        int topright_x = (scrW>>1) * ( sin((3.1415/180) * (angle+45))/scl) + (scrW>>1) - topleft_x;
        int topright_y = (scrH>>1) * (-cos((3.1415/180) * (angle+45))/scl) + (scrH>>1) - topleft_y;
//        topleft_x %= scrW; topright_x %= scrW; topleft_y %= scrW; topright_y %= scrW;
        //                               ^----------------------^      ^
        //                     convert left and right to radians, see desmos for +-45 meaning
        //                                                            |
        //                                                  normalize distance to 1
        imlib_context_set_image(rotated_temp);
        Imlib_Image topo = imlib_create_image(scrW, scrH);
        imlib_context_set_image(topo);
        imlib_blend_image_onto_image(temp, 0, 0, 0, scrW, scrH, 0, 0, scrW, scrH);
        imlib_context_set_image(rotated_temp);
        imlib_blend_image_onto_image_at_angle(topo, 0, 0, 0, scrW, scrH,
                topleft_x, topleft_y, topright_x, topright_y);
        //imlib_blend_image_onto_image_at_angle(topo, 0, 0, 0, 1698, 1698, 160, -134, 1832, 160);
        
        img = imlib_create_cropped_image((scrW - screen->width)/2, (scrH - screen->height)/2,
                screen->width, screen->height);
        imlib_context_set_image(temp);
        imlib_free_image();
        imlib_context_set_image(rotated_temp);
        imlib_free_image();
        imlib_context_set_image(img);
        //imlib_save_image("rotated_cropped_temp.png");
      }
      break;

    default:
      return 0;
  }


  imlib_context_set_image(buffer);
  imlib_free_image();
  imlib_context_set_image(img);
  
  return 1;
}
