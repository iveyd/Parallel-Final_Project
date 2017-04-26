#include "support.h"

void freeMatrix( int** matrix, int count ) {
   
    int i;
   
    for ( i = 0 ; i < count; ++i ) {
   
        free( matrix[i] );
   
    }
   
    free( matrix );

}

void createMatrix( int** matrix, int size ) {

    int i,j;
    
    for ( i = 0; i < size; ++i ) {
    
        int* col = ( int* ) calloc( size, sizeof( int ) );
    
        for ( j = 0; j < size; ++j ) {
    
            if ( i != j ) {
    
                if ( ( rand() % 18 ) < 13 ) {
                    
                    col[j] = 1;
                
                }
    
            } 

            // printf("%d\n", col[j]);

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