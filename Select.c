/* include fig01 */
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
#include<errno.h>
#include<sys/sendfile.h>

#define MAXLINE 4096
#define SERV_PORT 9877
#define LISTENQ 1024


char webpage[]=
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>SellWaveX</title>\r\n"
"<style>body { background-color: #6565BF }</style></head>\r\n"
"<body><center><h1>Assignment 1</h1><br>\r\n"
"<img src=\"images.jpg\"/></center></body></html>\r\n";

int main(int argc, char **argv)
{
	int fdimg, i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t	n;
	fd_set	rset, allset;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(8080);

	bind(listenfd, ( struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
/* end fig01 */

/* include fig02 */
	while(1) 
	{
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) 
		{	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));
#endif

			for (i = 0; i < FD_SETSIZE; i++)
			{
				if (client[i] < 0) 
				{
					client[i] = connfd;	/* save descriptor */
					break;
				}
			}
			if (i == FD_SETSIZE)
				perror("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 0; i <= maxi; i++) 
		{	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) 
			{
				if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
						/*4connection closed by client */
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} 
				else if(!strncmp(buf, "GET /images.jpg",15))
                        	{
                                	fdimg = open("images.jpg", O_RDONLY);
                                	sendfile(sockfd, fdimg,NULL,8000);
                                	close(fdimg);
					close(sockfd);
                                        FD_CLR(sockfd, &allset);
                                        client[i]=-1;
                       		}

				else
				{
					write(sockfd, webpage,sizeof(webpage)-1);
					close(sockfd);
                                        FD_CLR(sockfd, &allset);
                                        client[i]=-1;
				}
				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
			printf("%s\n",buf);
			
		}
	}
}
/* end fig02 */
