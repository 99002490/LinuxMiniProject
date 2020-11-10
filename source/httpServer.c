#include "httpServer.h"

#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<dirent.h>
#include<time.h>
#include<pthread.h>
#include<signal.h>
#include<errno.h>


int main(){

	if(!checkDirectory()){
		printf("directory '\\www' not found\n");
		exit(0);
	}

	getResourceContents();

	int server_fd;
	struct sockaddr_in addr;
	 
	char hostname[1024];
    	
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("socket error");
		exit(0);
	}

	bzero((char *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
    	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    	addr.sin_port = 0;

	if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("bind error");
		exit(0);
	}

	gethostname(hostname, 1024);
	socklen_t addrlen = sizeof(addr);

	if(getsockname(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen) == -1){
		perror("port error");
		exit(0);
	}

	printf("host name : %s\t port : %d\n", hostname, ntohs(addr.sin_port));

	if(listen(server_fd, 5) < 0){
		perror("listen error");
		exit(0);
	}


	while(1){
		
		int new_conn;
		struct sockaddr_in client_fd;
		socklen_t addrcli;
		addrcli = sizeof(client_fd);
		bzero((char *) &client_fd, sizeof(client_fd));
		pthread_t client_td;

		new_conn = accept(server_fd, (struct sockaddr *)&client_fd, (socklen_t*)&addrcli);
	
		if(new_conn < 0){
			perror("accept error");
			exit(0);
		}

		if(pthread_create(&client_td, NULL, serveRequest, &new_conn) != 0){
			perror("error in thread creation");
			close(new_conn);
			pthread_exit(NULL);
		}
	}

	return 0;
}
