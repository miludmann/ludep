/**
 * ColorMatcher : image Analysis with OpenCV
 * Michaël Ludmann (michael@ludep.com)
 * Aarhus University
 * 30/08/11
 */

#include "client.hpp"
#include <opencv/highgui.h> // includes highGUI definitions
#include <deque>
#include <string>

#ifndef _COLORMATCHER_H_
#define _COLORMATCHER_H_

using namespace std;

struct Robot
{
	// Position in pixels of the robot in the camera FOV
	int x;
	int y;
	// Real physical position of the robot, in cm
	int phys_x;
	int phys_y;
	int phys_x_old;
	int phys_y_old;
	int angle; //orientation with respect to x-axis

	//Used for network communication
	int socket;
	int robotNumber;
	string robotNumber_str;
	
	deque<int> positions_x;
	deque<int> positions_y;
	int positions_array_counter;
	
	bool onTheField; // true as soon as the robot as been seen in the camera FOV for the very first time
	
	// Variables used for the color blob detection; depends on the setup (may vary according to lightning conditions)
	int h_val, s_val, v_val, h_val_max, s_val_max, v_val_max;
	
	int displayControlPanel; // Used to display the GUI specific to this robot
};

class ColorMatcher
{
	public:
		ColorMatcher(bool withWebcam, bool video);
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
		int getRobotsNumber();
		void addDefaultRobotSet();
		void addRobot(int h_val, int s_val, int v_val, int h_val_max, int s_val_max, int v_val_max);
		Robot* getRobot(int i);
		
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
	
		IplImage* frame;
		//GUI
		bool displayVideo;
		bool displayLog;
	
	private:
		void mapPosition(Robot* rpos);
		void mapPositions();
		void sendPositionToRobots();
	
		void setupGUI();
		void destroyWindows();
		void setDefaultValues();
		int setupWebcam();
		void lookForRobot(IplImage* frame, IplImage* frameCopy, Robot* robot);
		
		int PORTNO ;
		const char* IPHOST;		
		Client *client;
		CvFont font;
			
		//A structure for each robot
		vector<Robot> Robots;
		
		//Webcam
		CvCapture* capture;
		bool webcamUsed;
};

#endif
