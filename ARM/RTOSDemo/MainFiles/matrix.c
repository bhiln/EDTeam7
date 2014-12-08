/*------------------------------------------------------------------------------
 * File:	    	matrix.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Implementation file for matrix library.
 *----------------------------------------------------------------------------*/

#include "matrix.h"

void createMatrix(Matrix* A)
{
    if((A->rows) == 0 || (A->cols) == 0)
        return;

    if(A->allocated)
        return;

    uint16_t i;
    uint16_t r = A->rows;
    uint16_t c = A->cols;
    A->rowVectors = malloc(r * sizeof(Vector));
    for(i = 0; i < r; i++)
    {
        A->rowVectors[i].n = c;
        createVector(&(A->rowVectors[i]));
    }
    A->allocated = true;
}

void createVector(Vector* x)
{
    if(x->allocated)
        return;

    x->buf = malloc(x->n * sizeof(float));
    if(x->buf)
        x->allocated = true;

    uint16_t j;
    for(j = 0; j < x->n; j++)
        x->buf[j] = 0.0;
}

void freeMatrix(Matrix* A)
{
    if(!A->allocated)
        return;

    uint16_t i;
    for(i = 0; i < A->rows; i++)
        freeVector(&(A->rowVectors[i]));
    free(A->rowVectors);
    A->rowVectors = 0;
    A->rows = 0;
    A->cols = 0;
    A->allocated = false;
}

void freeVector(Vector* x)
{
    if(!x->allocated)
        return;

    free(x->buf);
    x->buf = 0;
    x->allocated = false;
}

void recreateMatrix(Matrix* A, uint8_t direction)
{
    // Allocate a matrix with the new size. 
    Matrix newMatrix; 

    uint16_t rows = A->rows;
    uint16_t cols = A->cols;

    // Put the values of the old matrix into the new one with shifted positions dependent on the direction parameter.
    switch(direction)
    {
    case SIDE_RIGHT:
    {
        // Allocated to the right, no need to shift.
        newMatrix.rows = rows;
        newMatrix.cols = 2*cols;
        createMatrix(&newMatrix);

        uint16_t i, j;
        for(i = 0; i < rows; i++)
        {
            for(j = 0; j < cols; j++)
               newMatrix.rowVectors[i].buf[j] = A->rowVectors[i].buf[j]; 
        }
        break;
    }
    
    case SIDE_FRONT:
    {
        // Allocate up so shift down.
        newMatrix.rows = 2*rows;
        newMatrix.cols = cols;
        createMatrix(&newMatrix);

        uint16_t i, j;
        for(i = 0; i < rows; i++)
        {
            for(j = 0; j < cols; j++)
                newMatrix.rowVectors[i + rows].buf[j] = A->rowVectors[i].buf[j];
        }
        break;
    }
    case SIDE_LEFT:
    {
        // Allocate left so shift right.
        newMatrix.rows = rows;
        newMatrix.cols = 2*cols;
        createMatrix(&newMatrix);

        uint16_t i, j;
        for(i = 0; i < rows; i++)
        {
            for(j = 0; j < cols; j++)
                newMatrix.rowVectors[i].buf[j + cols] = A->rowVectors[i].buf[j];
        }
        break;

    }
    case SIDE_BACK:
    {
        // Allocate down, so no need to shift anything.
        newMatrix.rows = 2*rows;
        newMatrix.cols = cols;
        createMatrix(&newMatrix);

        uint16_t i, j;
        for(i = 0; i < rows; i++)
        {
            for(j = 0; j < cols; j++)
               newMatrix.rowVectors[i].buf[j] = A->rowVectors[i].buf[j]; 
        }
        break;
    }
    default:
    {
        break;
    }
    }

    // Finish by freeing the old matrix.
    freeMatrix(A);
    A->rowVectors  = newMatrix.rowVectors;
    A->rows = newMatrix.rows;
    A->cols = newMatrix.cols;
    freeMatrix(&newMatrix);
}

void dotProduct(float* result, Vector* x, Vector* y)
{
    if(!(x->allocated && y->allocated))
        return;
    if(x->n != y->n)
        return;

    *result = 0.0;
	uint16_t n = x->n;
    uint16_t i;
    for(i = 0; i < n; i++)
        *result += x->buf[i] * y->buf[i];
}

void multiplyMatrix2Vector(Vector* y, Matrix* A, Vector* x)
{
    // Make sure A, B, and C are allocated.    
    if(!(A->allocated && x->allocated && y->allocated))
        return;

    if(!(y->n == A->cols) && (y->n == x->n))
        return;
     
    uint16_t i;
    for(i = 0; i < A->rows; i++)
        dotProduct(&(y->buf[0]), &(A->rowVectors[i]), x);
}
