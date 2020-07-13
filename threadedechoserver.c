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
#include <semaphore.h>
#include <stdint.h>

#include "prodcon.h"

//#define	QLEN			5
//#define	BUFSIZE			4096

#define THREADS	512

ITEM **itemBuffer;
int bufferIndex;

int count;
ITEM *buffer[BUFSIZE];

pthread_mutex_t mutex;
sem_t full, empty;

ITEM *makeItem()
{
	int i;
	ITEM *p = malloc( sizeof(ITEM) );
	p->size = random()%80;
	p->letters = malloc(p->size);
	for ( i = 0; i < p->size-1; i++ )
		p->letters[i] = 'X';
	p->letters[i] = '\0';

	return p;
}

void useItem( ITEM *p )
{
//	printf( "%s\n", p->letters );
	free( p->letters );
	free( p );
}

void *produce( int s )
{
	int ssock = (int) s;
	char buf[BUFSIZE];
	//	if GO
	write(ssock, "GO\r\n", 4);
	ITEM *p = malloc(sizeof(ITEM));

	int x;
	read(ssock, &x, 4);
	p->size = ntohl(x);
	p->letters = malloc(p->size);
	read(ssock, p->letters, p->size);


	// Wait for room in the buffer
	// while ( count > BUFSIZE );
	sem_wait( &empty );

	pthread_mutex_lock( &mutex );
	// Put the item in the next slot in the buffer
	itemBuffer[bufferIndex] = p;
	bufferIndex++;
	printf( "bufferIndex Count %d.\n", bufferIndex );
	pthread_mutex_unlock( &mutex );

	sem_post( &full );
	write(ssock, "DONE\r\n", 6);
	// Exit
	return 0;
}

void *consume( int s )
{
	int ssock = (int) s;
	char buf[BUFSIZE];
//	ITEM *p;
	// Wait for items in the buffer
	sem_wait( &full );

	pthread_mutex_lock( &mutex );
	// Remove the item and update the buffer
	ITEM *p = itemBuffer[bufferIndex-1];
	itemBuffer[bufferIndex-1] = NULL;
	bufferIndex--;

	printf( "bufferIndex Count %d.\n", bufferIndex );


	pthread_mutex_unlock( &mutex );

	sem_post( &empty );

	int x = htonl(p->size);
	write(ssock, &x, 4);
	write(ssock, p->letters, p->size);
	// Now use it
	useItem( p );
	return 0;
}

int passivesock( char *service, char *protocol, int qlen, int *rport );

void *echo( void *s )
{
	char buf[BUFSIZE];
	int cc;
	int ssock = (intptr_t) s;

	/* start working for this guy */
	/* ECHO what the client says */

		cc = read( ssock, buf, BUFSIZE );
		if ( cc <= 0 )
		{
			printf( "The client has gone.\n" );
			close(ssock);
			pthread_exit(0);
		}
		int produceS = strncmp(buf, "PRODUCE\r\n", 7);
		int consumeS = strncmp(buf, "CONSUME\r\n", 7);
		if(produceS == 0){
			produce(ssock);
			fprintf( stderr, "i've created a producer, ssock = %d.\n", ssock );
		}
		if(consumeS == 0){
			consume(ssock);
			fprintf( stderr, "i've created a consumer, ssock = %d.\n", ssock );
		}
	pthread_exit(0);
}


/*
*/
int
main( int argc, char *argv[] )
{
	char			*service;
	struct sockaddr_in	fsin;
	int			alen;
	int			msock;
	int			ssock;
	int			rport = 0;
	int bufferSize;


	pthread_t threads[THREADS*2];
	int status, j;




	switch (argc)
	{
		case	2:
			// No args? let the OS choose a port and tell the user
			rport = 1;
			bufferSize = atoi(argv[1]);
			break;
		case	3:
			// User provides a port? then use it
			service = argv[1];
			bufferSize = atoi(argv[2]);
			break;
		default:
			fprintf( stderr, "usage: server [port]\n" );
			exit(-1);
	}

	pthread_mutex_init( &mutex, NULL );
	sem_init( &full, 0, 0 );
	sem_init( &empty, 0, bufferSize );

	count = 0;

	msock = passivesock( service, "tcp", QLEN, &rport );
	if (rport)
	{
		//	Tell the user the selected port
		printf( "server: port %d\n", rport );
		fflush( stdout );
	}

	itemBuffer = (ITEM **)malloc(sizeof(ITEM *) * bufferSize);
	bufferIndex = 0;
	pthread_t thread_id;
	int i = 0;
	for(;;)
	{
		int	ssock;


		alen = sizeof(fsin);
		ssock = accept( msock, (struct sockaddr *)&fsin, &alen );
		if (ssock < 0)
		{
			fprintf( stderr, "accept: %s\n", strerror(errno) );
			break;
		}

//		printf( "A client has arrived for echoes.\n" );
		fflush( stdout );

		pthread_create( &thread_id, NULL, echo, (void *) (intptr_t) ssock );
	}
}
