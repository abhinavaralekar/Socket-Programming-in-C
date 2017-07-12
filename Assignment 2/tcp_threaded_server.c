/* CMPE 207  Lab Assignment #2
/* Topic : Server Design
/* Author: Group 8
/* Connection oriented file Server - Concurrent Multithreading server with one thread per request */

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
#include <pthread.h>
#include <fcntl.h>


extern int errno;

int errexit(const char *format,...);

int connectTCP(const char *service);
int connectsock(const char *service,const char *transport);
void handler(int);


 /* connectsock-Allocate and connect socket for TCP */

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
	server.sin_port=htons(10005);

	/*  Choose a socket type */
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

/* Function used by parent to complete child termination. */
void handler(int sig)
{
 	int status;
	while(wait3(&status,WNOHANG,(struct rusage *)0)>=0);
}
int errexit(const char* format,...)
{
	va_list args;

	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	exit(1);
}

void *handle(void *sock)
{
    int ssock=*(int*)sock;
	char msg[20000];
	printf("Thread created %lu\n",pthread_self);
    int rc = recv(ssock, msg, 20000, 0);
    if (rc <= 0)
    {
        perror("recv() failed");
        exit(-1);
    }
	printf("Recieved Message: %s\n", msg);
	/* Open the file */
    int f;
    if((f=open(msg, O_RDONLY))<0)
	{
        printf("File does not exist\n");
		close(ssock);
	}
      	else
	{
        printf("File opened\n");
		size_t readbytes,sendbytes;
		char sendmsg[20000];
		/* Read data from file and send it to the client */
		while((readbytes = read(f,sendmsg,20000))>0)
		{
			printf("%s\n",sendmsg);
			if((sendbytes =send(ssock,sendmsg,readbytes,0)) < readbytes)
			{
				perror("send() failed");
				exit(-1);
			}
		}
		close(f);
	}
	printf("Thread terminated %lu\n\n",pthread_self);
}

int main(char argc,char *argv[])
{
	char *service="echo";
    int alen;
    pthread_t thread;
	int msock,ssock;

	/* connectTCP is called to create a socket, bind it and place it in passive mode
	    call accept on listening socket to accept the incoming requests */

	msock=connectTCP(service);
	while(1)
	{
            struct sockaddr_in fsin;
	    	alen=sizeof(struct sockaddr_in);
	    	/* Accept a new child socket connection  */
	    	ssock=accept(msock,(struct sockaddr*)&fsin,&alen);
	    	if(ssock<0)
            {
                errexit("Accept: %s\n",strerror(errno));
            }

            int *thread_arg=malloc(sizeof(int));
            *thread_arg=ssock;
            /* Create a new thread to handle every request */
	        if(pthread_create(&thread,NULL,handle,thread_arg)!=0)
               {
                       printf("error in creating the thread\n");
                       return 1;
               }
	}
	close(msock);
	return 0;
}












