all: clcg4.h clcg4.c main.c
	gcc -I. -Wall -O3 -c clcg4.c -o clcg4.o
	mpicc -I. -Wall -g -O3 main.c support.c clcg4.o -o finalp
