/***************************************************************************/
/* Parallel Programming Final Project **************************************/
/* MATTHEW MOHR, DAVID IVEY, JUSTIN ETZINE *********************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include <support.h>

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

int NODESIZE;
int GRAPHSIZE;
int iterations;
int JUMPVAL;
int EQUALIZER;

static char* type_names[NUMTYPES] = { 

        "NORMAL", "FIRE", "WATER", "ELECTRIC", "GRASS", "ICE", 
        "FIGHTING", "POISON", "GROUND", "FLYING", "PSYCHIC",
        "BUG", "ROCK", "GHOST", "DRAGON", "DARK", "STEEL", "FAIRY"
    
};

static int weaknesses[NUMTYPES][NUMTYPES] = {

        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
        { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 }

    };

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

int findWinner( int* type_wins );
void battle( int A, int B, int weakness1, int weakness2, int* type_wins );
void iterate( int** graph, int* types, int* type_wins );
void sendWinner( int winner, int* types, int* type_wins, int mpi_commsize, int mpi_myrank );

void printtArray (int* arr) {
    
    int i;
    
    printf("\n[");
    
    for ( i = 0; i < ( sizeof( arr ) / sizeof( arr[0] ) ); i++ ) {

        printf("%d,",arr[i]);
    
    }
    
    printf("]\n");

}

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[]) {

    double start, end;
    int mpi_myrank;
    int mpi_commsize;

    NODESIZE = atoi(argv[1]);
    // NODESIZE = 20000;
    iterations = atoi(argv[2]);
    // iterations = 10000;
    JUMPVAL = atoi(argv[3]);
    // JUMPVAL = 0.2;
    EQUALIZER = atoi(argv[4]);
    // EQUALIZER = 0.08;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank   );

    InitDefault(); // initialize RNG streams

    /* Split nodes up amongst each process */
    GRAPHSIZE = NODESIZE/mpi_commsize;

    int** graph = ( int** ) malloc( GRAPHSIZE * sizeof( int* ) );

    /* 0 = A->B, 1 = B->A */
    //int* tournament_wins = ( int* ) calloc( 2, sizeof( int ) );

    /* Number of wins for each type */
    int* type_wins = ( int* ) calloc( NUMTYPES, sizeof( int ) ); 
    /* The type for each node in the graph */
    int* types = ( int* ) malloc( GRAPHSIZE * sizeof( int ) );
    /* Total wins for each type from every graph */
    int* total_type_wins = ( int* ) calloc( NUMTYPES, sizeof( int ) );

    /* Randomly initialize the node types array */
    createArray( types, GRAPHSIZE );

    /* 
       Randomly initialize the graph with a 13/18
       chance of having an edge between nodes 
       as we wanted many children for each
       node in the graph.
    */
    createMatrix( graph, GRAPHSIZE );
    
    /* Start MPI timer */
    start = MPI_Wtime();

    /* Iterate through the graph using a random walk algorithm */
    iterate( graph, types, type_wins );

    /* Get winner of processes graph */
    int winner = findWinner( type_wins );

    /* Send the winner of each processes graph to face another processes winner */
    sendWinner( winner, types, type_wins, mpi_commsize, mpi_myrank );

    /* Sum up the total wins for each type for each process */
    MPI_Reduce( type_wins, total_type_wins, NUMTYPES, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );


    if ( mpi_myrank == 0 ) {

        /* End MPI timer */
        end = MPI_Wtime();

        /* Output the winning type */
        printf( "winner is: %s\n", type_names[findWinner( total_type_wins )] );

        /* Output the elapsed time */
        printf( "time elapsed with %d ranks: %f\n", mpi_commsize, ( end - start ) );

    }

    /* Free memory */
    free( types );
    free( type_wins );
    free( total_type_wins );
    freeMatrix( graph, GRAPHSIZE );

    // END -Perform a barrier and then leave MPI
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;

}

/***************************************************************************/
/* Other Functions *********************************************************/
/***************************************************************************/


