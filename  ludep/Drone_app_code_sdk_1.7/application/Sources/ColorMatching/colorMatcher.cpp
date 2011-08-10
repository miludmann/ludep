#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <string>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <netdb.h>
#include <deque>
#include <opencv/cv.h> // includes OpenCV definitions
//#include <highgui.h> // includes highGUI definitions
#include <iostream>
#include <sstream>
#include <math.h>

#include "colorMatcher.hpp"

using namespace std;

void ColorMatcher::startConnections() {
	if(NULL == client) client = new Client(IPHOST, PORTNO);
	CONNECTION = 1;
}
void ColorMatcher::stopConnection() {
	delete client;
	client = NULL;
	CONNECTION = 0;
}

int ColorMatcher::getLeaderNumber(){
	return (NULL == client)?-1:client->leaderNumber;	
}

void ColorMatcher::calculateMappingScale(){
	float lengthOfFov = tan(CAM_FOV_ANGLE * M_PI / 180) * CAM_DISTANCE_TO_FIELD;
	MAPPING_SCALE = lengthOfFov * 2 / CAM_WIDTH;
}
void ColorMatcher::mapPosition(Robot* rpos) {
	//Store previous position
	rpos->phys_x_old = rpos->phys_x;
	rpos->phys_y_old = rpos->phys_y;
	//Scale new position
	calculateMappingScale();
	rpos->phys_x = (int)(MAPPING_SCALE*rpos->x);
	rpos->phys_y = (int)(MAPPING_SCALE*rpos->y);
	//Fill in the position history
	
	rpos->positions_x.push_back(rpos->phys_x);
	rpos->positions_y.push_back(rpos->phys_y);
	
	if(rpos->positions_x.size() > 10)
		rpos->positions_x.pop_front();
	if(rpos->positions_y.size() > 10)
		rpos->positions_y.pop_front();
	rpos->positions_array_counter++;

	if(rpos->positions_x.size()>= 2){
		////Calculate orientation////
		int tmp_phys_x_old=0;
		int tmp_phys_y_old=0;
		int tmp_phys_x_new=0;
		int tmp_phys_y_new=0;
		int half_size = rpos->positions_x.size()/2;
		for (int i = 0; i <= half_size-1; i++) {
			tmp_phys_x_old += rpos->positions_x[i];
			tmp_phys_y_old += rpos->positions_y[i];
		}
		for (int i = half_size; i <= (int) rpos->positions_x.size()-1; i++) {
			tmp_phys_x_new += rpos->positions_x[i];
			tmp_phys_y_new += rpos->positions_y[i];
		}
		//First find vector old->new
		float vec_x = (float)tmp_phys_x_new/(rpos->positions_x.size() - half_size) - tmp_phys_x_old/half_size;
		float vec_y = (float)tmp_phys_y_new/(rpos->positions_x.size() - half_size) - tmp_phys_y_old/half_size;
		
		if(0 != vec_x || 0 != vec_y) {
			//Find angle between vector and x-axis (orientation)
			float angle = atan2(-vec_y, vec_x);
		
			//Convert to degrees
			angle = angle * 180 / M_PI;
		
			//Update robot structure
			rpos->angle = (int) angle;
		} else {
			//The robot hasn't move, so the the direction stays the same	
		}
	} else {
		rpos->angle = 181;	
	}
	if(displayLog)
		cout << rpos->robotNumber << ": x, y = {" << rpos->phys_x << ", " << rpos->phys_y << "}, angle = " << rpos->angle << endl;
	/////////////////////////////

}

void ColorMatcher::sendPosition(Robot* robot){
	char buffer[256];
	bzero(buffer, 255);
	int pos_x_NewCoord = 10*(robot->phys_x - CAM_WIDTH/2 * MAPPING_SCALE); //convert to mm and translate coordinate system
	int pos_y_NewCoord = -10*(robot->phys_y - CAM_HEIGHT/2 * MAPPING_SCALE);
	//Warn robot to no drive outside field
	bool edge_warning = robot->phys_x < 0 + FIELD_EDGE_SIZE ||
		robot->phys_x > CAM_WIDTH*MAPPING_SCALE - FIELD_EDGE_SIZE ||
		robot->phys_y < 0 + 10 + FIELD_EDGE_SIZE || /*+10 = physical field not big enough, thus reduce virtual size*/
		robot->phys_y > CAM_HEIGHT*MAPPING_SCALE - FIELD_EDGE_SIZE;
		
    sprintf(buffer, "# %d : { %d , %d }, %d rbt, %d cam, %s ew\n", robot->robotNumber, pos_x_NewCoord, pos_y_NewCoord, robot->angle, CAM_ORIENTATION, (edge_warning)?"true":"false");
	cout << buffer;
	client->SendMessage(buffer);
}

