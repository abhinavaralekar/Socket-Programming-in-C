/* CMPE 207  Lab Assignment #2
/* Topic : Server Design
/* Author: Group 8
/* Connectionless file Server - Concurrent Multithreading server with one thread per request */

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

struct handle_details
{
   	char recvd_msg[20000];
   	struct sockaddr_in sin;
   	int sock;
};

extern int errno;
int errexit(const char *format,...);
int connectUDP(const char *service);
int connectsock(const char *service,const char *transport);
void *handle(void *arg);
/* connectsock-Allocate and connect socket for UDP */

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
	server.sin_port=htons(10006);

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
	return listen_fd;
}

int connectUDP(const char *service)
{
	return connectsock(service, "udp");
}

int errexit(const char* format,...)
{
	va_list args;

	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	exit(1);
}

void *handle(void *arg)
{
	struct handle_details *funct = arg;
	struct sockaddr_in client;
	char msg[2048];
	int ssock;
	/* Structure created to pass multiple objects to the function. */
	ssock = funct->sock;
	client = funct->sin;
	strcpy(msg, funct->recvd_msg);

	printf("Thread created %lu\n", pthread_self);
	/* Open the file */
    int f;
    if((f=open(msg, O_RDONLY))<0)
	{
        printf("File does not exist\n");
		exit(-1);
	}
    else
	{
        printf("File opened\n");
		size_t readbytes,sendbytes;
		char sendmsg[20000];
		/* Read data from file and send it to the client */
		if((readbytes = read(f,sendmsg,20000)))
		{
			printf("%s\n",sendmsg);
			if((sendbytes =sendto(ssock, sendmsg, readbytes, 0, (struct sockaddr_in *)&client, sizeof(client))) < readbytes)
			{
				perror("send() failed");
				exit(-1);
			}
		}
		close(f);
	}
	printf("Thread terminated %lu\n\n", pthread_self);
	pthread_exit;
}

int main(char argc,char *argv[])
{
	char *service="echo";
        pthread_t thread;
	int msock;

	/* connectUDP is called to create a socket, bind it and place it in passive mode
	    call accept on listening socket to accept the incoming requests */

	msock=connectUDP(service);
	char msg[20000];
	struct sockaddr_in client;
	int alen = sizeof(client);
	struct handle_details h;

	while(1)
	{
        /* Receive data from client and store in msg */
        int rc = recvfrom(msock, msg, 20000, 0, (struct sockaddr_in *)&client, &alen);
        if (rc <= 0)
        {
            perror("recv() failed");
            exit(-1);
        }

		strcpy(h.recvd_msg, msg);
		h.sin = client;
		h.sock = msock;

		int *thread_arg=malloc(sizeof(int));
        	*thread_arg=msock;

		printf("Recieved Message: %s\n", msg);
		int val;
		/* Create a new thread to handle every request */
		if((val=(pthread_create(&thread, NULL, handle, &h)))!=0)
        	{
              		printf("error in creating the thread\n");
              		return 1;
        	}
	}
	close(msock);
	return 0;
}
