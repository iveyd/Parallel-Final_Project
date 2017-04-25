#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <clcg4.h>
#include <mpi.h>


#ifndef __SUPPORT_H__
#define __SUPPORT_H__

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define NUMTYPES 17
#define JUMPVAL 0.80
#define EQUALIZER 0.08

/***************************************************************************/
/* Function Declarations ***************************************************/
/***************************************************************************/

int getOwner( int index, int row_size );
void createArray( int* array, int size );
void freeMatrix( int** matrix, int count );
void createMatrix( int** matrix, int count, int mpi_commsize );
void createWeaknessMatrix( int** matrix, int count, int mpi_commsize );


#endif