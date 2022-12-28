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
#include <fcntl.h>
#include <sys/sendfile.h>


#define SIZE 1024
#define BACKLOG 10


struct sockaddr_in serverAddress;
char client_msg[8000] ="";
int serverSocket;
char httpHeader[]="HTTP/1.1 200 OK\r\n";
char content_type_text_html[] = "Content-Type: text/html\r\n";
char content_type_text_css[] = "Content-Type: text/css\r\n";
char content_type_image_jpg[] = "Content-Type: image/jpeg\r\n";
char content_type_image_gif[] = "Content-Type: image/gif\r\n";
char content_type_image_png[] = "Content-Type: image/png\r\n";
char content_length[] = "Content-Length:";
void report(struct sockaddr_in *serverAddress);
void serverInit();
void *handle_http_request(void* arg);
struct Route * route;

void send_image(char *,char *,int);
void send_resp(char* ,int);


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
		struct Route* destination = search(route,urlroute);
        	
		if(destination!=NULL)
		{
			char *filename=destination->value;
			int i=0;
			for(;filename[i]!='.';i++);
			i++;
			char *extension=filename+i;
			printf("%s\n",extension);
			if((strcmp(extension,"jpg")==0) || (strcmp(extension,"jpeg")==0) || (strcmp(extension,"gif")==0))
				send_image(filename,extension,client_Socket);
			else
				send_resp(filename,client_Socket);
		}
		else
			send_resp("error.html",client_Socket);
	}

}

void send_image(char *file_name,char *extension,int client_socket)
{
   int ffpr;
    FILE *fpr;
    char buff[100];

    ffpr = open(file_name, O_RDONLY);

    fpr = fopen(file_name, "rb");
    int file_size = 100000;

    // help to find the size of the file
    if (fpr != NULL)
    {
        fseek(fpr, 0, SEEK_END);
        file_size = ftell(fpr);
        fseek(fpr, 0, SEEK_SET);
    }

    //sending image with proper extension to web browser
    if(strcmp(extension,"png") == 0)
    {
        sprintf(buff, "%s%s\r\n", httpHeader, content_type_image_png);
    }
    else if(strcmp(extension,"jpeg") == 0)
    {
        sprintf(buff, "%s%s\r\n", httpHeader, content_type_image_jpg);
    }
    else if(strcmp(extension,"gif") == 0)
    {
        sprintf(buff, "%s%s\r\n", httpHeader, content_type_image_gif);
    }

  //      sprintf(buff, "%s%s\r\n", response_202, content_type_image_jpg);

    // sending 202 response to web browser
    send(client_socket, buff, strlen(buff), 0);

    // sendfile is use to copy data from one descriptor to another
    sendfile(client_socket, ffpr, NULL, file_size);

    // close file descriptor
    close(ffpr);

    // closing file
    fclose(fpr); 

    close(client_socket);
}


void send_resp(char* file_name,int client_socket)
{
	printf("inside sendresp\n");
	FILE *fpr;
   	char get[200] = {0};
	char send_buff[200]="";
	printf("\n filename:%s\n",file_name);
	fpr = fopen(file_name, "r");

        // concatenate the 200 status to send buffer
        sprintf(send_buff, "%s%s\r\n\r\n", httpHeader, content_type_text_html);

        // coping the request file data into the send buffer
        while (fgets(get, 200, (FILE *)fpr) != 0)
        {
            strcat(send_buff, get);
        }
	printf("%s\n",send_buff);
	send(client_socket, send_buff, strlen(send_buff), 0);

    	// closing the file
    	fclose(fpr);

	close(client_socket);
}

int main(void)
{

	serverInit();

	//INITIALIZING THE ROUTE
	//struct Route *
	 route = initroute("/","index.html");

	//ADD YOUR ROUTE HERE
	route=add(route,"/hi","hi.html");

	route=add(route,"/cat","cat.jpeg");

	route=add(route,"/index","index.html");
       
	inorder(route);
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