void iterate( int** graph, int* types, int* type_wins ) {
   

    /* Initialize vars */
    int A, B, _size, i;

    /* Start walk at a random node */
    A = rand() % GRAPHSIZE;

    /* Step through the graph iteration number of times */
    for (i = 0; i < ITERATIONS; i++) {

        /* Set size of children array to zero */
        _size = 0;

        /* Allocate space for children array */
        int* children = ( int* ) malloc( GRAPHSIZE * sizeof( int ) );

        /* Loop through the possible edges of the node */
        for ( B = 0; B < GRAPHSIZE; B++ ) {
           
            /*
                If there is an edge between graph[A] and graph[B].
                A.k.a, graph[B] is a child of graph[A].
            */
            if (graph[A][B]) {
                
                /* Store the location to the children array. */
                children[_size] = B;

                /* Increase children count size */
                ++_size;

            }

        }

        /* If a node has at least one child */
        if ( _size != 0 ) {

            /* Pick a random child from children array */
            B = children[rand() % _size];

            /* Battle the child */
            battle(types[A], types[B], weaknesses[types[A]][types[B]], weaknesses[types[B]][types[A]], type_wins );
        
        }

        /* 
            If the node has children, and the number from the
            RNG is within the jump threshold, set A = B so that
            you are now looking at the child that was selected above
        */
        if ( GenVal( 32768 ) > JUMPVAL && _size != 0 ) {
            
            A = B;

        } else {
            /* 
                Jump to a random Node.
                This helps to prevent loops and dead-ends.
            */
            A = rand() % GRAPHSIZE;

        }

        /* Free memory */
        free(children);

    }

}

void battle( int A, int B, int weakness1, int weakness2, int* type_wins ) {

    // printf("TypeA:%d, TypeB:%d\n", A, B);

    /* If A is weak against B */
    if ( weakness1 ) { 

        /* Chance that weakness is ignored */
        if ( GenVal( 32768 ) <= EQUALIZER ) {

            // tournament_wins[0] += 1; // A wins
            
            /*Type of A wins*/
            type_wins[A] += 1;

            // printf("Weakness1(void) type: %d, wins: %d\n", A, type_wins[A]);
        
        } else {
        
            // tournament_wins[1] += 1; // B wins

            /*Type of B wins*/
            type_wins[B] += 1;

            // printf("weakness type: %d, wins: %d\n", B, type_wins[B]);
        
        }

    } else {

        /* If B is weak against A */
        if ( weakness2 ) {

            /* Chance that weakness is ignored */
            if ( GenVal( 32768 ) <= EQUALIZER ) {
            
                // tournament_wins[1] += 1; // B wins

                /*Type of B wins*/
                type_wins[B] += 1;

                // printf("weakness2(void) type: %d, wins: %d\n", B, type_wins[B]);
            
            } else {
            
                // tournament_wins[0] += 1; // A wins
                
                /*Type of A wins*/
                type_wins[A] += 1;

                // printf("weakness2 type: %d, wins: %d\n", A, type_wins[A]);
            
            }

        } else {

            /* No weakness so each type has a 50% of winning */
            if ( GenVal( 32768 ) <= 0.5 ) {
            
                // tournament_wins[1] += 1; // B wins

                /*Type of B wins*/
                type_wins[B] += 1;


                // printf("No weakness(B) type: %d, wins: %d\n", B, type_wins[B]);
            
            } else {
            
                // tournament_wins[0] += 1; // A wins
            
                /*Type of A wins*/
                type_wins[A] += 1;
                
                // printf("No weakness(A) type: %d, wins: %d\n", A, type_wins[A]);
            
            }

        }

    }
}

int findWinner( int* type_wins ) {

    /* Initialize vars */
    int i, count, winner;
    winner = 0;
    count = 0;

    /* Loop through all of the types */
    for ( i = 0; i < NUMTYPES; i++ ) {

        // printf( "current type: %d, current wins:%d\n", i, type_wins[i] );

        /*
            If the current type has more wins than
            the current winner, update the winner
        */
        if ( type_wins[i] > count ) {

            // printf( "current type: %d, current wins:%d\n", i, type_wins[i] );

            count = type_wins[i];
            winner = i;

        }
        
    }

    return winner;

}

int modVal( int n, int m ) {

    /* Gets rank for sends/recvs */
    return ( n + m ) % m;
 
}

void sendWinner( int winner, int* types, int* type_wins, int mpi_commsize, int mpi_myrank ) {

    /* Initialize vars */
    int i, challenger;
    MPI_Request reqs_send, reqs_recv;
    MPI_Status status;

    /* 
        Use a fraction of the given iterations to minimize scewing type wins
        Each process sends it's winning type to the next with a wraparound,
        then the winners from the two processes battle.
    */
    for ( i = 0; i < ITERATIONS/(ITERATIONS * 0.01); i++) {

        MPI_Isend( &winner, 1, MPI_INT, modVal(mpi_myrank+1, mpi_commsize), 0, MPI_COMM_WORLD, &reqs_send );

        MPI_Irecv( &challenger, 1, MPI_INT, modVal(mpi_myrank-1, mpi_commsize), 0, MPI_COMM_WORLD, &reqs_recv );

        MPI_Wait( &reqs_send, &status );
        MPI_Wait( &reqs_recv, &status );
    
        // printf("MPI battle:: winnerType:%d, ChallengerType:%d :: ", winner, challenger);
        battle( winner, challenger, weaknesses[winner][challenger], weaknesses[winner][challenger], type_wins );

    }


}