void ColorMatcher::mapPositions() {
	for(int i=0; i<getRobotsNumber() ; i++)
	{
		if(Robots[i].onTheField)
			mapPosition(&Robots[i]);
	}
}
void ColorMatcher::sendPositionToRobots() {
	if (CONNECTION) {
		for(int i=0; i<getRobotsNumber() ; i++)
		{
			if(Robots[i].onTheField)
				sendPosition(&Robots[i]);
		}
	}
}

void ColorMatcher::mapAndSendPosition(Robot* robot)
{
	mapPosition(robot);
	if(CONNECTION)
		sendPosition(robot);	
}


vector<int> ColorMatcher::getLeaderPosition()
{
	vector<int> result (2, -1);
	if(-1 == getLeaderNumber()) // Program just started or no leader has been chosen
	{
		for(int i=0; i<getRobotsNumber(); i++)
		{
			if(Robots[i].onTheField)
			{
				result[0] = Robots[i].x;	
				result[1] = Robots[i].y;	
				return result; // first robot found is used for an immediate return value
			}
		}
		return result; // return default result
	}
	else 
	{
		int i = getLeaderNumber() - 1;
		if(Robots[i].onTheField)
		{
			result[0] = Robots[i].x;	
			result[1] = Robots[i].y;	
		}
		return result;
	}
}

static void setBoundSize(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->BOUND_SIZE = pos;
}
static void setDistance(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (myObj->CAM_DISTANCE_TO_FIELD != pos) {
		myObj->CAM_DISTANCE_TO_FIELD = pos;
		myObj->calculateMappingScale();
	}
}
static void setPositionFilterThreshold(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->POSITION_FILTER_THRESHOLD = pos;
}
static void setGaussian(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->ENABLE_GAUSSIAN_SMOOTH = pos;
}
static void setConvex(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->FORCE_CONVEX_SHAPES = pos;
}
static void setEdgeSize(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj; //recast
	myObj->FIELD_EDGE_SIZE = pos;
}
static void toggle(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->stopConnection();
	} else if (pos == 1) {
		myObj->startConnections();
	}
}
static void toggleVideo(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->displayVideo = false;
		myObj->destroySelectedWindows();
	} else if (pos == 1) {
		myObj->displayVideo = true;
		myObj->createWindows();
	}
}

static void togglePanel(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if(0 == pos)
		myObj->destroySelectedWindows();
	else if (1 == pos)
		myObj->createWindows();
}

static void toggleLog(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->displayLog = false;
	} else if (pos == 1) {
		myObj->displayLog = true;
	}
	
}

Robot* ColorMatcher::getRobot(int i)
{
	return &Robots[i];	
}

void ColorMatcher::createWindows()
{
	if(displayVideo)
		cvNamedWindow("Video", CV_WINDOW_NORMAL);
	
	for(int i=0; i<getRobotsNumber(); i++)
	{
		if(Robots[i].displayControlPanel)
		{			
			string title_str("Contours-R#"+Robots[i].robotNumber_str);
			const char* title_char = title_str.c_str();
			cvNamedWindow(title_char);
			
			title_str = "Settings-R#"+Robots[i].robotNumber_str;
			title_char = title_str.c_str();
			cvNamedWindow(title_char);
			
			//Create trackbars for settings
			cout << Robots[0].h_val << " " << Robots[1].h_val << " " << Robots[2].h_val << " " << Robots[3].h_val << endl;
			cvCreateTrackbar2("B Val", title_char, &(Robots[i].h_val),255, NULL, this);
			cout << Robots[0].h_val << " " << Robots[1].h_val << " " << Robots[2].h_val << " " << Robots[3].h_val << endl;
			cvCreateTrackbar2("G Val", title_char, &(Robots[i].s_val),255, NULL, this);
			cvCreateTrackbar2("R Val", title_char, &(Robots[i].v_val),255, NULL, this);
			cvCreateTrackbar2("B Val Max", title_char, &(Robots[i].h_val_max),255, NULL, this);
			cvCreateTrackbar2("G Val Max", title_char, &(Robots[i].s_val_max),255, NULL, this);
			cvCreateTrackbar2("R Val Max", title_char, &(Robots[i].v_val_max),255, NULL, this);
		}
	}
}

