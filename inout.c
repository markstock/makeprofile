/*
 * VIC-MOC - inout.c - routines for input and output
 *
 * Copyright 2004-7,20 Mark J. Stock <mstock@umich.edu>
 */

#include <stdlib.h>
#include "inout.h"


/*
 * print a frame using 1 or 3 channels to png - 2D
 */
int write_png (const char *outfile, const int nx, const int ny,
   const int three_channel, const int high_depth,
   float **red, float redmin, float redrange,
   float **grn, float grnmin, float grnrange,
   float **blu, float blumin, float blurange) {

   int autorange = FALSE;
   int i,j,printval,bit_depth;
   float newminrange,newmaxrange;
   // gamma of 1.8 looks normal on most monitors...that display properly.
   //float gamma = 1.8;
   // must do 5/9 for stuff to look right on Macs....why? I dunno.
   float gamma = .55555;
   FILE *fp;
   png_uint_32 height,width;
   png_structp png_ptr;
   png_infop info_ptr;
   static png_byte **img;
   static int is_allocated = FALSE;
   static png_byte **imgrgb;
   static int rgb_is_allocated = FALSE;

   // set specific bit depth
   if (high_depth) bit_depth = 16;
   else bit_depth = 8;

   // allocate the space for the special array
   if (!is_allocated && !three_channel) {
      img = allocate_2d_array_pb(nx,ny,bit_depth);
      is_allocated = TRUE;
   }
   if (!rgb_is_allocated && three_channel) {
      imgrgb = allocate_2d_rgb_array_pb(nx,ny,bit_depth);
      rgb_is_allocated = TRUE;
   }

   // set the sizes in png-understandable format
   height=ny;
   width=nx;

   // auto-set the ranges
   if (autorange) {

      // first red
      newminrange = 9.9e+9;
      newmaxrange = -9.9e+9;
      for (i=0; i<nx; i++) {
         for (j=ny-1; j>=0; j--) {
            if (red[i][j]<newminrange) newminrange=red[i][j];
            if (red[i][j]>newmaxrange) newmaxrange=red[i][j];
         }
      }
      //printf("range %g %g\n",newminrange,newmaxrange);
      redmin = newminrange;
      redrange = newmaxrange-newminrange;

      // then the rest
      if (three_channel) {

         // then green
         newminrange = 9.9e+9;
         newmaxrange = -9.9e+9;
         for (i=0; i<nx; i++) {
            for (j=ny-1; j>=0; j--) {
               if (grn[i][j]<newminrange) newminrange=grn[i][j];
               if (grn[i][j]>newmaxrange) newmaxrange=grn[i][j];
            }
         }
         grnmin = newminrange;
         grnrange = newmaxrange-newminrange;

         // then blue
         newminrange = 9.9e+9;
         newmaxrange = -9.9e+9;
         for (i=0; i<nx; i++) {
            for (j=ny-1; j>=0; j--) {
               if (blu[i][j]<newminrange) newminrange=blu[i][j];
               if (blu[i][j]>newmaxrange) newmaxrange=blu[i][j];
            }
         }
         blumin = newminrange;
         blurange = newmaxrange-newminrange;
      }
   } else {
       // report the range
      newminrange = 9.9e+9;
      newmaxrange = -9.9e+9;
      for (i=0; i<nx; i++) {
         for (j=ny-1; j>=0; j--) {
            if (red[i][j]<newminrange) newminrange=red[i][j];
            if (red[i][j]>newmaxrange) newmaxrange=red[i][j];
         }
      }
      printf("  output range %g %g\n",newminrange,newmaxrange);
   }

   // write the file
   fp = fopen(outfile,"wb");
   if (fp==NULL) {
      fprintf(stderr,"Could not open output file %s\n",outfile);
      fflush(stderr);
      exit(0);
   }

   // now do the other two channels
   if (three_channel) {

     // no scaling, 16-bit per channel, RGB
     if (high_depth) {
       // am I looping these coordinates in the right memory order?
       for (j=ny-1; j>=0; j--) {
         for (i=0; i<nx; i++) {
           // red
           printval = (int)(0.5 + 65535*(red[i][j]-redmin)/redrange);
           if (printval<0) printval = 0;
           else if (printval>65535) printval = 65535;
           imgrgb[ny-1-j][6*i] = (png_byte)(printval/256);
           imgrgb[ny-1-j][6*i+1] = (png_byte)(printval%256);
           // green
           printval = (int)(0.5 + 65535*(grn[i][j]-grnmin)/grnrange);
           if (printval<0) printval = 0;
           else if (printval>65535) printval = 65535;
           imgrgb[ny-1-j][6*i+2] = (png_byte)(printval/256);
           imgrgb[ny-1-j][6*i+3] = (png_byte)(printval%256);
           // blue
           printval = (int)(0.5 + 65535*(blu[i][j]-blumin)/blurange);
           if (printval<0) printval = 0;
           else if (printval>65535) printval = 65535;
           imgrgb[ny-1-j][6*i+4] = (png_byte)(printval/256);
           imgrgb[ny-1-j][6*i+5] = (png_byte)(printval%256);
         }
       }

     // no scaling, 8-bit per channel, RGB
     } else {
       // am I looping these coordinates in the right memory order?
       for (j=ny-1; j>=0; j--) {
         for (i=0; i<nx; i++) {
           // red
           printval = (int)(0.5 + 256*(red[i][j]-redmin)/redrange);
           if (printval<0) printval = 0;
           else if (printval>255) printval = 255;
           imgrgb[ny-1-j][3*i] = (png_byte)printval;
           // green
           printval = (int)(0.5 + 256*(grn[i][j]-grnmin)/grnrange);
           if (printval<0) printval = 0;
           else if (printval>255) printval = 255;
           imgrgb[ny-1-j][3*i+1] = (png_byte)printval;
           // blue
           printval = (int)(0.5 + 256*(blu[i][j]-blumin)/blurange);
           if (printval<0) printval = 0;
           else if (printval>255) printval = 255;
           imgrgb[ny-1-j][3*i+2] = (png_byte)printval;
         }
       }
     }

   // monochrome image, read data from red array
   } else {

     // no scaling, 16-bit per channel
     if (high_depth) {
       // am I looping these coordinates in the right memory order?
       for (j=ny-1; j>=0; j--) {
         for (i=0; i<nx; i++) {
           printval = (int)(0.5 + 65534*(red[i][j]-redmin)/redrange);
           if (printval<0) printval = 0;
           else if (printval>65535) printval = 65535;
           img[ny-1-j][2*i] = (png_byte)(printval/256);
           img[ny-1-j][2*i+1] = (png_byte)(printval%256);
         }
       }

     // no scaling, 8-bit per channel
     } else {
       // am I looping these coordinates in the right memory order?
       for (j=ny-1; j>=0; j--) {
         for (i=0; i<nx; i++) {
           printval = (int)(0.5 + 254*(red[i][j]-redmin)/redrange);
           if (printval<0) printval = 0;
           else if (printval>255) printval = 255;
           img[ny-1-j][i] = (png_byte)printval;
         }
       }
     }

   }

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   if (png_ptr == NULL) {
      fclose(fp);
      fprintf(stderr,"Could not create png struct\n");
      fflush(stderr);
      exit(0);
      return (-1);
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
      return (-1);
   }

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr))) {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return (-1);
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
   // png_set_IHDR(png_ptr, info_ptr, height, width, bit_depth,
   if (three_channel) {
      png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
         PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
         PNG_FILTER_TYPE_BASE);
   } else {
      png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
         PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
         PNG_FILTER_TYPE_BASE);
   }

   /* Optional gamma chunk is strongly suggested if you have any guess
    * as to the correct gamma of the image. */
   //png_set_gAMA(png_ptr, info_ptr, 2.2);
   png_set_gAMA(png_ptr, info_ptr, gamma);

   /* Write the file header information.  REQUIRED */
   png_write_info(png_ptr, info_ptr);

   /* One of the following output methods is REQUIRED */
   // png_write_image(png_ptr, row_pointers);
   if (three_channel) {
      png_write_image(png_ptr, imgrgb);
   } else {
      png_write_image(png_ptr, img);
   }

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, &info_ptr);

   // close file
   fclose(fp);

   // free the data array
   //free_2d_array_pb(img);
   //free_2d_array_pb(imgrgb);

   return(0);
}


