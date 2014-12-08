/*------------------------------------------------------------------------------
 * File:	    	matrix.h
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Specification file for matrix library. Contains useful
 *                  matrix operations.
 *----------------------------------------------------------------------------*/

#ifndef MATRIX_H
#define MATRIX_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "debug.h"

typedef struct __Vector
{
    float* buf;
    uint16_t n;
    bool allocated;
} Vector;

typedef struct __Matrix
{
    Vector* rowVectors;
    uint16_t rows;
    uint16_t cols;
    bool allocated;
} Matrix;

// Allocates a n dimensional vector.
void createVector(Vector* x);

// Allocates an (A->rows x A->cols) size matrix.
void createMatrix(Matrix* A);

// Frees an n dimensional vector.
void freeVector(Vector* v);

// Frees an (A->rows x A->cols) size matrix.
void freeMatrix(Matrix* A);

// Recreates a matrix into a new matrix of size (rows x cols), shifting the
// contents in the direction specified.
void recreateMatrix(Matrix* A, uint8_t direction);

// Multiplies two matrices together.
void multiplyMatrix2Vector(Vector* y, Matrix* A, Vector* x);

#endif
