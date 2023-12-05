/*
 * memory.h - library for multidimensional arrays in C
 *
 * Copyright 2004-10 Mark J. Stock mstock@umich.edu
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// allocation and utility routines
float* allocate_1d_array_f (long int);
int free_1d_array_f (float*);
float** allocate_2d_array_f (int,int);
int free_2d_array_f (float**);
float*** allocate_3d_array_f(int,int,int);
int free_3d_array_f(float***);
int** allocate_2d_array_i (int,int);
int free_2d_array_i (int**);

#ifdef __cplusplus
}
#endif
