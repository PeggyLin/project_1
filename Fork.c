#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
char webpage[]=
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>SellWaveX</title>\r\n"
"<style>body { background-color: #99BBFF }</style></head>\r\n"
"<body><center><h1>Assignment 1</h1><br>\r\n"
"<img src=\"images.jpg\"/></center></body></html>\r\n";

int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr,client_addr;
	socklen_t sin_len = sizeof(client_addr);
	int fd_server, fd_client;
	char buf[2048];
	int fdimg;

	fd_server =socket(AF_INET, SOCK_STREAM, 0);
	if(fd_server <0)
	{
		perror("socket");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);
	
	if(bind(fd_server,(struct sockaddr *) &server_addr,sizeof(server_addr))==-1)
	{
		perror("bind");
		close(fd_server);
		exit(1);
	}
	if(listen(fd_server,10)==-1)
	{
		perror("liesten");
		close(fd_server);
		exit(1);
	}
	while(1)
	{
		fd_client = accept(fd_server,(struct sockaddr *) &client_addr, &sin_len);

		if(fd_client == -1)
		{
			perror("Connection failed....\n");
			continue;
		}
		printf("Got client connection......\n");

		if(!fork())
		{
			/*child process*/
			close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client,buf,2047);

			printf("%s\n",buf);

			if(!strncmp(buf, "GET /images.jpg",15))
                        {
                                fdimg = open("images.jpg", O_RDONLY);
                                sendfile(fd_client, fdimg,NULL,8000);
                                close(fdimg);
                        }
			else
				write(fd_client,webpage,sizeof(webpage)-1);

			close(fd_client);
			printf("closing...\n");
			exit(0);
		}
		/*parent process*/
		wait(NULL);
		close(fd_client);
	}
		
	return 0;
}