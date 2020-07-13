#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdint.h>


#include <pthread.h>
#include <time.h>
#include "prodcon.h"


char		*service;
char		*host = "localhost";

int connectsock( char *host, char *service, char *protocol );

void *thread_producer(void *);

/*
**	Client - produces multithreaded consumers
**	1. try to make 2 multithreaded clients
*/
int main( int argc, char *argv[] )
{
	char		buf[BUFSIZE];
	char		*service;
	char		*host = "localhost";
	int		cc;
	int		csock;
	int			ssock;
	int			msock;
	int			rport = 0;
	struct sockaddr_in	fsin;
	int			alen;
	int 	pNumber = 0;

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

	srand(time(NULL));   // Initialization, should only be called once.


	pthread_t thread_id[pNumber];

	for (int k = 0; k < pNumber; k++){

		/*	Create the socket to the controller  */
		if ( ( ssock = connectsock( host, service, "tcp" )) == 0 )
		{
			fprintf( stderr, "Cannot connect to server.\n" );
			exit( -1 );
		}

		pthread_create( &thread_id[k] , NULL , thread_producer , (void *) (intptr_t)ssock);
//		fprintf( stderr, "lolyep producer is here, 73.\n" );
	}
	for (int k = 0; k < pNumber; k++){
		pthread_join(thread_id[k++],NULL);
	}
	exit(0);
}

void *thread_producer(void *sd)
{

	char		buf[BUFSIZE];
	int		cc;
	int		csock = (intptr_t) sd;
	int 	itemsize = 0;
	int			ssock;


	int writeS = write(csock, "PRODUCE\r\n", 9);

	// Send to the server
	if ( writeS < 0 )
	{
		fprintf( stderr, "consumer write: %s\n", strerror(errno) );
		pthread_exit( NULL );
	}
	// Read the echo and print it out to the screen
	cc = read( csock, buf, BUFSIZE );
	int go = strncmp(buf, "GO\r\n", 4);
	fprintf( stderr, "lolyep producer is here, 109, cc = %d; go = %d.\n", cc, go);
	if (cc <= 0 )
	{
		printf( "The server has gone.\n" );
	    close(csock);
	    pthread_exit( NULL );
	}
	else if(go == 0)
	{

    	int r = random()%MAX_LETTERS;      // Returns a pseudo-random integer between 0 and RAND_MAX. MAX_LETTERS
    	int x = ntohl(r);
		writeS = write(csock, &x, 4);


		int i;
		ITEM *p = malloc( sizeof(ITEM) );
		p->size = r;
		p->letters = malloc(p->size);
		for ( i = 0; i < p->size-1; i++ )
			p->letters[i] = 'X';
		p->letters[i] = '\0';
		printf( "Size: %d\n", p->size);
		int writeS = write(csock, p->letters, p->size);

		if (writeS < 0){
				printf("There is the problem with char streaming/n");
			}
		read(csock, buf, BUFSIZE);
		int done = strncmp(buf, "DONE\r\n", 6);
		free( p->letters );
		free( p );
		fprintf( stderr, "lolyep producer is here, 137.\n" );
		if(done == 0){
			close(csock);
			pthread_exit( NULL );
		}
	}
	else
	{
		fprintf( stderr, "There is the problem with writing to the server %s\n", strerror(errno) );
		pthread_exit( NULL );
	}
	close( csock );
	pthread_exit(0);
}
