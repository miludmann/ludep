/*
 * LegoServer.h
 *
 *  Created on: Jan 7, 2010
 *      Author: mark
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <netinet/in.h>
#include <netdb.h>
#include <deque>

#ifndef LEGOSERVER_H_
#define LEGOSERVER_H_

#endif /* LEGOSERVER_H_ */

#include <stdio.h>

int const PORTNO = 4242;
char * const IPHOST = "10.11.88.168";
//char * const IPHOST = "127.0.0.1";

using namespace std;

struct Robot
{
	int x;
	int y;
	int phys_x;
	int phys_y;
	int phys_x_old;
	int phys_y_old;
	int angle; //orientation with respect to x-axis

	//Used for  comm
	int socket;
	int robotNumber;
	
	deque<int> positions_x;
	deque<int> positions_y;
	//int positions_x[10]; //Holds position history used in orientation calculations
	//int positions_y[10];
	int positions_array_counter;
};


class ColorMatcher
{
	public:
	ColorMatcher();
	~ColorMatcher();
	void SendMessage(char* msg);
  
	Robot RobotOne, RobotTwo;
	
	private:
    pthread_t analyzeVideo;
};
