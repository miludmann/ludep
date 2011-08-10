/*
 * ColorMatcher.hpp
 *
 *  Created on: August 3, 2011
 *      Author: Michael
 */

#include "client.hpp"
//#include <cv.h> // includes OpenCV definitions
#include <opencv/highgui.h> // includes highGUI definitions
#include <deque>

#ifndef _COLORMATCHER_H_
#define _COLORMATCHER_H_

int const PORTNO = 4242;
char* const IPHOST = "10.11.106.251";
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
	int positions_array_counter;
	
	bool onTheField; // true as soon as the robot as been seen in the camera FOV for the very first time
};

class ColorMatcher
{
	public:
		ColorMatcher(bool withWebcam, bool video, bool controlPanels);
		~ColorMatcher();
		void analyzeFrame(IplImage *frame);
		void sendPosition(Robot* robot);
		void mapAndSendPosition(Robot* robot);
		void startConnections();
		void stopConnection();
		void calculateMappingScale();	
		IplImage* getCurrentFrame();
		void createWindows();
		void destroySelectedWindows();
		void setAltitude(int altitude);
		void setOrientation(int orientation);
		int getLeaderNumber();
		vector<int> getLeaderPosition();
		
		int CONNECTION; //Socket connection switch
		int CAM_DISTANCE_TO_FIELD; //Physically meassured
		int CAM_WIDTH;
		int CAM_HEIGHT;
		int CAM_FOV_ANGLE; //Previously calculated based on cam distance to field and length of cam view
		float MAPPING_SCALE;
		int FIELD_EDGE_SIZE; //Just a standard value - GUI adjustable
		int POSITION_FILTER_THRESHOLD; //Just a standard value - GUI adjustable
		int ENABLE_GAUSSIAN_SMOOTH; //Just a standard value - GUI adjustable
		int FORCE_CONVEX_SHAPES; //Just a standard value - GUI adjustable
		int BOUND_SIZE; //Min. size of the bounding rect on the contours - standard value - GUI adjustable
		int POSITION_TIME; //Used to limit the amount of positions send in order to not overload the  stack
		
		int CAM_ORIENTATION; //Orientation of camera
		
		//Set for red - these are the values that worked best for us in our last setup.
		//They will be different in any other setup. GUI adjustable
		int H_VAL_R1;
		int S_VAL_R1;
		int V_VAL_R1;
		int H_VAL_R1_MAX;
		int S_VAL_R1_MAX;
		int V_VAL_R1_MAX;
		//Set for blue
		int H_VAL_R2;
		int S_VAL_R2;
		int V_VAL_R2;
		int H_VAL_R2_MAX;
		int S_VAL_R2_MAX;
		int V_VAL_R2_MAX;
		//Set for green
		int H_VAL_R3;
		int S_VAL_R3;
		int V_VAL_R3;
		int H_VAL_R3_MAX;
		int S_VAL_R3_MAX;
		int V_VAL_R3_MAX;
	
		IplImage* frame;
		//GUI
		bool displayVideo;
		bool displayControlPanelR1, displayControlPanelR2, displayControlPanelR3;
		bool displayLog;
		int NB_ROBOTS;
	
	private:
		void mapPosition(Robot* rpos);
		void mapPositions();
		void sendPositionToRobots();
	
		void setupGUI();
		void destroyWindows();
		void setDefaultValues();
		int setupWebcam();
		
		Client *client;
		CvFont font;
			
		//A structure for each robot
		Robot RobotOne;
		Robot RobotTwo;
		Robot RobotThree;
		
		//Webcam
		CvCapture* capture;
		bool webcamUsed;
};

#endif
