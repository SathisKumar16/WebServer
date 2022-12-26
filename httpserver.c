#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>
#include "routes.h"
#include<pthread.h>
#define SIZE 1024
#define BACKLOG 10

struct sockaddr_in serverAddress;
char client_msg[8000] ="";
int serverSocket;
char httpHeader[8000]="HTTP/1.1 200 OK\r\n\n";
void report(struct sockaddr_in *serverAddress);
void serverInit();
void *handle_http_request(void* arg);
struct Route * route;
//MATCH THE FILE WITH THE URL
void setHttpHeader(char httpHeader[],char * urlroute,struct Route *jk)
{
	char *r=(urlroute+1);
	FILE *htmlData;
	struct Route* destination = search(jk,urlroute);
	
	//WHEN  FILE IS NOT PRESENT
	if(destination == NULL)
		htmlData=fopen("error.html","r");
	
	else
		htmlData=fopen(destination->value,"r");

	char line[120];
	char respData[8000]="";

	//PASSING THE DATA INTO THE HTTPHEADER
        while(fgets(line,120,htmlData)!=0)
		strcat(respData,line);

	strcat(httpHeader, respData);
	fclose(htmlData);
}

void serverInit()
{
	serverSocket=socket(AF_INET,SOCK_STREAM,0);

	//struct sockaddr_in serverAddress;
	serverAddress.sin_family=AF_INET;
	serverAddress.sin_port=htons(8001);
	serverAddress.sin_addr.s_addr=htonl(INADDR_LOOPBACK);

	bind(
	serverSocket,
	(struct sockaddr *) &serverAddress,
	sizeof(serverAddress)
	);

	int listening=listen(serverSocket,BACKLOG);
	if(listening<0){
		printf("Error: The server is not listening.\n");
	}

	report(&serverAddress);
}

void *handle_http_request(void *arg)
{
	int client_Socket= *((int *)arg);
	read(client_Socket,client_msg,8000);
	char *method="";
	char *urlroute="";
	char *client_httpheader=strtok(client_msg,"\n");
	printf("\n clienthttpheader: %s\n",client_httpheader);
        char *clientheadtoken=strtok(client_httpheader," ");
	int tokenflag=0;

	while(clientheadtoken!=NULL)
	{
	        switch(tokenflag)
		{
			case 0:
				method=clientheadtoken;
			case 1:
				urlroute=clientheadtoken;
		}
		clientheadtoken=strtok(NULL," ");
                 tokenflag++;
	}

	printf("\nThe urlroute is :%s\n",urlroute);
        if(strcmp(urlroute,"/favicon.ico")!=0){
        	setHttpHeader(httpHeader,urlroute,route);
		send(client_Socket,httpHeader,sizeof(httpHeader),0);
		printf("%s\n",httpHeader);
	}
	//printf("%d\n",clientSocket);
	close(client_Socket);

	bzero(httpHeader,8000);
       // httpHeader[0]='\0';
        strcpy(httpHeader,"HTTP/1.1 200 OK \r\n\n");

}
int main(void)
{
	pid_t childid;

	serverInit();

	//INITIALIZING THE ROUTE
	//struct Route *
	 route = initroute("/","index.html");

	//ADD YOUR ROUTE HERE
	add(route,"/hi","hi.html");


	while(1)
	{
		client_msg[0] ='\0';

	        int clientSocket =accept(serverSocket, NULL,NULL);

		pthread_t request_thread;

		int *ptr=&clientSocket;

		pthread_create (&request_thread,NULL,handle_http_request,(void *)ptr);

	}
	return 0;
}

//TO GET INFORMATION WHETHER THW SERVER IS LISTENING
void report(struct sockaddr_in *serverAddress)
{
	char hostBuffer[INET6_ADDRSTRLEN];
	char serviceBuffer[NI_MAXSERV];
	socklen_t addr_len=sizeof(*serverAddress);
	int err=getnameinfo(
		(struct sockaddr *)serverAddress,
		addr_len,
		hostBuffer,
		sizeof(hostBuffer),
		serviceBuffer,
		sizeof(serviceBuffer), 
		NI_NUMERICHOST
	);
	if(err!=0)
		printf("It doesnt work");
	printf("\n\nServer Listening on http://%s:%s\n",hostBuffer,serviceBuffer);
}


