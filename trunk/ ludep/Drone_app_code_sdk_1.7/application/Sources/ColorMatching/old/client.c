#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "client.hpp"


Client *client;

int main(int argc, char *argv[])
{
	int portno = 4242;
	char* iphost = "10.11.88.168";
	
	if (argc < 3) {
		fprintf(stderr,"Default parameters: host = %s:%d\n", iphost, portno);
       exit(0);
    } else {
    	portno = atoi(argv[2]);
    	iphost = argv[1];
    }
	if(client == NULL) client = new Client(iphost, portno);
	
	char buffer[256];
	strcpy(buffer, "BOOOOM");
	client->SendMessage(buffer);
	
    while(1){}
    return 0;
}
