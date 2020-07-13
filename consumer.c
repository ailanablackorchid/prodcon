#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdint.h>


#include <pthread.h>
#include <fcntl.h>
#include "prodcon.h"


char		*service;
char		*host = "localhost";

int connectsock( char *host, char *service, char *protocol );

void *thread_consumer(void *);

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
	struct sockaddr_in	fsin;
	int			alen;
	int			msock;
	int			ssock;
	int			rport = 0;
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

	pthread_t thread_id[pNumber];

	for (int k = 0; k < pNumber; k++){

		/*	Create the socket to the controller  */
		if ( ( ssock = connectsock( host, service, "tcp" )) == 0 )
		{
			fprintf( stderr, "Cannot connect to server.\n" );
			exit( -1 );
		}

		pthread_create( &thread_id[k] , NULL ,  thread_consumer , (void*) (intptr_t) ssock);
	}
	for (int k = 0; k < pNumber; k++){
		pthread_join(thread_id[k++],NULL);
	}

	exit(0);
}

void *thread_consumer(void *sd)
{
	char		*service;
	char		*host = "localhost";
	char		buf[BUFSIZE];
	int		cc, x, y;
	int		csock = (intptr_t) sd;
	int 	itemsize = 0;

	int writeS = write(csock, "CONSUME\r\n", 9);

	// Send to the server
	if ( writeS < 0 )
	{
		fprintf( stderr, "consumer write: %s\n", strerror(errno) );
		pthread_exit( NULL );
	}
	// Read the echo and print it out to the screen
	cc = read( csock, &x, 4 );
//	fprintf( stderr, "lolyep consumer is here, 110, %d.\n", cc);
	if (cc <= 0 )
	{
		printf( "The server has gone.\n" );
	    close(csock);
	    pthread_exit( NULL );
	}
	else if(cc == 4)
	{
	     //new integer which shows buffersize
	     itemsize = ntohl(x);

	}

	char item[itemsize];
	read( csock, item, itemsize );
	fprintf( stderr, "consumer is here, 114 and itemsize = %d, item = %s.\n", itemsize, item );

	    char filename[64];

	    int out;
	    sprintf(filename, "txtfiles/%ld.txt", (long) pthread_self());

	    out = open( filename, O_RDWR | O_CREAT, S_IRWXU);

	    write(out, item, itemsize);

	    close(out);

	    pthread_exit( NULL );
	close( csock );
	pthread_exit(0);
}

