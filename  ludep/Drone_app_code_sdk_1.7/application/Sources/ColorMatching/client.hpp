/**
 * Client for socket communication
 * Michaël Ludmann (michael@ludep.com)
 * Aarhus University
 * 30/08/11
 */

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class Client
{
	public:
		Client(const char* ipHost, int port);
		~Client();
		void SendMessage(char* msg);
	  
		int sockfd, portno;
		struct sockaddr_in serv_addr;
		struct hostent *server;
		
		int threadid_listener, threadid_writer;
		pthread_t clientlistenerthread, clientwriterthread;
		bool listenerRunning, writerRunning; //If the system is running    
		bool enabled; //If the algorithms are enabled (should run)
		
		int leaderNumber;
    
    private:

};
    
#endif
