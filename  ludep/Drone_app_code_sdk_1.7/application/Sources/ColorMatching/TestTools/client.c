#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "../client.hpp"


Client *client;

// Simple main to test the Client class (launch the server first)
// To compile it: g++ -o Client client.c ../client.hpp ../client.cpp -lpthread
int main(int argc, char *argv[])
{
	int portno = 4242;
	char* iphost = "127.0.0.1";
	
	if (argc < 3) {
		fprintf(stderr,"Default parameters: host = %s:%d\n", iphost, portno);
       exit(0);
    } else {
    	portno = atoi(argv[2]);
    	iphost = argv[1];
    }
	if(client == NULL) client = new Client(iphost, portno);
	
	char buffer[256];
	
	// Send one test string to the server
	strcpy(buffer, "BOOOOM");
	client->SendMessage(buffer);
	
    while(1){}
    return 0;
}