void ColorMatcher::destroySelectedWindows()
{
	if(!displayVideo)
		cvDestroyWindow("Video");
	
	for(int i=0; i<getRobotsNumber(); i++)
	{
		if(0 == Robots[i].displayControlPanel)
		{
			string title_str("Settings-R#"+Robots[i].robotNumber_str);
			const char* title_char = title_str.c_str();
			cvDestroyWindow(title_char);
			
			title_str = "Contours-R#"+Robots[i].robotNumber_str;
			title_char = title_str.c_str();
			cvDestroyWindow(title_char);
		}
	}
}

void ColorMatcher::setupGUI()
{
	int video, log;
	displayVideo ? video=1: video=0;
	displayLog ? log=1: log=0;
	//////////////////// GUI SETUP //////////////////////
	//Create windows
	cvStartWindowThread();
	cvNamedWindow("Settings");
	
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1);
	//Create trackbars for settings
	cvCreateTrackbar2("Field Edge size", "Settings", &FIELD_EDGE_SIZE, 100, setEdgeSize, this);
	cvCreateTrackbar2("Cam distance", "Settings", &CAM_DISTANCE_TO_FIELD, 500, setDistance, this);
	cvCreateTrackbar2("Position filter threshold","Settings",&POSITION_FILTER_THRESHOLD,100,setPositionFilterThreshold, this);
	cvCreateTrackbar2("Bound min size","Settings",&BOUND_SIZE,200,setBoundSize, this);
	cvCreateTrackbar2("Smoothing (Gaussian)","Settings",&ENABLE_GAUSSIAN_SMOOTH,1,setGaussian, this);
	cvCreateTrackbar2("Require convex shapes","Settings",&FORCE_CONVEX_SHAPES,1,setConvex, this);
	cvCreateTrackbar2("Socket I/O","Settings",0,1,toggle, this);
	cvCreateTrackbar2("Video","Settings",&video,1,toggleVideo, this);
	cvCreateTrackbar2("Log","Settings",&log,1,toggleLog, this);

	for(int i=0; i<getRobotsNumber(); i++)
	{
		string title_str("PanelR"+Robots[i].robotNumber_str);
		const char* title_char = title_str.c_str();
		cvCreateTrackbar2(title_char, "Settings", &Robots[i].displayControlPanel, 1, togglePanel, this);	
	}
	
	// Create remaining windows & panels
	createWindows();
	////////////////////////////////////////////////////	
}

void ColorMatcher::destroyWindows()
{
	cvDestroyAllWindows();	
}

