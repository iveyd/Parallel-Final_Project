#include "support.h"

void freeMatrix( int** matrix, int count ) {
   
    int i;
   
    for ( i = 0 ; i < count; ++i ) {
   
        free( matrix[i] );
   
    }
   
    free( matrix );

}

void createMatrix( int** matrix, int row_size, int col_size ) {

    int i,j;
    
    for ( i = 0; i < row_size; ++i ) {
    
        int* col = ( int* ) calloc( col_size, sizeof( int ) );
    
        for ( j = 0; j < col_size; ++j ) {
    
            if ( i != j ) {
    
                col[j] = rand() % 1;
    
            } 

        }
    
        matrix[i] = col;
    
    }

}

void createWeaknessMatrix( int** matrix, int row_size, int col_size ) {

    int i,j;
    
    for ( i = 0; i < row_size; ++i ) {
    
        int* col = ( int* ) calloc( col_size, sizeof( int ) );
    
        for ( j = 0; j < col_size; ++j ) {
    
            // if ( rand() % 2 ) {

                col[j] = 1;

            // }

        }
    
        matrix[i] = col;
    
    }

}

void createArray( int* array, int size ) {

    int i;

    for ( i = 0; i < size; ++i ) {

        array[i] = rand() % NUMTYPES;

    }

}

int getOwner( int index, int row_size ) {

    return index/row_size;

}