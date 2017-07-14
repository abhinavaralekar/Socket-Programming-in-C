/* CMPE 207  Lab Assignment #2
/* Topic : Server Design
/* Author: Group 8
/* Connectionless file Server - Concurrent Multiprocessing server with one process per request */

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

extern int errno;
int errexit(const char *format,...);
int connectUDP(const char *service);
int connectsock(const char *service,const char *transport);
void handler(int);

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
	server.sin_port=htons(10004);

	/* Use protocol to choose a socket type */
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


int main(char argc,char *argv[])
{
	char msg[20000];
	int msock;
	char *service="echo";

	/* call connectUDP to create a socket */
	msock=connectUDP(service);

	(void) signal(SIGCHLD,handler);

	struct sockaddr_in client;
	int y = sizeof(client);

	while(1)
	{
	    /* Receive connection details, msg from the client and call fork to create a child process  */
		int rc = recvfrom(msock, msg, 20000, 0,(struct sockaddr*)&client, &y);
      		if (rc <= 0)
      		{
         		perror("recv() failed");
			//close(msock);
         		exit(0);
      		}
            printf("\nRecieved Message: %s\n", msg);
	    	int pid;
	    	pid=fork();
	        if(pid==0)
            {
                printf("Child created: %d\n",getpid());
      			int f;
      			if((f=open(msg, O_RDONLY))<0)
                {
         			printf("File not available\n");
                    //close(msock);
         			exit(-1);
                }
      			else
                {
        			printf("File opened\n");
                    size_t readbytes,sendbytes;
                    char sendmsg[20000];
                    /* Read data from file and send it to the client */
                    if((readbytes = read(f,sendmsg,20000))>0)
                    {
                        printf("%s\n",sendmsg);
                        if((sendbytes =sendto(msock,sendmsg,readbytes,0,(struct sockaddr*)&client, sizeof(client))) < readbytes)
                        {
                            perror("send() failed");
                            exit(-1);
                        }
                    }
				close(f);
			}
			printf("Child terminated: %d\n",getpid());
			exit(0);
		}
	        else if(pid==-1)
		{
			printf("error in forking\n");
			//close(msock);
			return 1;
		}
	}
	return 0;
}
