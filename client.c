#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include <arpa/inet.h>

#define MAX 100
#define PORT 1234
#define SA struct sockaddr



void get(int sockfd) {
	char buff[MAX];
	int n;
	int i;
	for (i=0;i<1000;i++) {
		bzero(buff, MAX);
		recv(sockfd, buff, 100, 0);
		// read(sockfd, buff, sizeof(buff));
		printf("%s", buff);
	}
}

int main(int argc, char* argv[]) {
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	} else
		printf("Socket successfully created..\n");
	bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(PORT);
	if (connect(sockfd, (SA * ) & servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
		
	get(sockfd);
	close(sockfd);	
}