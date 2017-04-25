/***************************************************************************/
/* Parallel Programming Final Project **************************************/
/* DAVID IVEY **************************************************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include <support.h>

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

void iterate( int** graph, int** weaknesses, int* types, int* type_wins, int* tournament_wins, int iterations );
void battle( int A, int B, int weakness, int* type_wins, int* tournament_wins );
void findTypes( int** matrix, int row_size, int col_size, int* types, int mpi_commsize, int mpi_myrank );
void sendNumNeeded( int* needed, int neededSize, int* types, int mpi_commsize, int mpi_myrank );
void sendNeeded( int* needed, int* gather_send, int* gather_recv, int* types, int mpi_commsize );
void sendAnswers( int* gather_send, int* gather_recv, int* disp_send, int* disp_recv, int wanted_size, int* wanted, int* types, int mpi_commsize );


/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[]) {

    double start, end;
    int mpi_myrank;
    int mpi_commsize;

    // int iterations = atoi(argv[1]);
    int iterations = 10;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank   );

    int** weaknesses = ( int** ) malloc( NUMTYPES * sizeof( int* ) );
    int** graph      = ( int** ) malloc( NODESIZE/mpi_commsize * sizeof( int* ) );

    /* 0 = A->B, 1 = B->A */
    int* tournament_wins  = ( int* ) calloc( 2, sizeof( int ) );

    int* type_wins = ( int* ) calloc( NUMTYPES, sizeof( int ) );
    int* types     = ( int* ) malloc( NODESIZE/mpi_commsize * sizeof( int ) );

    createArray( types, NODESIZE/mpi_commsize );
    createMatrix( weaknesses, NUMTYPES, NUMTYPES );
    createMatrix( graph, NODESIZE/mpi_commsize, NODESIZE );

    iterate( graph, weaknesses, types, type_wins, tournament_wins, iterations );
    
    MPI_Barrier( MPI_COMM_WORLD );
    start = MPI_Wtime();


    if ( mpi_myrank == 0 ) {

        end = MPI_Wtime();

        printf("time elapsed with %d ranks: %f\n", mpi_commsize, ( end - start ) );

    }

    free( types );
    free( type_wins );
    free( tournament_wins );
    freeMatrix( weaknesses, NUMTYPES );
    freeMatrix( graph, NODESIZE/mpi_commsize );

    // END -Perform a barrier and then leave MPI
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;

}

/***************************************************************************/
/* Other Functions *********************************************************/
/***************************************************************************/


void iterate( int** graph, int** weaknesses, int* types, int* type_wins, int* tournament_wins, int iterations ) {
   
    int A, B, _size, i;

    A = rand() % NODESIZE;

    for (i = 0; i < iterations; ++i) {

        _size = 0;

        int* children = ( int* ) malloc( NODESIZE * sizeof( int ) );

        for ( B = 0; B < NODESIZE; ++B ) { //iterate through possible children

            if (graph[A][B]) { //if there is a connection between nodes

                children[_size] = B; //store child

                ++_size;

            }

        }

        if ( GenVal(0) < JUMPVAL && _size != 0 ) {

            B = children[rand() % NODESIZE];

            battle(types[A], types[B], weaknesses[types[A]][types[B]], type_wins, tournament_wins);

            A = B;

        } else {

            A = rand() % NODESIZE;

        }

        free(children);

    }

}

void battle( int A, int B, int weakness, int* type_wins, int* tournament_wins ) {

    if ( weakness ) { // If A is weak against B

        if ( GenVal(0) <= EQUALIZER ) { //Chance that weakness is ignored

            tournament_wins[0] += 1; // A wins
            type_wins[A] += 1; //Type of A wins
        
        } else {
        
            tournament_wins[1] += 1; // B wins
            type_wins[B] += 1; //Type of B wins
        
        }

    } else {

        if ( GenVal(0) <= EQUALIZER ) { //Chance that weakness is ignored
        
            tournament_wins[1] += 1; // B wins
            type_wins[B] += 1; //Type of B wins
        
        } else {
        
            tournament_wins[0] += 1; // A wins
            type_wins[A] += 1; //Type of A wins
        
        }

    }
}

void findTypes( int** matrix, int row_size, int col_size, int* types, int mpi_commsize, int mpi_myrank ) {

    int i, j, isNeeded, _size = 0;

    int* needed = ( int* ) malloc( ( col_size - row_size ) * sizeof( int ) );

    for ( j = 0; j < col_size; ++j ) {

        if ( getOwner(j, row_size ) != mpi_myrank ) {

            isNeeded = 0;
           
            for ( i = 0; i < row_size; ++i ) {

                if ( matrix[i][j] ) { 

                    isNeeded = 1;

                }

            }

            if ( isNeeded ) {

                needed[_size] = j;
                ++_size;

            }

        }

    }

    sendNumNeeded( needed, neededSize, types, mpi_commsize, mpi_myrank );

}

void sendNumNeeded( int* needed, int neededSize, int* types, int mpi_commsize, int mpi_myrank ) {

    int i;

    int* gather_send = ( int* ) calloc( ( mpi_commsize ), sizeof( int ) );
    int* gather_recv = ( int* ) calloc( ( mpi_commsize ), sizeof( int ) );

    for ( i = 0; i < neededSize; ++i ) {

        gather_send[getOwner( needed[i] )] += 1;

    }

    MPI_Alltoall( gather_send, mpi_commsize, MPI_INT, gather_recv, mpi_commsize, MPI_INT, MPI_COMM_WORLD );

    sendNeeded( needed, gather_send, gather_recv, types, mpi_commsize );
    
}

void sendNeeded( int* needed, int* gather_send, int* gather_recv, int* types, int mpi_commsize ) {

    int i;

    int* disp_send = ( int* ) malloc( ( mpi_commsize ) * sizeof( int ) );
    int* disp_recv = ( int* ) malloc( ( mpi_commsize ) * sizeof( int ) );

    disp_send[0] = 0;
    disp_recv[0] = 0;

    for ( i = 1; i < mpi_commsize; ++i ) {

        disp_send[i] = disp_send[i-1] + gather_send[i-1];
        disp_recv[i] = disp_recv[i-1] + gather_recv[i-1];

    }

    int wanted_size = disp_send[mpi_commsize - 1] + gather_send[mpi_commsize - 1];

    int* wanted = ( int* ) malloc( ( wanted_size ) * sizeof( int ) );

    MPI_Alltoallv( needed, gather_send, disp_send, mpi_commsize, MPI_INT, wanted,
                   gather_recv, disp_recv, mpi_commsize, MPI_INT, MPI_COMM_WORLD );

    sendAnswers( gather_send, gather_recv, disp_send, disp_recv, wanted_size, wanted, types, mpi_commsize );

}

void sendAnswers( int* gather_send, int* gather_recv, int* disp_send, int* disp_recv,
                  int wanted_size, int* wanted, int* types, int mpi_commsize ) {

    int i;

    int* replies = ( int* ) malloc( ( wanted_size ) * sizeof( int ) );
    int* answers = ( int* ) malloc( ( wanted_size ) * sizeof( int ) );

    for ( i = 0; i < wanted_size; ++i ) {

        replies[i] = types[wanted[i] % row_size];

    }

    MPI_Alltoallv( replies, gather_recv, disp_recv, mpi_commsize, MPI_INT, answers,
                   gather_send, disp_send, mpi_commsize, MPI_INT, MPI_COMM_WORLD );

}