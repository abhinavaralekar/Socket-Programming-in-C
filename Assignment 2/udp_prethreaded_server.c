/* CMPE 207  Lab Assignment #2
/* Topic : Server Design
/* Author: Group 8
/* Connectionless file Server - Concurrent Pre-threaded Multithreading server */

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
int connectUDP(const char *service);
int connectsock(const char *service,const char *transport);
void handler(int);


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
	server.sin_port=htons(10010);

	/*  Choose a socket type */

	if(strcmp(transport,"udp")==0)
	{
		type=SOCK_DGRAM;
	}
	else
	{
		type=SOCK_STREAM;
	}

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
void *handle(void *sock)
{
        int msock=*(int*)sock;
	char msg[20000];
	int ssock;
	int alen;
	while(1)
	{
        struct sockaddr_in fsin;
		alen=sizeof(fsin);
		/* Receive file from client */
        int rc = recvfrom(msock, msg, 20000, 0, (struct sockaddr_in *)&fsin, &alen);
        if (rc<0)
        {
			printf("Receive failed\n");
			exit(0);
		}
		printf("Recieved Message: %s\n", msg);
		/* Open the file */
        int f;
        if((f=open(msg, O_RDONLY))<0)
        printf("File does not exist\n");
        else
		{
			printf("File exists\n");
			size_t readbytes,sendbytes;
			char sendmsg[20000];
			/* Read data from file and add it to sendmsg */
			while((readbytes = read(f,sendmsg,20000))>0)
			{
				printf("%s\n",sendmsg);
				if((sendbytes =sendto(msock,sendmsg,readbytes,0,(struct sockaddr_in *)&fsin, sizeof(fsin))) < readbytes)
				{
					perror("send() failed");
					exit(-1);
				}
				pthread_self();
            }
			close(f);
		}
	}
}

int main(char argc,char *argv[])
{
    /* Creates number of threads entered dynamically */
	int nthreads = atoi(argv[1]);
	char *service="echo";
    int alen;
    pthread_t thread[nthreads];
	int msock;

    msock=connectUDP(service);
	if(msock<0)
	{
		printf("Error in creating socket\n");
		exit(0);
	}

    int *thread_arg=malloc(sizeof(int));
    *thread_arg=msock;

	int i;
	for(i=0; i<nthreads; i++)
	{
	    /* Creates thread */
        if(pthread_create(&thread[i],NULL,handle,thread_arg)!=0)
        {
                printf("Error in creating the thread\n");
                return 1;
        }
	}
	for(i=0; i<nthreads; i++)
	{
        	pthread_join(thread[i],NULL);
	}
	close(msock);
	return 0;
}
