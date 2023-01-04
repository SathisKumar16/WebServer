

//INCLUDE SECTION
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


//MACRO'S DEFTINITION
#define SIZE 1024
#define BACKLOG 10
#define PORT 6969

//GLOBAL DECLARATION AND FUNCTIONS
struct sockaddr_in serverAddress;
char client_msg[8000] ="";
int serverSocket;
char error_404[] = "HTTP/1.1 404 Not Found\r\n";
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
void send_browser(char*,int);
void post_msgsave(char*);

//SERVER IS INITIALIZED
void serverInit()
{
	//server socket creation
	serverSocket=socket(AF_INET,SOCK_STREAM,0);

	serverAddress.sin_family=AF_INET;
	serverAddress.sin_port=htons(PORT);
	serverAddress.sin_addr.s_addr=htonl(INADDR_LOOPBACK);

	bind(
	serverSocket,
	(struct sockaddr *) &serverAddress,
	sizeof(serverAddress)
	);

	//whether server listens or not
	int listening=listen(serverSocket,BACKLOG);
	if(listening<0){
		printf("Error: The server is not listening.\n");
	}

	report(&serverAddress);
}

//TO HANDLE THE GET AND POST METHORD
void *handle_http_request(void *arg)
{
	int client_Socket= *((int *)arg);
	read(client_Socket,client_msg,8000);
	char method[20]="";
	char urlroute[20]="";
	char copy[2000]="";
	char firstline[200]={0};
	strcpy(copy,client_msg);
	//parsing the request to find the method  url 
	int i=0;
        while (client_msg[i] != ' ')
        {
		*(method+i)=client_msg[i];
		i++;
	}
	i++;
	int j=0;
	while (client_msg[i] != ' ')
	{
		*(urlroute+j)=client_msg[i];
		i++;
		j++;
	}
	printf("\nmethod:%s\n",method);
	printf("\nurlroute:%s\n",urlroute);

	//checking what type of method the request is 
        if((strcmp(urlroute,"/favicon.ico")!=0) && (strcmp(method,"GET")==0)){
		send_browser(urlroute,client_Socket);
	}
	else if((strcmp(urlroute,"/favicon.ico")!=0) && (strcmp(method,"POST")==0))
	{
		post_msgsave(copy);
		send_browser(urlroute,client_Socket);
	}
}

//DIFFERENTIATES IMAGES AND DOCUMENT
void send_browser(char* urlroute,int client_Socket)
{
	//check whether the url route given is valid
	struct Route* destination = search(route,urlroute);
		if(destination!=NULL)
		{
			char *filename=destination->value;
			int i=0;
			for(;filename[i]!='.';i++);
			i++;
			char *extension=filename+i;
			if((strcmp(extension,"jpg")==0) || (strcmp(extension,"jpeg")==0) || (strcmp(extension,"gif")==0))
				send_image(filename,extension,client_Socket);
			else
				send_resp(filename,client_Socket);
		}
		else{
			send(client_Socket,error_404,strlen(error_404),0);
			close(client_Socket);
		}
}


//SENDS IMAGE TO THE CLIENT SOCKET
void send_image(char *file_name,char *extension,int client_socket)
{
    int ffpr;
    FILE *fpr;
    char buff[100];

    ffpr = open(file_name, O_RDONLY);

    fpr = fopen(file_name, "rb");
    int file_size = 100000;

    //help to find the size of the file
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


    //sending 202 response to web browser
    send(client_socket, buff, strlen(buff), 0);

    //sendfile is use to copy data from one descriptor to another
    sendfile(client_socket, ffpr, NULL, file_size);

    //close file descriptor
    close(ffpr);

    //closing file
    fclose(fpr); 

    //closing the cliwnt socket
    close(client_socket);
}

//SENDS DOCUMENT TO THE CLIENT SOCKET
void send_resp(char* file_name,int client_socket)
{

	FILE *fpr;
   	char get[1000] = {0};
	char send_buff[2000]="";

	//opening the requested file to send
	fpr = fopen(file_name, "r");

        sprintf(send_buff, "%s%s\r\n\r\n", httpHeader, content_type_text_html);

        //coping the request file data into the send buff
        while (fgets(get, 1000, (FILE *)fpr) != 0)
        {
            strcat(send_buff, get);
        }

	send(client_socket, send_buff, strlen(send_buff), 0);

    	fclose(fpr);

	close(client_socket);
}

//THE POST DATA IS SRORED IN THE SERVER AS FILE
void post_msgsave(char *copy)
{

	//THE POST MESSAGE STARTS AFTER \r\n\r\n
	int i=0;
	while(1)
	{
		 if (*(copy+i) == '\r' && *(copy + 1+i) == '\n' && *(copy + 2+i) == '\r' && *(copy + 3+i) == '\n')
       		 {
            		break;
        	 }
		i++;
	}
	//stored only the post data 
	char *postmsg=copy+i;

	//opening a dile and storing the content
	FILE *ptr;
	ptr=fopen("data","w");
	for (int i = 0; postmsg[i] != '\0'; i++)
        {
            putc((int)postmsg[i], ptr);
        }
	fclose(ptr);

}


int main()
{

	//server is initialized
	serverInit();

	//initializing the route
	route = initroute("/","index.html");

	//add your route here--------------------------------------------------
	route=add(route,"/hi","hi.html");

	route=add(route,"/cat","cat.jpeg");

	route=add(route,"/index","index.html");

        route=add(route,"/form","form.html");

	route=add(route,"/upload","uploadfile.html");


	//-----------------------------------------------------------------------


	while(1)
	{
		client_msg[0] ='\0';

	        int clientSocket =accept(serverSocket, NULL,NULL);

		printf("\naccepted\n");

		//Creating the thread
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