void ColorMatcher::lookForRobot(IplImage* frame, IplImage* frameCopy, Robot* robot)
{
	//Prepare colour mask image structure
	IplImage* colorMask = cvCreateImage(
			cvSize(
					frame->width,
					frame->height
			),
			8, //depth 8
			1 //single channel
	);
	//Compute the color mask
	cvInRangeS(frameCopy, cvScalar(robot->h_val, robot->s_val, robot->v_val), cvScalar(robot->h_val_max, robot->s_val_max, robot->v_val_max), colorMask);
	//Prepare for contours
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* first_contour = NULL;

	//Find countours for Robot
	int numberOfContours = cvFindContours(colorMask, storage, &first_contour);
	int ok_contour = 0;
	CvRect bound, bound_max;
	CvRect pos;
	for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) {
		bound = cvBoundingRect(c, 0);

		//If force convex shapes (of contours) is enabled, to the check
		if (cvCheckContourConvexity(c) || !FORCE_CONVEX_SHAPES) {
			//Filter rects that are too small
			if (bound.width > BOUND_SIZE || bound.height > BOUND_SIZE) {

				//Draw 'detection' rect around detected object(s)
				cvRectangle(
						frame,
						cvPoint(bound.x, bound.y),
						cvPoint(bound.x+bound.width, bound.y+bound.height),
						CV_RGB(robot->v_val, robot->s_val, robot->h_val)
				);

				if(0 == ok_contour || (bound.width >= bound_max.width && bound.height >= bound_max.height) || (bound.height >= bound_max.width && bound.width >= bound_max.height)) 
				{
					bound_max = bound;
					//save this for future use
					pos = cvBoundingRect(c, 0);
				}
				ok_contour++;
			} else {
				numberOfContours--;	
			}
		} else {
			//If we filter something out
			numberOfContours--;
		}
	}
	//Reset/release variables
	cvClearMemStorage(storage);
	cvReleaseMemStorage(&storage);
	first_contour = NULL;
	
	//Update Robot structure - only if at least match was found
	if (numberOfContours >= 1) {
		robot->onTheField = true;
		int newX = abs(pos.x+(pos.width/2));
		int newY = abs(pos.y+(pos.height/2));
		if(abs(newX) > 5000) newX = -1; // filter values that make no sense
		if(abs(newY) > 5000) newY = -1; // filter values that make no sense

		//Apply position filter - filter out "small jumps" and absurd values
		if ((POSITION_FILTER_THRESHOLD <= abs(newX - robot->x) ||
			POSITION_FILTER_THRESHOLD <= abs(newY - robot->y)) &&
			(-1 != newX && -1 != newY)) {
			robot->x = newX;
			robot->y = newY;

			//Draw robot identifier
			cvCircle(frame, cvPoint(robot->x, robot->y), 2, CV_RGB(robot->v_val, robot->s_val, robot->h_val), 2);
			string title_str("Robot#"+robot->robotNumber_str);
			const char* title_char = title_str.c_str();
			
			cvPutText(frame, title_char, cvPoint(robot->x+10, robot->y+10), &font, CV_RGB(robot->v_val, robot->s_val, robot->h_val));
			
			//Draw orientation arrow
			cvLine(frame, cvPoint(robot->phys_x_old/MAPPING_SCALE, robot->phys_y_old/MAPPING_SCALE), cvPoint(robot->x, robot->y), CV_RGB(255-robot->v_val, 255-robot->s_val, 255-robot->h_val));
			mapAndSendPosition(robot);
		} else {
			//cout << "Robot# Position filtered!" << endl;
		}
	} else {
		robot->onTheField = false;
		//cout << "Multiple positions for Robot# - NOT sending position" << endl;
	}
	
	//Show results
	if(robot->displayControlPanel)
	{
		string titleContours_str("Contours-R#"+robot->robotNumber_str);
		const char* titleContours_char = titleContours_str.c_str();
		cvShowImage(titleContours_char, colorMask);
	}
		//Clean up
	cvReleaseImage(&colorMask);
}

void ColorMatcher::analyzeFrame(IplImage* frame)
{
	//Apply a Gaussian Blur to reduce noice in frame
	if (ENABLE_GAUSSIAN_SMOOTH == 1) {
		cvSmooth(frame, frame, CV_GAUSSIAN);
	}
	
	// need a copy that is staying untouched by the algo (no drawing on it, so
	// we can still perform image analysis without interferences due to drawing)
	IplImage* frameCopy = cvCloneImage(frame);

	for(int i=0; i<getRobotsNumber(); i++)
	{
		lookForRobot(frame, frameCopy, getRobot(i));
	}
	
	if(displayVideo)
		cvShowImage("Video", frame);

	//15 FPS
	//cvWriteFrame(writer, frame); /*Video recording*/
	cvReleaseImage(&frameCopy);
	cvWaitKey(1000/15);	
}

int ColorMatcher::setupWebcam()
{	
	//Initialize the web cam
	capture = cvCreateCameraCapture(0);
	if (capture == 0) {
		cout << "Failed to open video src\n"<< endl;
		return 0;
	}

	//Prepare image structures
	frame = cvQueryFrame(capture);
	return 1;
}

