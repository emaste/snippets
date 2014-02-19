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
