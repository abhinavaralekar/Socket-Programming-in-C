//Abhinav Aralekar's Assignment for CMPE207
///////////////////////////////////////////
////////////Socket Programming //////// ///
///////////////////////////////////////////


#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"netdb.h"
#include<time.h>  
#include <fcntl.h>
#define BUF_SIZE 1024 
  
int connectTCP(char * pathname, int port);//function to create connection
char path[1000];

int main(int argc, char**argv)
{  
 	struct sockaddr_in addr, cl_addr;  
 	int sockfd, ret; 
 	struct hostent * server;
 	char * pathname, * temp;
 	int portNumber;
 	char * fileName;
 	char status_ok[] = "OK";
 	char buffer[BUF_SIZE]; 
 	char http_not_found[] = "HTTP/1.0 404 Not Found";
 	char http_ok[] = "HTTP/1.0 200 OK";
 	char location[] = "Location: "; 	
	clock_t start,end,total;

 	if (argc < 3) 
	{
 		printf("usage: [URL] [port number]\n");
  		exit(1);  
	}
	start=clock();
 	pathname = argv[1];
 	portNumber = atoi(argv[2]);
//create socket with the host on the port
	sockfd = connectTCP(pathname, portNumber); 
	memset(&buffer, 0, sizeof(buffer));

   	int flag=0;
	int f;
	ssize_t sent_bytes,rcvd_bytes;
	char recv_str[20000];
	int create = 0;	
	/* Receives data from the Server */
	while((rcvd_bytes = recv(sockfd,recv_str,20000,0))>0)
	{
    	/* Create a file or write to the file if it already exists. */
		if(create == 0)
		{
			printf("creating file\n");
			if((f = open(path, O_WRONLY|O_CREAT, 0644))<0)
   				printf("Error creating file\n");
		}
		create = 1;
		printf("Writing to file\n");
		printf("%s\n",recv_str);

		/* Write to the file, the data received */
		if(write(f,recv_str,rcvd_bytes)<0)
		{
     			printf("error in writing to file");
		}
		flag=1;
		close(f);
		close(sockfd);
	}
	if(flag==0 && rcvd_bytes<=0)
	{
		printf("File not found\n");
	}

	
end=clock();//clock to keep timer
	total=(double)(end-start);
	printf("Time required  : %f\n", (double)end);//printing the end time i.e. total tym involved
 	return 0;
}

int connectTCP(char * pathname, int port)
{
	struct hostent *server;
	int sockfd, bindfd;
	char * ptr, * host;
	char getrequest[1024];
	struct sockaddr_in addr, check;
 	int valid = inet_pton(AF_INET, pathname, &(check.sin_addr));
       //creating the GET REQUEST
	if (valid != 0)
 	{ //when an IP address is given
  		sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", pathname);
 	}
	else 
 	{ //when a host name is given
  		if ((ptr = strstr(pathname, "/")) == NULL) 
		{
   			//when hostname does not contain a slash
                  
   			sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", pathname);
  		} 
		else 
		{
   			//when hostname contains a slash, it is a path to file
   			strcpy(path, ptr);
          		host = strtok(pathname, "/");
   			sprintf(getrequest, "GET %s HTTP/1.0\nHOST: %s\n\n", path, pathname);
  		}
 	} 
	printf("%s\n",getrequest);
 	// creates a socket to the host
 	sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sockfd < 0) 
	{  
  		printf("Error creating socket!\n");  
  		exit(1);  
 	}  
 	printf("Socket created...\n");
	
 	memset(&addr, 0, sizeof(addr));  
 	addr.sin_family = AF_INET;
	server = gethostbyname (pathname);
	bcopy((char*)server->h_addr, (char*)&addr.sin_addr.s_addr, server->h_length); 
 	//addr.sin_addr.s_addr = inet_addr(pathname);
 	addr.sin_port = htons(port);

 	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) 
	{
  		printf("Connection Error!\n");
  		exit(1);
 	}
 	ptr = strtok(path, "/");
 	strcpy(path, ptr);	
 	write(sockfd, getrequest, strlen(getrequest));
 	return sockfd;
}


