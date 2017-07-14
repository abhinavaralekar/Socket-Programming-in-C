/* CMPE 207  Lab Assignment #2
/* Topic : Server Design
/* Author: Group 8
/* Connectionless file Server - Iterative */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define SERVER_PORT  8002

extern int errno;
int errexit(const char *format,...);
int connectUDP(const char *service);
int connectsock(const char *service,const char *transport);

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
	server.sin_port=htons(10002);

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

int main (int argc, char *argv[])
{
   	int i, len, num, rc, on = 1;
	int accept_sd;
   	char buffer[20000];
  	int msock;
	struct sockaddr_in client;
	int alen = sizeof(client);
	char *service = "echo";
	num=15;
	msock= connectUDP(service);

	printf("The server is ready\n");
	/* Using for loop for num number of iterations. */
   	for(i=0; i< num; i++)
  	{
      		printf("Interation: %d\n", i+1);
      		printf("waiting on accept()\n");
      		/* Receives the data and stores it in buffer */
            int rcd = recvfrom(msock, buffer, 20000, 0,(struct sockaddr *)&client, &alen);
      		if (rcd <= 0)
      		{
         		perror("recv() failed");
         		close(msock);
         		exit(-1);
      		}
		printf("Message Received from Client: %s\n", buffer);
		/* Open the file requested file */
      		int f;
      		if((f=open(buffer, O_RDONLY))<0)
		{
         		printf("File does not exist\n");
			close(msock);
		}
      		else
		{
        		printf("File opened\n");
			size_t readbytes,sendbytes;
			while((readbytes = read(f,buffer,1024))>0)
			{
				printf("%s\n",buffer);
				/* Sends the data from buffer to the client. */
				if((sendbytes =sendto(msock,buffer,readbytes,0,(struct sockaddr*)&client, sizeof(client))) < readbytes)
				{
					perror("send() failed");
					close(msock);
					exit(-1);
				}
			}
		}
	}

      	/* Close the incoming connection  */

      	close(msock);
}
