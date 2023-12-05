/*
 * inout.h - routines for input and output
 *
 * Copyright 2004-7 Mark J. Stock mstock@umich.edu
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define FALSE 0
#define TRUE 1
#define MAXCHARS 255

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#include "png.h"

int write_png (const char*, const int, const int, const int, const int, float**, float, float, float**, float, float, float**, float, float);
int read_png_res (const char *infile, int *hgt, int *wdt);
int read_png (const char*, const int, const int, const int, const int, const float, const int, float**, float, float, float**, float, float, float**, float, float);
png_byte** allocate_2d_array_pb (const int,const int,const int);
png_byte** allocate_2d_rgb_array_pb (const int,const int,const int);
int free_2d_array_pb (png_byte**);

#ifdef __cplusplus
}
#endif
