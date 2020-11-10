#ifndef httpServer_H
#define httpServer_H

typedef struct{
	char file_name[256];
	int access_times;
}file;

int checkDirectory();
void getResourceContents();
int checkResource(char *);
int fileAccess(char *);
char* get_mime(char []);
void *serveRequest(void *client);
void startServer();

#endif
