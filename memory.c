/*
 * memory.h - library for multidimensional arrays in C
 *
 * Copyright 2004-10 Mark J. Stock mstock@umich.edu
 */

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>

/*
 * allocate memory for a one-dimensional array of float
 */
float* allocate_1d_array_f(long int nx) {

   float *array = (float *)malloc(nx * sizeof(float));

   return(array);
}

int free_1d_array_f(float* array){
   free(array);
   return(0);
}

/*
 * allocate memory for a two-dimensional array of float
 */
float** allocate_2d_array_f(int nx,int ny) {

   int i;
   float **array = (float **)malloc(nx * sizeof(float *));

   array[0] = (float *)malloc(nx * ny * sizeof(float));
   for (i=1; i<nx; i++)
      array[i] = array[0] + i * ny;

   return(array);
}

int free_2d_array_f(float** array){
   free(array[0]);
   free(array);
   return(0);
}

/*
 * allocate memory for a three-dimensional array of floats
 */
float*** allocate_3d_array_f(int nx, int ny, int nz) {

   int i,j;
   float ***array = (float ***)malloc(nx * sizeof(float **));

   array[0] = (float **)malloc(nx * ny * sizeof(float *));
   array[0][0] = (float *)malloc(nx * ny * nz * sizeof(float));

   for (i=1; i<nx; i++)
      array[i] = array[0] + i * ny;

   for (i=0; i<nx; i++) {
      if (i!=0)
         array[i][0] = array[0][0] + i * ny * nz;
      for (j=1; j<ny; j++)
         array[i][j] = array[i][0] + j * nz;
   }

   return(array);
}

int free_3d_array_f(float*** array){
   free(array[0][0]);
   free(array[0]);
   free(array);
   return(0);
}

/*
 * allocate memory for a two-dimensional array of ints
 */
int** allocate_2d_array_i(int nx,int ny) {

   int i;
   int **array = (int **)malloc(nx * sizeof(int *));

   array[0] = (int *)malloc(nx * ny * sizeof(int));
   for (i=1; i<nx; i++)
      array[i] = array[0] + i * ny;

   return(array);
}

int free_2d_array_i(int** array){
   free(array[0]);
   free(array);
   return(0);
}

