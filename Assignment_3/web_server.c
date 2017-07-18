#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define DEFAULT_SERVER_PORT 8000
#define Max_SIZE 1024
#define MAX_LENGTH 128
int listen_sock;
char *root_path;
//HTML Strings to return expected HTTP Response
int build_path(char* path, const char* request_path);
static char* ACCEPTED_MSG =
  "HTTP/1.0 OK\n";
static char* NOT_FOUND_MSG =
  "HTTP/1.0 404 Not Found\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Not Found</h1>\n"
  "  <p>The requested URL was not found on this server.</p>\n"
  " </body>\n"
  "</html>\n";


int main(int argc, char* argv[])
{
     	int port;
     	int cli_sock;
     	char *host;
     	struct sockaddr_in serv_name;
     	size_t len = sizeof(serv_name);

     	/* Handle any command line arguments */
     	if (argc > 1)
    	{
		root_path = argv[3]; //where file is stored...location
        port = atoi(argv[2]);
	  	host = argv[1];
     	}

     	/* Set socket descriptor */
     	if (0 > (listen_sock = socket(AF_INET, SOCK_STREAM, 0)))
     	{
          	perror("Error opening socket\n");
          	exit(1);
     	}

     	bzero(&serv_name, sizeof(serv_name));
     	serv_name.sin_family = AF_INET;
     	serv_name.sin_port = htons(port);

	int num=1;

	if(setsockopt(listen_sock,SOL_SOCKET,SO_REUSEPORT,(const char*)&num,sizeof(num))<0)
	{
		printf("setsockopt(SO_REUSEPORT) failed\n");
		exit(0);
	}

     	/* connect socket to descriptor */
     	if (0 > bind(listen_sock, (struct sockaddr*)&serv_name, sizeof(serv_name)))
     	{
          	perror("Error binding listen_socket\n");
          	exit(1);
     	}

     	listen(listen_sock, 5);  /* listen for connections on the socket */
	while(1)
	{
        printf("\nServer is waiting for new connection.\n");
     	/* accept a connection request from client */
     	cli_sock = accept(listen_sock, (struct sockaddr*)&serv_name, &len);
     	printf("Got connection\n");

	time_t timenow;
	struct tm * timeinfo;
	time (&timenow);
	timeinfo = localtime(&timenow);
     	int count;
     	char buffer[Max_SIZE];
     	char request_str[MAX_LENGTH];
     	char filename[MAX_LENGTH];
     	int fd;
	char * header;
	header = (char *)malloc(Max_SIZE*sizeof(char));
    count = read(cli_sock, buffer, Max_SIZE-1);
    buffer[count] = (char)0;
	printf("\n%s\n",buffer);

     	if (count == 0)
     	{
          	close(cli_sock);
          	return;
     	}
           /**Check the Get Request*/
     	if (1 > sscanf(buffer, "GET %s", request_str))
     	{
		printf("The requested URL %s is not found on this server.\n",filename);
          	send(cli_sock, NOT_FOUND_MSG,strlen(NOT_FOUND_MSG),0);
     	}
     	else
     	{
          	printf("Requested: %s\n", request_str);
          	if (0 != build_path(filename, request_str))
          	{
			printf("The requested URL %s is not found on this server.\n",filename);
			send(cli_sock, NOT_FOUND_MSG,strlen(NOT_FOUND_MSG),0);
          	}
          	else
          	{
               		printf("Filename: %s\n", filename);
			sprintf(header, "Date: %sHostname: %s:%d\nLocation: %s\n\n", asctime(timeinfo), host, port, root_path);
               		fd = open(filename, O_RDONLY);
               		if ((fd < 0) && (errno == EACCES))
               		{
				printf("The requested URL %s is not found on this server.\n",filename);
                    		send(cli_sock, NOT_FOUND_MSG,strlen(NOT_FOUND_MSG),0);
               		}
               		else if (fd < 0)
               		{
				printf("The requested URL %s is not found on this server.\n",filename);
                    		send(cli_sock, NOT_FOUND_MSG,strlen(NOT_FOUND_MSG),0);
               		}
               		else
               		{
		    		size_t readbytes,sendbytes;
                                //Send & Receive Data
				while((readbytes = read(fd,buffer,1024))>0)
				{
					send(cli_sock, ACCEPTED_MSG,strlen(ACCEPTED_MSG),0);
					send(cli_sock, header,strlen(header),0);
					printf("%s\n",buffer);
					if((sendbytes =send(cli_sock,buffer,readbytes,0)) < readbytes)
					{
						perror("send() failed");
						exit(-1);
					}
				}
                    		close(fd);
            }
          	}
     	}
     	close(cli_sock);
}
	printf("Exiting server.\n");
  	close(listen_sock);
     	return 0;
}

int build_path(char* path, const char* request_path)
{
 	int success;
  	bzero(path, MAX_LENGTH);

  	strncpy(path, root_path, MAX_LENGTH);
    	strncat(path, request_path, MAX_LENGTH);
  	return 0;
}
