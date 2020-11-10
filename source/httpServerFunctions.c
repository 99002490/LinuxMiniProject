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

#include "httpServer.h"

file files[256]; /* stores the files from the dir 'www' */

int N = 0;

/* checks if the directory 'www' exists or not */
int checkDirectory(){
	
	struct stat dir;
	if(stat("www", &dir) == 0 && S_ISDIR(dir.st_mode)){
		return 1;
	}
	else{
		return 0;
	}
}

/* add's filenames to the structure files[] */ 
void getResourceContents(){

	DIR *directory;
	struct dirent *dir;
	directory = opendir("www");
	if(directory){
		while((dir = readdir(directory)) != NULL){
			if(dir->d_name[0] != '.'){
				strcpy(files[N].file_name, dir->d_name);
				files[N].access_times = 0;
				N++;
			}
		}
	closedir(directory);
	}
}

/* check if the requested resource is available or not */
int checkResource(char *fileName){
	int i;
	for(i=0; i<N; i++){
		if(!strcmp(files[i].file_name, fileName)){
			files[i].access_times++;
			return 0;
		}
	}
	return 1;
}

/* returns the access times of a file */
int fileAccess(char *fileName){

	int i;
	for(i=0; i<N; i++){
		if(!strcmp(files[i].file_name, fileName)){
			return files[i].access_times;
		}
	}
}

/* gets the mime content of the file
   opens mime.types file from the /etc/ dir
   returns the requested mime type of the file 
   or returns the default mime type if not found
*/
char* get_mime(char filename[]){

	char *a;
	char extension[50];
	char def[] = "application/octet-stream";
	a = strrchr(filename, '.');
	strcpy(extension,a+1);

	char first[1024];
	char *i;
	i = first;
	char sep[] = "' '\t\n";
	memset(first,0,1024);
	if(strcmp(extension,"html") == 0){
		
		strcpy(first,"text/html");
//		printf("%s\n",first);		
		return i;
	}

	char *line = NULL;
	size_t len = 0;
	int r;
	
	FILE *f;

	f = fopen("/etc/mime.types", "r");

	if(!f){
		i = def;
		return i;
	}
	
	while(( r = getline(&line, &len, f)) != -1){
		
		if(!strstr(line,"#") || line[0] == '\n'){
			
//			printf("%s",line);

			char *p;
			char ext[50],temp[1024];
			p = strtok(line, sep);
			if(p != NULL){
			strcpy(temp,p);
			p = strtok(NULL,sep);	
			if(p != NULL){
				strcpy(ext,p);
				char *s;
				s = strstr(ext, extension);
				if(s != NULL){
					strcpy(first,temp);
					return i;
				}
				else{
					continue;
				}
			}}
		}
	}
	free(line);
	fclose(f);
	i = def;
	return i;
}

/* serves the httpRequest for the client, replies with an httpResponse */
void *serveRequest(void *client){

		struct sockaddr_in client_fd;
		socklen_t addrcli;
		int new_conn;
		addrcli = sizeof(client_fd);
		bzero((char *) &client_fd, sizeof(client_fd));
		new_conn = * (int *)client;
		char buffer[256];
		time_t st = time(0);
		struct tm t = *gmtime(&st); 		
		
		bzero(buffer,256);
		int r = read(new_conn,buffer,255);
		if(r < 0){
			printf("Error reading");
			exit(0);	
		}
	
		if(getpeername(new_conn,(struct sockaddr *) &client_fd, &addrcli) == -1){
			perror("Error in get peer name");
		}
	
		char file[256], protocol[256];
		char *p;
//		printf("%s\n",buffer);
		p = strtok(buffer, "GET /");
		strcpy(file, p);
		char buf[1024],date[256], last[256];
		bzero(buf,sizeof(buf));
		if(checkResource(file)){
			strcpy(buf, "HTTP/1.1 404 Not Found\n");
			strcat(buf,"\n");
			write(new_conn, buf, sizeof(buf));
		}
		else{	
			int access;
			access = fileAccess(file);
			struct stat fp;
			bzero(&fp,sizeof(fp));		
			chdir("www");
			stat(file, &fp);
			int content_length = fp.st_size;

			/* Appending httpRespond headers */

			strftime(last , sizeof(last), "%a, %d %b %Y %H:%M:%S %Z\r\n", gmtime(&fp.st_ctime));
			strcpy(buf, "HTTP/1.1 200 Ok\r\n");
			strftime(date , sizeof(date), "%a, %d %b %Y %H:%M:%S %Z\r\n", &t);
			strcat(buf, "Date: ");
			strcat(buf, date);
			strcat(buf, "Server: ASimpleHttpServer\r\n");
			strcat(buf, "Last-Modified: ");
			strcat(buf, last);
			strcat(buf, "Content-Type: ");

			char *mimetype;
			mimetype = get_mime(file);
			strcat(buf, mimetype);
			strcat(buf, "\r\n");
			strcat(buf, "Content-Length: ");
			
			char c[256];
			sprintf(c, "%d\r\n",content_length);
			strcat(buf, c);
			strcat(buf,"\r\n");
			write(new_conn, buf, strlen(buf));

			/* sending the file */

			int f;
			size_t r;
			unsigned char bread[256];
			f = open(file, O_RDONLY);
			if(f == -1){
				printf("error opening file");
				exit(0);
			}
			do{
				memset(&bread, 0, 256);				
				r = read(f, bread, 256);
          			if (r == -1) {
                			perror("read");
                			exit(0);
            			}
            			if (write(new_conn, bread, r) == -1) {
                			perror("write");
                			exit(0);
            			}	
        		} while (r > 0);
			printf("/%s|%s|%d|%d\n",file,inet_ntoa(client_fd.sin_addr),ntohs(client_fd.sin_port),access);
        		close(f);	
		}
		close(new_conn);
	return NULL;
}