/*
 * read a PNG header and return width and height
 */
int read_png_res (const char *infile, int *hgt, int *wdt) {

   FILE *fp;
   unsigned char header[8];
   png_uint_32 height,width;
   int bit_depth,color_type,interlace_type;
   png_structp png_ptr;
   png_infop info_ptr;


   // check the file
   fp = fopen(infile,"rb");
   if (fp==NULL) {
      fprintf(stderr,"Could not open input file %s\n",infile);
      fflush(stderr);
      exit(0);
   }

   // check to see that it's a PNG
   fread (&header, 1, 8, fp);
   if (png_sig_cmp(header, 0, 8)) {
      fprintf(stderr,"File %s is not a PNG\n",infile);
      fflush(stderr);
      exit(0);
   }

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      exit(0);
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.  */
   if (setjmp(png_jmpbuf(png_ptr))) {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      exit(0);
   }

   /* One of the following I/O initialization methods is REQUIRED */
   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* If we have already read some of the signature */
   png_set_sig_bytes(png_ptr, 8);

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED */
   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
       &interlace_type, int_p_NULL, int_p_NULL);

   // set the sizes so that we can understand them
   (*hgt) = height;
   (*wdt) = width;

   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

   /* close the file */
   fclose(fp);

   return(0);
}


/*
 * read a PNG, write it to 1 or 3 channels
 */