IplImage* ColorMatcher::getCurrentFrame()
{
	if(webcamUsed)	
	{
		frame = cvQueryFrame(capture);	
	}
	return frame;	
}

/*
 * @param: altitude in mm
 */
void ColorMatcher::setAltitude(int altitude)
{
	CAM_DISTANCE_TO_FIELD = altitude/10;	
	calculateMappingScale();	
}

/*
 * @param: orientation in degrees
 */
void ColorMatcher::setOrientation(int orientation)
{
	CAM_ORIENTATION = orientation;	
}

int ColorMatcher::getRobotsNumber()
{
	return Robots.size();	
}

void ColorMatcher::setDefaultValues()
{
	// configure client/server 
	PORTNO = 4242;
	//string ipHost_str ("10.11.106.251");
	//string ipHost_str ("127.0.0.1");
	//IPHOST = ipHost_str.c_str();
	IPHOST = "10.11.106.251";
	
	CONNECTION = 0; //Socket connection switch
	CAM_DISTANCE_TO_FIELD = 274; //Physically meassured
	if(webcamUsed)
	{
		CAM_WIDTH = 640; //Webcam params
		CAM_HEIGHT = 480; //Webcam params	
		CAM_FOV_ANGLE = 22.34; //Previously calculated based on cam distance to field and length of webcam view	
	} else {
		CAM_WIDTH = 176;  //ARDrone cam
		CAM_HEIGHT = 144; //ARDrone cam
		CAM_FOV_ANGLE = 32; //ARDrone cam
	}
	MAPPING_SCALE = 0;
	FIELD_EDGE_SIZE = 15; //Just a standard value - GUI adjustable
	POSITION_FILTER_THRESHOLD = 0; //Just a standard value - GUI adjustable
	ENABLE_GAUSSIAN_SMOOTH = 1; //Just a standard value - GUI adjustable
	FORCE_CONVEX_SHAPES = 0; //Just a standard value - GUI adjustable
	BOUND_SIZE = 8; //Min. size of the bounding rect on the contours - standard value - GUI adjustable
	POSITION_TIME = 0; //Used to limit the amount of positions send in order to not overload the  stack
	
	CAM_ORIENTATION = 90; //Orientation of camera - GUI adjustable
	
	addDefaultRobotSet();
}

void ColorMatcher::addRobot(int h_val, int s_val, int v_val, int h_val_max, int s_val_max, int v_val_max)
{
	Robot newRobot;
	newRobot.x = -1;
	newRobot.y = -1;
	newRobot.robotNumber = Robots.size()+1;
	stringstream out;
	out<<newRobot.robotNumber;
	newRobot.robotNumber_str = out.str();
	newRobot.angle = 181;
	newRobot.positions_array_counter = 0;
	newRobot.onTheField = false;
	newRobot.displayControlPanel = 0;
	newRobot.h_val = h_val;
	newRobot.s_val = s_val;
	newRobot.v_val = v_val;
	newRobot.h_val_max = h_val_max;
	newRobot.s_val_max = s_val_max;
	newRobot.v_val_max = v_val_max;
	Robots.push_back(newRobot);
}

void ColorMatcher::addDefaultRobotSet()
{
	//Default values that work well in our testing environnment 
	//They will be different in any other setup. GUI adjustable
	addRobot(0, 0, 141, 137, 138, 255);		//red
	addRobot(165, 0, 0, 255, 255, 145); 	//blue
	addRobot(0, 143, 0, 140, 255, 144); 	//dark green
	addRobot(0, 146, 175, 145, 255, 255); 	//yellow
	cout << getRobotsNumber() << " robot(s) added for the camera detection." << endl;
}

ColorMatcher::ColorMatcher(bool withWebcam, bool video)
{
	webcamUsed = withWebcam;
	displayVideo = video;
	displayLog = false;
	setDefaultValues();
	calculateMappingScale();	
	setupGUI();
	
	if(webcamUsed)
		setupWebcam();	
}

ColorMatcher::~ColorMatcher()
{
	if(CONNECTION)
		stopConnection();
	//Clean up
	destroyWindows();
	if(webcamUsed)
		cvReleaseCapture(&capture);
}
