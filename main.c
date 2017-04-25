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

int NODESIZE;
int GRAPHSIZE;

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

int findWinner( int* type_wins );
void battle( int A, int B, int weakness1, int weakness2, int* type_wins, int* tournament_wins );
void iterate( int** graph, int** weaknesses, int* types, int* type_wins, int* tournament_wins, int iterations );
void sendWinner( int winner, int** weaknesses, int* types, int* type_wins, int* tournament_wins, int mpi_commsize, int mpi_myrank, int iterations );


/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[]) {

    double start, end;
    int mpi_myrank;
    int mpi_commsize;

    // int iterations = atoi(argv[2]);
    int iterations = 100;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank   );

    InitDefault();

    // NODESIZE = atoi(argv[1]);
    NODESIZE = 200;

    GRAPHSIZE = NODESIZE/mpi_commsize;

    int** weaknesses = ( int** ) malloc( NUMTYPES * sizeof( int* ) );
    int** graph      = ( int** ) malloc( GRAPHSIZE * sizeof( int* ) );

    /* 0 = A->B, 1 = B->A */
    int* tournament_wins  = ( int* ) calloc( 2, sizeof( int ) );

    int* type_wins = ( int* ) calloc( NUMTYPES, sizeof( int ) );
    int* types     = ( int* ) malloc( GRAPHSIZE * sizeof( int ) );
    int* total_type_wins = ( int* ) calloc( NUMTYPES, sizeof( int ) );

    createArray( types, GRAPHSIZE );
    createWeaknessMatrix( weaknesses, NUMTYPES, NUMTYPES );
    createMatrix( graph, GRAPHSIZE, GRAPHSIZE );

    iterate( graph, weaknesses, types, type_wins, tournament_wins, iterations );

    int winner = findWinner( type_wins );

    sendWinner( winner, weaknesses, types, type_wins, tournament_wins, mpi_commsize, mpi_myrank, iterations );

    MPI_Reduce( type_wins, total_type_wins, NUMTYPES, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    
    MPI_Barrier( MPI_COMM_WORLD );
    start = MPI_Wtime();


    if ( mpi_myrank == 0 ) {

        end = MPI_Wtime();

        // printf( "winner is: %d\n", types[findWinner( total_type_wins )] );

        printf( "time elapsed with %d ranks: %f\n", mpi_commsize, ( end - start ) );

    }

    free( types );
    free( type_wins );
    free( total_type_wins );
    free( tournament_wins );
    freeMatrix( weaknesses, NUMTYPES );
    freeMatrix( graph, GRAPHSIZE );

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

    A = rand() % GRAPHSIZE;

    for (i = 0; i < iterations; i++) {

        _size = 0;

        int* children = ( int* ) malloc( GRAPHSIZE * sizeof( int ) );

        for ( B = 0; B < GRAPHSIZE; B++ ) { //iterate through possible children

            if (graph[A][B]) { //if there is a connection between nodes

                children[_size] = B; //store child

                ++_size;

            }

        }

        if ( _size != 0 ) {

            B = children[rand() % _size];
            battle(types[A], types[B], weaknesses[types[A]][types[B]], weaknesses[types[B]][types[A]], type_wins, tournament_wins);
        
        }

        if ( GenVal( A ) < JUMPVAL && _size != 0 ) {
            
            A = B;

        } else {

            A = rand() % GRAPHSIZE;

        }

        free(children);

    }

}

void battle( int A, int B, int weakness1, int weakness2, int* type_wins, int* tournament_wins ) {

    if ( weakness1 ) { // If A is weak against B

        if ( GenVal( A ) <= EQUALIZER ) { //Chance that weakness is ignored

            tournament_wins[0] += 1; // A wins
            type_wins[A] += 1; //Type of A wins

            printf("Weakness1(void) type: %d, wins: %d\n", A, type_wins[A]);
        
        } else {
        
            tournament_wins[1] += 1; // B wins
            type_wins[B] += 1; //Type of B wins

            printf("weakness type: %d, wins: %d\n", B, type_wins[B]);
        
        }

    } else {

        if ( weakness2 ) {

            if ( GenVal( A ) <= EQUALIZER ) { //Chance that weakness is ignored
            
                tournament_wins[1] += 1; // B wins
                type_wins[B] += 1; //Type of B wins

                printf("weakness2(void) type: %d, wins: %d\n", B, type_wins[B]);
            
            } else {
            
                tournament_wins[0] += 1; // A wins
                type_wins[A] += 1; //Type of A wins

                printf("weakness2 type: %d, wins: %d\n", A, type_wins[A]);
            
            }

        } else {

             if ( GenVal( A ) <= 0.5 ) { //Chance that weakness is ignored
            
                tournament_wins[1] += 1; // B wins
                type_wins[B] += 1; //Type of B wins

                printf("No weakness(void) type: %d, wins: %d\n", B, type_wins[B]);
            
            } else {
            
                tournament_wins[0] += 1; // A wins
                type_wins[A] += 1; //Type of A wins

                printf("No weakness type: %d, wins: %d\n", A, type_wins[A]);
            
            }

        }

    }
}

int findWinner( int* type_wins ) {

    int i, count, winner;
    winner = 0;
    count = 0;

    for ( i = 0; i < NUMTYPES; i++ ) {

        // printf( "current type: %d, current wins:%d\n", i, type_wins[i] );

        if ( type_wins[i] > count ) {

            // printf( "current type: %d, current wins:%d\n", i, type_wins[i] );

            count = type_wins[i];
            winner = i;

        }
        
    }

    return winner;

}

int modVal( int n, int m ) {

    return ( n + m ) % m;
 
}

void sendWinner( int winner, int** weaknesses, int* types, int* type_wins, int* tournament_wins, int mpi_commsize, int mpi_myrank, int iterations ) {

    int i, challenger;

    MPI_Request reqs_send, reqs_recv;
    MPI_Status status;

    for ( i = 0; i < iterations; i++) {

        MPI_Isend( &winner, 1, MPI_INT, modVal(mpi_myrank+1, mpi_commsize), 0, MPI_COMM_WORLD, &reqs_send );

        MPI_Irecv( &challenger, 1, MPI_INT, modVal(mpi_myrank-1, mpi_commsize), 0, MPI_COMM_WORLD, &reqs_recv );

        MPI_Wait( &reqs_send, &status );
        MPI_Wait( &reqs_recv, &status );
    
        battle( types[winner], types[challenger], weaknesses[winner][challenger], weaknesses[winner][challenger], type_wins, tournament_wins );

    }


}