int read_png (const char *infile, const int _nx, const int _ny,
   const int expect_three_channel,
   const int overlay, const float overlay_frac, const int darkenonly,
   float **red, float redmin, float redrange,
   float **grn, float grnmin, float grnrange,
   float **blu, float blumin, float blurange) {

   int autorange = FALSE;
   int nx = _nx;
   int ny = _ny;
   int high_depth;
   int three_channel;
   int i,j,printval; //,bit_depth,color_type,interlace_type;
   float newminrange,newmaxrange;
   float overlay_divisor;
   FILE *fp;
   unsigned char header[8];
   png_uint_32 height,width;
   int bit_depth,color_type,interlace_type;
   png_structp png_ptr;
   png_infop info_ptr;
   png_byte **img;


   // set up overlay divisor
   if (overlay == TRUE) {
      // set divisor to blend original and incoming
      overlay_divisor = 1.0 + overlay_frac;
   } else if (overlay == 2) {
      // set divisor to add original and incoming
      overlay_divisor = 1.0;
   }

   // check the file
   fp = fopen(infile,"rb");
   if (fp==NULL) {
      fprintf(stderr,"Could not open input file %s\n",infile);
      fflush(stderr);
      exit(0);
   }

   // check to see that it's a PNG
   fread (&header, 1, 8, fp);
   if (png_sig_cmp(header, 0, 8)) {
      fprintf(stderr,"File %s is not a PNG\n",infile);
      fflush(stderr);
      exit(0);
   }

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      exit(0);
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.  */
   if (setjmp(png_jmpbuf(png_ptr))) {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      exit(0);
   }

   /* One of the following I/O initialization methods is REQUIRED */
   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* If we have already read some of the signature */
   png_set_sig_bytes(png_ptr, 8);

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED */
   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
       &interlace_type, int_p_NULL, int_p_NULL);

   /* Set up the data transformations you want.  Note that these are all
    * optional.  Only call them if you want/need them.  Many of the
    * transformations only work on specific types of images, and many
    * are mutually exclusive.  */

   /* tell libpng to strip 16 bit/color files down to 8 bits/color */
   //png_set_strip_16(png_ptr);

   /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images).  */
   png_set_packing(png_ptr);

   /* Expand paletted colors into true RGB triplets */
   //if (color_type == PNG_COLOR_TYPE_PALETTE)
   //   png_set_palette_rgb(png_ptr);

   /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
   if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand_gray_1_2_4_to_8(png_ptr);

   /* Optional call to gamma correct and add the background to the palette
    * and update info structure.  REQUIRED if you are expecting libpng to
    * update the palette for you (ie you selected such a transform above).
    */
   //png_read_update_info(png_ptr, info_ptr);

   // check image type for applicability
   if (bit_depth != 8 && bit_depth != 16) {
     fprintf(stderr,"INCOMPLETE: read_png expect 8-bit or 16-bit images\n");
     fprintf(stderr,"   bit_depth: %d\n",bit_depth);
     fprintf(stderr,"   file: %s\n",infile);
     exit(0);
   }
   if (color_type != PNG_COLOR_TYPE_GRAY && color_type != PNG_COLOR_TYPE_RGB) {
     fprintf(stderr,"INCOMPLETE: read_png expect grayscale (%d) or RGB (%d) images\n",PNG_COLOR_TYPE_GRAY,PNG_COLOR_TYPE_RGB);
     fprintf(stderr,"   color_type: %d\n",color_type);
     fprintf(stderr,"   file: %s\n",infile);
     exit(0);
   }

   // set channels
   if (color_type == PNG_COLOR_TYPE_GRAY) three_channel = FALSE;
   else three_channel = TRUE;

   if (expect_three_channel && !three_channel) {
     fprintf(stderr,"ERROR: expecting 3-channel PNG, but input is 1-channel\n");
     fprintf(stderr,"  file (%s)",infile);
     fprintf(stderr,"  Convert file to color and try again.\n");
     exit(0);
   }

   if (!expect_three_channel && three_channel) {
     fprintf(stderr,"ERROR: not expecting 3-channel PNG, but input is 3-channel\n");
     fprintf(stderr,"  file (%s)",infile);
     fprintf(stderr,"  Convert file to grayscale and try again.\n");
     exit(0);
   }

   // set specific bit depth
   if (bit_depth == 16) high_depth = TRUE;
   else high_depth = FALSE;

   // make sure pixel sizes match exactly!
   if (ny != height || nx != width) {
     fprintf(stderr,"INCOMPLETE: read_png expects image resolution to match\n");
     fprintf(stderr,"  the simulation resolution.");
     fprintf(stderr,"  simulation %d x %d",nx,ny);
     fprintf(stderr,"  image %d x %d",width,height);
     fprintf(stderr,"  file (%s)",infile);
     exit(0);
   }

   // set the sizes so that we can understand them
   ny = height;
   nx = width;

   // allocate the space for the image array
   if (three_channel) {
      img = allocate_2d_rgb_array_pb(nx,ny,bit_depth);
   } else {
      img = allocate_2d_array_pb(nx,ny,bit_depth);
   }

   /* Now it's time to read the image.  One of these methods is REQUIRED */
   png_read_image(png_ptr, img);

   /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
   png_read_end(png_ptr, info_ptr);

   /* At this point you have read the entire image */

   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

   /* close the file */
   fclose(fp);


   // now convert the data to stuff we can use
   if (three_channel) {

     // no scaling, 16-bit per channel, RGB
     if (high_depth) {
       if (overlay && !darkenonly) {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = (red[i][j] + overlay_frac*(redmin+redrange*(img[ny-1-j][6*i]*256+img[ny-1-j][6*i+1])/65535.)) / overlay_divisor;
             grn[i][j] = (grn[i][j] + overlay_frac*(grnmin+grnrange*(img[ny-1-j][6*i+2]*256+img[ny-1-j][6*i+3])/65535.)) / overlay_divisor;
             blu[i][j] = (blu[i][j] + overlay_frac*(blumin+blurange*(img[ny-1-j][6*i+4]*256+img[ny-1-j][6*i+5])/65535.)) / overlay_divisor;
           }
         }
       } else if (overlay && darkenonly) {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] -= overlay_frac*(redmin+redrange*(1.-img[ny-1-j][3*i]/255.));
             red[i][j] -= overlay_frac*(redmin+redrange*(1.-(img[ny-1-j][6*i]*256+img[ny-1-j][6*i+1])/65535.));
             grn[i][j] -= overlay_frac*(grnmin+grnrange*(1.-(img[ny-1-j][6*i+2]*256+img[ny-1-j][6*i+3])/65535.));
             blu[i][j] -= overlay_frac*(blumin+blurange*(1.-(img[ny-1-j][6*i+4]*256+img[ny-1-j][6*i+5])/65535.));
           }
         }
       } else {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = redmin+redrange*(img[ny-1-j][6*i]*256+img[ny-1-j][6*i+1])/65535.;
             grn[i][j] = grnmin+grnrange*(img[ny-1-j][6*i+2]*256+img[ny-1-j][6*i+3])/65535.;
             blu[i][j] = blumin+blurange*(img[ny-1-j][6*i+4]*256+img[ny-1-j][6*i+5])/65535.;
           }
         }
       }

     // no scaling, 8-bit per channel, RGB
     } else {
       if (overlay && !darkenonly) {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = (red[i][j] + overlay_frac*(redmin+redrange*img[ny-1-j][3*i]/255.)) / overlay_divisor;
             grn[i][j] = (grn[i][j] + overlay_frac*(grnmin+grnrange*img[ny-1-j][3*i+1]/255.)) / overlay_divisor;
             blu[i][j] = (blu[i][j] + overlay_frac*(blumin+blurange*img[ny-1-j][3*i+2]/255.)) / overlay_divisor;
           }
         }
       } else if (overlay && darkenonly) {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] -= overlay_frac*(redmin+redrange*(1.-img[ny-1-j][3*i]/255.));
             grn[i][j] -= overlay_frac*(grnmin+grnrange*(1.-img[ny-1-j][3*i+1]/255.));
             blu[i][j] -= overlay_frac*(blumin+blurange*(1.-img[ny-1-j][3*i+2]/255.));
           }
         }
       } else {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = redmin+redrange*img[ny-1-j][3*i]/255.;
             grn[i][j] = grnmin+grnrange*img[ny-1-j][3*i+1]/255.;
             blu[i][j] = blumin+blurange*img[ny-1-j][3*i+2]/255.;
           }
         }
       }
     }

   // monochrome image, read data from red array
   } else {

     // no scaling, 16-bit per channel
     if (high_depth) {
       if (overlay) {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = (red[i][j] + overlay_frac*(redmin+redrange*(img[ny-1-j][2*i]*256+img[ny-1-j][2*i+1])/65534.)) / overlay_divisor;
           }
         }
       } else {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = redmin+redrange*(img[ny-1-j][2*i]*256+img[ny-1-j][2*i+1])/65534.;
           }
         }
       }

     // no scaling, 8-bit per channel
     } else {
       if (overlay) {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = (red[i][j] + overlay_frac*(redmin+redrange*img[ny-1-j][i]/254.)) / overlay_divisor;
           }
         }
       } else {
         for (j=ny-1; j>=0; j--) {
           for (i=0; i<nx; i++) {
             red[i][j] = redmin+redrange*img[ny-1-j][i]/254.;
           }
         }
       }
     }

   }

   // free the data array
   free_2d_array_pb(img);

   return(0);
}


/*
 * allocate memory for a two-dimensional array of png_byte
 */
png_byte** allocate_2d_array_pb(const int nx, const int ny, const int depth) {

   int i,bytesperpixel;
   png_byte **array;

   if (depth <= 8) bytesperpixel = 1;
   else bytesperpixel = 2;
   array = (png_byte **)malloc(ny * sizeof(png_byte *));
   array[0] = (png_byte *)malloc(bytesperpixel * nx * ny * sizeof(png_byte));

   for (i=1; i<ny; i++)
      array[i] = array[0] + i * bytesperpixel * nx;

   return(array);
}

png_byte** allocate_2d_rgb_array_pb(const int nx, const int ny, const int depth) {

   int i,bytesperpixel;
   png_byte **array;

   if (depth <= 8) bytesperpixel = 3;
   else bytesperpixel = 6;
   array = (png_byte **)malloc(ny * sizeof(png_byte *));
   array[0] = (png_byte *)malloc(bytesperpixel * nx * ny * sizeof(png_byte));

   for (i=1; i<ny; i++)
      array[i] = array[0] + i * bytesperpixel * nx;

   return(array);
}

int free_2d_array_pb(png_byte** array){
   free(array[0]);
   free(array);
   return(0);
}

