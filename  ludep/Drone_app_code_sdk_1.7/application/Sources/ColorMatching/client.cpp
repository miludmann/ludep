#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "client.hpp"
#include <string>
#include <signal.h>

void error(const char *msg)
{
    perror(msg);
    //exit(0);
}

/**
 *Thread to run separately: client listen for message and parse strings to
 *get and save the last value received
 */
void *ClientListener_Thread(void *params)
{
    Client *self = (Client *)params;
    int n;
    char buffer[256];
    std::string leaderNumber;
    
    while(self->listenerRunning){
    	bzero(buffer,256);
		n = read(self->sockfd, buffer, 255);
		if (n < 0) 
			 error("ERROR reading from socket");
		//printf("%s",buffer);
		else if(0 == strcmp(buffer, "STOP") || 0 == strcmp(buffer, ""))
			self->listenerRunning = false;
		else {
			leaderNumber = buffer;
			leaderNumber = leaderNumber.substr(leaderNumber.rfind(" ")+1);
			self->leaderNumber = atoi(leaderNumber.c_str());
			printf("And the leader is %d\n", self->leaderNumber);
		}
		pthread_yield();
    } 
    pthread_exit(NULL);
}

void *ClientWriter_Thread(void *params)
{
	Client *self = (Client *)params;
    int n;
    char buffer[256]; 
    char* returnCode;
    while(self->writerRunning){
		//printf("Please enter the message: \n");
		bzero(buffer,256);
		returnCode = fgets(buffer,255,stdin);
		if(NULL == returnCode)
			error("ERROR with fgets");
		n = write(self->sockfd, buffer, strlen(buffer));
		if (n < 0) 
			 error("ERROR writing to socket");
		pthread_yield();
    } 
    pthread_exit(NULL);
}

void Client::SendMessage(char* msg)
{
    int n;

	n = write(sockfd, msg, strlen(msg));
	if (n < 0) 
		 error("ERROR writing to socket");
}

Client::Client(const char* ipHost, int port)
{
    enabled = false;
    writerRunning = true;
    listenerRunning = true;
    leaderNumber = -1;

    portno = port;
    
    signal(SIGCHLD, SIG_IGN);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(ipHost);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    printf("+++ Client connection to %s:%d established...\n", ipHost, portno);
    /**
     * Create the Client Threads
	 */
    threadid_writer = pthread_create( &clientwriterthread, NULL, ClientWriter_Thread, (void*) this);  //comment if you don't want to be able to write in the console to the server
    threadid_listener = pthread_create( &clientlistenerthread, NULL, ClientListener_Thread, (void*) this);
	printf("Client threads launched\n");
}

Client::~Client()
{
    //destructor
    printf("--- Client Shutdown\n");

	writerRunning = false;
    listenerRunning = false;
    usleep(1000);
    close(sockfd);
}

