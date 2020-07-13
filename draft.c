#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <pthread.h>
#include <time.h>


#define THREADS	1000

#define BUFSIZE		4096

int connectsock( char *host, char *service, char *protocol );

void *thread_producer(void *);

/*
**	Client - creates multithreaded producer
*/
int
main( int argc, char *argv[] )
{
	char		buf[BUFSIZE];
	char		*service;
	char		*host = "localhost";
	int		cc;
	int		csock;

	int 	itemsize = 0;

	switch( argc )
	{
		case    3:
			service = argv[1];
			pNumber = atoi(argv[2]);
			break;
		case    4:
			host = argv[1];
			service = argv[2];
			pNumber = atoi(argv[3]);
			break;
		default:
			fprintf( stderr, "usage: chat [host] port\n" );
			exit(-1);
	}

	pthread_t thread_id[pNumber];

	/*	Create the socket to the controller  */
	if ( ( csock = connectsock( host, service, "tcp" )) == 0 )
	{
		fprintf( stderr, "Cannot connect to server.\n" );
		exit( -1 );
	}

//	printf( "The server is ready, please start sending to the server.\n" );
//	printf( "Type q or Q to quit.\n" );
//	fflush( stdout );

	// 	Start the loop
//	while ( fgets( buf, BUFSIZE, stdin ) != NULL )
	//change to for loop
	while(1)
	{
		int	ssock;

		alen = sizeof(fsin);
		ssock = accept( msock, (struct sockaddr *)&fsin, &alen );
		if (ssock < 0)
		{
			fprintf( stderr, "accept: %s\n", strerror(errno) );
			exit(-1);
		} else {
			if( pthread_create( &thread_id[k] , NULL ,  thread_consumer , (void*) &ssock) < 0){
				printf("error creating thread");
				exit(-1);
			}
			if( k >= pNumber){
				k = 0;
			    while(k < pNumber){
			    	pthread_join(thread_id[k++],NULL);
			    }
			    k = 0;
			}
		}
	}
}


//make an item and send it on GO
//everything else should be in the server (semaphore and mutexes)
//keep buffer and counter and all the semaphores

void *thread_producer(void *ssocket)
{
	//		// If user types 'q' or 'Q', end the connection
	//		if ( buf[0] == 'q' || buf[0] == 'Q' )
	//		{
	//			break;
	//		}
	//		if ( buf[0] == 'done' || buf[0] == 'DONE' )
	//				{
	//
	//				}
			buf[0] = 'CONSUME\r\n';
			// Send to the server
			if ( write( csock, buf, strlen(buf) ) < 0 )
			{
				fprintf( stderr, "consumer write: %s\n", strerror(errno) );
				exit( -1 );
			}
			// Read the echo and print it out to the screen
			if ( (cc = read( csock, buf, BUFSIZE )) <= 0 )
	                {
	                	printf( "The server has gone.\n" );
	                        close(csock);
	                        break;
	                }
	                else if((cc = read( csock, buf, BUFSIZE )) == 'GO')
	                {
	                	srand(time(NULL));   // Initialization, should only be called once.
	                	int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
	                    //send
	                } else((cc = read( csock, buf, BUFSIZE )) == 'DONE'){

	                	break;
	                }
			close( csock );
}

