/* CMPE 207  Lab Assignment #2
/* Topic : Server Design
/* Author: Group 8
/* Connection oriented file Server - Concurrent Pre-forked Multiprocessing server */

#include <stdio.h>
#define _USE_BSD 1
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#define MAX_SEND_BUF 1000
#define MAX_RECV_BUF 1000
#define MAX_DATA 1000
static int nchildren;
static pid_t *pids;

extern int errno;
int errexit(const char *format,...);
int connectTCP(const char *service);
int connectsock(const char *service,const char *transport);
void handler(int sig);

int connectsock(const char *service,const char *transport)
{
	/*
	Arguments:
	*service   - service associated with desired port
	*transport - name of the transport protocol to use
	*/
	struct sockaddr_in server;
	int listen_fd,type,num;
	memset(&server,0,sizeof(server));
	//INADDR_ANY to match any IP address
	server.sin_addr.s_addr=htons(INADDR_ANY);
	//family name
	server.sin_family=AF_INET;
	//port number
	server.sin_port=htons(10007);

	/* Use protocol to choose a socket type */

	if(strcmp(transport,"udp")==0)
	{
		type=SOCK_DGRAM;
	}
	else
	{
		type=SOCK_STREAM;
	}

    /* Allocate a socket  */
	listen_fd=socket(AF_INET,type,0);

	if(listen_fd<0)
	{
		printf("Socket can't be created\n");
		exit(0);
	}

	/* to set the socket options- to reuse the given port multiple times */
	num=1;

	if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEPORT,(const char*)&num,sizeof(num))<0)
	{
		printf("setsockopt(SO_REUSEPORT) failed\n");
		exit(0);
	}

	/* bind the socket to known port */
	int b;
	b=bind(listen_fd,(struct sockaddr*)&server,sizeof(server));

	if(b<0)
	{
		printf("Error in binding\n");
		exit(0);
	}

	/* Listen Call : Allows 10 connections at the socket to wait in queue  */
	int l;
	l=listen(listen_fd,10);
	if(l<0)
	{
		printf("Error in listening\n");
		exit(0);
	}
	return listen_fd;
}

int connectTCP(const char *service)
{
	return connectsock(service, "tcp");
}

int errexit(const char* format,...)
{
	va_list args;

	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	exit(1);
}

void handler(int sig)
{
	int i;
	/* terminate all children */
	for (i = 0; i < nchildren; i++)
	kill(pids[i], SIGTERM);
	while (wait(NULL) > 0) /* wait for all children */
	;
	exit(0);

}
/* Function for accepting request by every child created */
void child_main(int i, int listenfd, int addrlen)
{
	int connfd;
	int filefunction(int);
	socklen_t clilen;
	struct sockaddr *cliaddr;
	cliaddr = malloc(addrlen);
	for ( ; ; )
	{
		clilen = addrlen;
		connfd = accept(listenfd, cliaddr, &clilen);
		filefunction(connfd); /* process the request */
		close(connfd);
	}

}
/* Function creating child process */
pid_t child_make(int i, int listenfd, int addrlen)
{
	pid_t pid;
	void child_main(int, int, int);
	if ( (pid = fork()) > 0)
	{
		return (pid); /* parent */
	}
	printf("child %d created\n",getpid());
	child_main(i, listenfd, addrlen); /* never returns */
}


int main(char argc,char *argv[])
{
    /* Creates these many number of child processes. */
	nchildren = atoi(argv[1]);

	socklen_t addrlen;
	void sig_int(int);
	pid_t child_make(int, int, int);

	char *service="echo";
    int alen;
    int msock,ssock;

    /* connectTCP is called to create a socket, bind it and place it in passive mode
	    call accept on listening socket to accept the incoming requests */

    msock=connectTCP(service);

   	int i;
   	/* Initializes allocated memory to zero. */
	pids = calloc(nchildren, sizeof(pid_t));
	for (i = 0; i < nchildren; i++)
	pids[i] = child_make(i, msock, addrlen); /* parent returns */

	(void) signal(SIGCHLD,handler);

   	while(1)
        {
            	pause();
        }
}

int filefunction(int ssock)
{
       	char msg[1000];
        char send_buf[MAX_SEND_BUF];
        /* Receive data from client and store it in msg  */
        int rc = recv(ssock,msg,MAX_DATA,0);
        if(rc<0)
        {
           	printf("Error in receiving\n");
       	}
        printf("Message received from client: %s\n",msg);
        /* for reading local file(server file) */
        int file;
        /* Open the file */
        if((file = open(msg,O_RDONLY))<0)
        {
                printf("File does not exist\n");
        }
        else
        {
            printf("child %ld starting \n", (long) getpid());
            printf("File opened successfully\n");
            ssize_t read_bytes, sent_bytes;
            char send_buf[MAX_SEND_BUF];
            /* Read data from file and send it to the client */
            while((read_bytes = read(file, send_buf, MAX_RECV_BUF)) > 0 )
            {
                    printf("%s",send_buf);
                    if( (sent_bytes = send(ssock, send_buf, read_bytes, 0)) < read_bytes )
                    {
                        printf("send error");
                        return -1;
                    }
            }
            close(file);
		    printf("child %ld terminated \n", (long) getpid());
    	}
      	return 0;
}
