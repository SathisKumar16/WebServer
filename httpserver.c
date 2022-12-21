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
#define SIZE 1024
#define BACKLOG 10

void report(struct sockaddr_in *serverAddress);
void setHttpHeader(char httpHeader[]){
	FILE *htmlData=fopen("hi.html","r");
	char line[120];
	char respData[8000];
        while(fgets(line,120,htmlData)!=0){
		strcat(respData,line);
	}
	strcat(httpHeader, respData);
}

int main(void)
{
	char httpHeader[8000]="HTTP/1.1 200 OK\r\n\n";

	int serverSocket=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serverAddress;
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
		return 1;
	}
	report(&serverAddress);
	setHttpHeader(httpHeader);
	int clientSocket;

	while(1){
		clientSocket =accept(serverSocket, NULL,NULL);
		send(clientSocket,httpHeader,sizeof(httpHeader),0);
		close(clientSocket);
	}
	return 0;
}
void report(struct sockaddr_in *serverAddress){
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
