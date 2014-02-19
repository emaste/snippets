/*
 * Andre Albsmeier's pthread resource consumption demo, from
 * http://lists.freebsd.org/pipermail/freebsd-hackers/2014-February/044280.html
 *
 * cc -g -Wall -lpthread albsmeier_pthread_2.c
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define M 1000

pthread_t pthr[M];

void* mythread( void* arg )
{
  return NULL;
}

int main( int argc, const char* const argv[] )
{
  int i;

  while( 1 ) {

    for( i=M; i; i-- )
      if( pthread_create( &pthr[i], NULL, mythread, NULL ) != 0 )
        fprintf( stderr, "pthread_create\n" );

    for( i=M; i; i-- )
      if( pthread_join( pthr[i], NULL ) != 0 )
        fprintf( stderr, "pthread_join\n" );

    putchar( '.' );
    fflush( stdout );
    usleep( 25000 );
  }
}
