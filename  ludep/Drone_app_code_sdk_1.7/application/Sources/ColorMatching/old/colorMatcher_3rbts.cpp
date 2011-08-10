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
#include <math.h>

#include "colorMatcher.hpp"

using namespace std;

void ColorMatcher::startConnections() {
	if(NULL == client) client = new Client(IPHOST, PORTNO);
	CONNECTION = 1;
}
void ColorMatcher::stopConnection() {
	close(RobotOne.socket);
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
		for (int i = half_size; i <= rpos->positions_x.size()-1; i++) {
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
	printf(buffer);
	client->SendMessage(buffer);
}

void ColorMatcher::mapPositions() {
	mapPosition(&RobotOne);
	mapPosition(&RobotTwo);
	mapPosition(&RobotThree);
}
void ColorMatcher::sendPositionToRobots() {
	if (CONNECTION) {
		if(RobotOne.onTheField)
			sendPosition(&RobotOne);
		if(RobotTwo.onTheField)
			sendPosition(&RobotTwo);
		if(RobotThree.onTheField)
			sendPosition(&RobotThree);
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
		if(RobotOne.onTheField)
		{
			result[0] = RobotOne.x;	
			result[1] = RobotOne.y;	
		}
		else if(RobotTwo.onTheField)
		{
			result[0] = RobotTwo.x;	
			result[1] = RobotTwo.y;	
		}
		else if(RobotThree.onTheField)
		{
			result[0] = RobotThree.x;	
			result[1] = RobotThree.y;	
		}
		return result;
	}
	else 
	{
		if(1 == getLeaderNumber() && RobotOne.onTheField)
		{
			result[0] = RobotOne.x;	
			result[1] = RobotOne.y;
		} 
		else if(2 == getLeaderNumber() && RobotTwo.onTheField)
		{
			result[0] = RobotTwo.x;	
			result[1] = RobotTwo.y;
		} 
		else if(3 == getLeaderNumber() && RobotThree.onTheField)
		{
			result[0] = RobotThree.x;	
			result[1] = RobotThree.y;
		}
		return result;
	}
}

static void setHValR1(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->H_VAL_R1 = pos;
}
static void setSValR1(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->S_VAL_R1 = pos;
}
static void setVValR1(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->V_VAL_R1 = pos;
}
static void setHValR1Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->H_VAL_R1_MAX = pos;
}
static void setSValR1Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->S_VAL_R1_MAX = pos;
}
static void setVValR1Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->V_VAL_R1_MAX = pos;
}
static void setHValR2(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->H_VAL_R2 = pos;
}
static void setSValR2(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->S_VAL_R2 = pos;
}
static void setVValR2(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->V_VAL_R2 = pos;
}
static void setHValR2Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->H_VAL_R2_MAX = pos;
}
static void setSValR2Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->S_VAL_R2_MAX = pos;
}
static void setVValR2Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->V_VAL_R2_MAX = pos;
}
static void setHValR3(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->H_VAL_R3 = pos;
}
static void setSValR3(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->S_VAL_R3 = pos;
}
static void setVValR3(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->V_VAL_R3 = pos;
}
static void setHValR3Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->H_VAL_R3_MAX = pos;
}
static void setSValR3Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->S_VAL_R3_MAX = pos;
}
static void setVValR3Max(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	myObj->V_VAL_R3_MAX = pos;
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

static void togglePanelR1(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->displayControlPanelR1 = false;
		myObj->destroySelectedWindows();
	} else if (pos == 1) {
		myObj->displayControlPanelR1 = true;
		myObj->createWindows();
	}
}
static void togglePanelR2(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->displayControlPanelR2 = false;
		myObj->destroySelectedWindows();
	} else if (pos == 1) {
		myObj->displayControlPanelR2 = true;
		myObj->createWindows();
	}
}
static void togglePanelR3(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->displayControlPanelR3 = false;
		myObj->destroySelectedWindows();
	} else if (pos == 1) {
		myObj->displayControlPanelR3 = true;
		myObj->createWindows();
	}
}

static void toggleLog(int pos, void* obj) {
	ColorMatcher* myObj = (ColorMatcher*) obj;
	if (pos == 0) {
		myObj->displayLog = false;
	} else if (pos == 1) {
		myObj->displayLog = true;
	}
	
}
void ColorMatcher::createWindows()
{
	if(displayVideo)
		cvNamedWindow("Video", CV_WINDOW_NORMAL);
	if(displayControlPanelR1)
	{
		cvNamedWindow("Settings-Robot#1");
		cvNamedWindow("Contours-Robot#1");
	
		//Create trackbars for settings
		cvCreateTrackbar2("Robot#1: B Val","Settings-Robot#1",&H_VAL_R1,255,setHValR1, this);
		cvCreateTrackbar2("Robot#1: G Val","Settings-Robot#1",&S_VAL_R1,255,setSValR1, this);
		cvCreateTrackbar2("Robot#1: R Val","Settings-Robot#1",&V_VAL_R1,255,setVValR1, this);
		cvCreateTrackbar2("Robot#1: B Val Max","Settings-Robot#1",&H_VAL_R1_MAX,255,setHValR1Max, this);
		cvCreateTrackbar2("Robot#1: G Val Max","Settings-Robot#1",&S_VAL_R1_MAX,255,setSValR1Max, this);
		cvCreateTrackbar2("Robot#1: R Val Max","Settings-Robot#1",&V_VAL_R1_MAX,255,setVValR1Max, this);
	}
	if(displayControlPanelR2)
	{
		cvNamedWindow("Settings-Robot#2");
		cvNamedWindow("Contours-Robot#2");
	
		//Create trackbars for settings
		cvCreateTrackbar2("Robot#2: B Val","Settings-Robot#2",&H_VAL_R2,255,setHValR2, this);
		cvCreateTrackbar2("Robot#2: G Val","Settings-Robot#2",&S_VAL_R2,255,setSValR2, this);
		cvCreateTrackbar2("Robot#2: R Val","Settings-Robot#2",&V_VAL_R2,255,setVValR2, this);
		cvCreateTrackbar2("Robot#2: B Val Max","Settings-Robot#2",&H_VAL_R2_MAX,255,setHValR2Max, this);
		cvCreateTrackbar2("Robot#2: G Val Max","Settings-Robot#2",&S_VAL_R2_MAX,255,setSValR2Max, this);
		cvCreateTrackbar2("Robot#2: R Val Max","Settings-Robot#2",&V_VAL_R2_MAX,255,setVValR2Max, this);
	}
	if(displayControlPanelR3)
	{
		cvNamedWindow("Settings-Robot#3");
		cvNamedWindow("Contours-Robot#3");
	
		//Create trackbars for settings
		cvCreateTrackbar2("Robot#3: B Val","Settings-Robot#3",&H_VAL_R3,255,setHValR3, this);
		cvCreateTrackbar2("Robot#3: G Val","Settings-Robot#3",&S_VAL_R3,255,setSValR3, this);
		cvCreateTrackbar2("Robot#3: R Val","Settings-Robot#3",&V_VAL_R3,255,setVValR3, this);
		cvCreateTrackbar2("Robot#3: B Val Max","Settings-Robot#3",&H_VAL_R3_MAX,255,setHValR3Max, this);
		cvCreateTrackbar2("Robot#3: G Val Max","Settings-Robot#3",&S_VAL_R3_MAX,255,setSValR3Max, this);
		cvCreateTrackbar2("Robot#3: R Val Max","Settings-Robot#3",&V_VAL_R3_MAX,255,setVValR3Max, this);
	}
}

void ColorMatcher::destroySelectedWindows()
{
	if(!displayVideo)
		cvDestroyWindow("Video");
	if(!displayControlPanelR1)
	{
		cvDestroyWindow("Settings-Robot#1");
		cvDestroyWindow("Contours-Robot#1");
	}
	if(!displayControlPanelR2)
	{
		cvDestroyWindow("Settings-Robot#2");
		cvDestroyWindow("Contours-Robot#2");
	}
	if(!displayControlPanelR3)
	{
		cvDestroyWindow("Settings-Robot#3");
		cvDestroyWindow("Contours-Robot#3");
	}
}

void ColorMatcher::setupGUI()
{
	int video, panelR1, panelR2, panelR3, log;
	displayVideo ? video=1: video=0;
	displayControlPanelR1 ? panelR1=1: panelR1=0;
	displayControlPanelR2 ? panelR2=1: panelR2=0;
	displayControlPanelR3 ? panelR3=1: panelR3=0;
	displayLog ? log=1: log=0;
	//////////////////// GUI SETUP //////////////////////
	//Create windows
	cvStartWindowThread();
	cvNamedWindow("Settings");
	
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 2);
	//Create trackbars for settings
	cvCreateTrackbar2("Field Edge size", "Settings", &FIELD_EDGE_SIZE, 100, setEdgeSize, this);
	cvCreateTrackbar2("Cam distance", "Settings", &CAM_DISTANCE_TO_FIELD, 500, setDistance, this);
	cvCreateTrackbar2("Position filter threshold","Settings",&POSITION_FILTER_THRESHOLD,100,setPositionFilterThreshold, this);
	cvCreateTrackbar2("Bound min size","Settings",&BOUND_SIZE,200,setBoundSize, this);
	cvCreateTrackbar2("Smoothing (Gaussian)","Settings",&ENABLE_GAUSSIAN_SMOOTH,1,setGaussian, this);
	cvCreateTrackbar2("Require convex shapes","Settings",&FORCE_CONVEX_SHAPES,1,setConvex, this);
	cvCreateTrackbar2("Socket I/O","Settings",0,1,toggle, this);
	cvCreateTrackbar2("Video","Settings",&video,1,toggleVideo, this);
	cvCreateTrackbar2("PanelR1","Settings",&panelR1,1,togglePanelR1, this);
	cvCreateTrackbar2("PanelR2","Settings",&panelR2,1,togglePanelR2, this);
	cvCreateTrackbar2("PanelR3","Settings",&panelR3,1,togglePanelR3, this);
	cvCreateTrackbar2("Log","Settings",&log,1,toggleLog, this);
	
	// Create remaining windows & panels
	createWindows();
	////////////////////////////////////////////////////	
}

void ColorMatcher::destroyWindows()
{
	cvDestroyAllWindows();	
}

void ColorMatcher::analyzeFrame(IplImage* frame)
{
	//Apply a Gaussian Blur to reduce noice in frame
	if (ENABLE_GAUSSIAN_SMOOTH == 1) {
		cvSmooth(frame, frame, CV_GAUSSIAN);
	}

	//Prepare colour mask image structure
	IplImage* colorMaskR1 = cvCreateImage(
			cvSize(
					frame->width,
					frame->height
			),
			8, //depth 8
			1 //single channel
	);
	IplImage* colorMaskR2 = cvCreateImage(
			cvSize(
					frame->width,
					frame->height
			),
			8, //depth 8
			1 //single channel
	);
	IplImage* colorMaskR3 = cvCreateImage(
			cvSize(
					frame->width,
					frame->height
			),
			8, //depth 8
			1 //single channel
	);

	//Explained in report
	cvInRangeS(frame, cvScalar(H_VAL_R1, S_VAL_R1, V_VAL_R1), cvScalar(H_VAL_R1_MAX, S_VAL_R1_MAX, V_VAL_R1_MAX), colorMaskR1);
	cvInRangeS(frame, cvScalar(H_VAL_R2, S_VAL_R2, V_VAL_R2), cvScalar(H_VAL_R2_MAX, S_VAL_R2_MAX, V_VAL_R2_MAX), colorMaskR2);
	cvInRangeS(frame, cvScalar(H_VAL_R3, S_VAL_R3, V_VAL_R3), cvScalar(H_VAL_R3_MAX, S_VAL_R3_MAX, V_VAL_R3_MAX), colorMaskR3);

	//Prepare for contours
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* first_contour = NULL;

	//Find countours for Robot#1
	int numberOfContours = cvFindContours(colorMaskR1, storage, &first_contour);
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
						CV_RGB(255, 0, 0)
				);

				if(0 == ok_contour || (bound.width >= bound_max.width && bound.height >= bound_max.height) || (bound.height >= bound_max.width && bound.width >= bound_max.height)) 
				{
					bound_max = bound;
					//save this for future use
					pos = cvBoundingRect(c, 0);
				}
				ok_contour++;
			}
		} else {
			//If we filter something out
			numberOfContours--;
		}
	}
	//Reset variables
	cvClearMemStorage(storage);
	first_contour = NULL;

	//Find contours for Robot#2
	int numberOfContoursR2 = cvFindContours(colorMaskR2, storage, &first_contour);
	ok_contour = 0;
	CvRect boundR2, bound_maxR2;
	CvRect posR2;
	for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) {
		boundR2 = cvBoundingRect(c, 0);

		//If force convex shapes (of contours) is enabled, to the check
		if (cvCheckContourConvexity(c) || !FORCE_CONVEX_SHAPES) {
		//Filter rects that are too small
			if (boundR2.width > BOUND_SIZE || boundR2.height > BOUND_SIZE) {

				//Draw 'detection' rect around detected object(s)
				cvRectangle(
						frame,
						cvPoint(boundR2.x, boundR2.y),
						cvPoint(boundR2.x+boundR2.width, boundR2.y+boundR2.height),
						CV_RGB(0, 0, 255)
				);
			
				if(0 == ok_contour || (boundR2.width >= bound_maxR2.width && boundR2.height >= bound_maxR2.height) || (boundR2.height >= bound_maxR2.width && boundR2.width >= bound_maxR2.height)) 
				{
					bound_maxR2 = boundR2;
					//save this for future use
					posR2 = cvBoundingRect(c, 0);
				}
				ok_contour++;
			}
		} else {
			numberOfContoursR2--;
		}
	}
	
	//Reset variables
	cvClearMemStorage(storage);
	first_contour = NULL;

	//Find contours for Robot#3
	int numberOfContoursR3 = cvFindContours(colorMaskR3, storage, &first_contour);
	ok_contour = 0;
	CvRect boundR3, bound_maxR3;
	CvRect posR3;
	for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) {
		boundR3 = cvBoundingRect(c, 0);

		//If force convex shapes (of contours) is enabled, to the check
		if (cvCheckContourConvexity(c) || !FORCE_CONVEX_SHAPES) {
		//Filter rects that are too small
			if (boundR3.width > BOUND_SIZE || boundR3.height > BOUND_SIZE) {

				//Draw 'detection' rect around detected object(s)
				cvRectangle(
						frame,
						cvPoint(boundR3.x, boundR3.y),
						cvPoint(boundR3.x+boundR3.width, boundR3.y+boundR3.height),
						CV_RGB(0, 255, 0)
				);
			
				if(0 == ok_contour || (boundR3.width >= bound_maxR3.width && boundR3.height >= bound_maxR3.height) || (boundR3.height >= bound_maxR3.width && boundR3.width >= bound_maxR3.height)) 
				{
					bound_maxR3 = boundR3;
					//save this for future use
					posR3 = cvBoundingRect(c, 0);
				}
				ok_contour++;
			}
		} else {
			numberOfContoursR3--;
		}
	}
	
	//Control variable to determine if we should update robots positions
	bool allPositionsFiltered = true;

	//Update Robot#1 structure - only if at least match was found
	if (numberOfContours >= 1) {
		RobotOne.onTheField = true;
		int newX = abs(pos.x+(pos.width/2));
		int newY = abs(pos.y+(pos.height/2));

		//Apply position filter - filter out "small jumps"
		if (POSITION_FILTER_THRESHOLD <= abs(newX - RobotOne.x) ||
			POSITION_FILTER_THRESHOLD <= abs(newY - RobotOne.y)) {
			allPositionsFiltered = false;
			RobotOne.x = newX;
			RobotOne.y = newY;

			//Draw robot identifier
			cvCircle(frame, cvPoint(RobotOne.x, RobotOne.y), 2, CV_RGB(255, 0, 0), 2);
			cvPutText(frame, "Robot#1", cvPoint(RobotOne.x+10, RobotOne.y+10), &font, CV_RGB(255, 0, 0));
			
			//Draw orientation arrow
			cvLine(frame, cvPoint(RobotOne.phys_x_old/MAPPING_SCALE, RobotOne.phys_y_old/MAPPING_SCALE),cvPoint(RobotOne.x, RobotOne.y), CV_RGB(0, 0, 255));
			//cout << "R1 degrees: " << RobotOne.angle << endl;
			mapAndSendPosition(&RobotOne);
		} else {
			//cout << "Robot#1 Position filtered!" << endl;
		}
	} else {
		RobotOne.onTheField = false;
		//cout << "Multiple positions for Robot#1 - NOT sending position" << endl;
	}

	//Update Robot#2 structure - only if at least match was found
	if (numberOfContoursR2 >= 1) {
		RobotTwo.onTheField = true;
		int newX = abs(posR2.x+(posR2.width/2));
		int newY = abs(posR2.y+(posR2.height/2));

		//Apply position filter - filter out "small jumps"
		if (POSITION_FILTER_THRESHOLD <= abs(newX - RobotTwo.x) ||
				POSITION_FILTER_THRESHOLD <= abs(newY - RobotTwo.y)) {
			allPositionsFiltered = false;
			RobotTwo.x = newX;
			RobotTwo.y = newY;

			//Draw robot identifier
			cvCircle(frame, cvPoint(RobotTwo.x, RobotTwo.y), 2, CV_RGB(0, 0, 255), 2);
			cvPutText(frame, "Robot#2", cvPoint(RobotTwo.x+10, RobotTwo.y+10), &font, CV_RGB(0, 0, 255));

			//Draw orientation arrow
			cvLine(frame, cvPoint(RobotTwo.phys_x_old/MAPPING_SCALE, RobotTwo.phys_y_old/MAPPING_SCALE),cvPoint(RobotTwo.x, RobotTwo.y), CV_RGB(255, 0, 0));
			//cout << "R2 degrees: " << RobotTwo.angle << endl;
			mapAndSendPosition(&RobotTwo);
		} else {
			//cout << "Robot#2 Position filtered!" << endl;
		}
	} else {
		RobotTwo.onTheField = false;
		//cout << "Multiple positions for Robot#2 - NOT sending position" << endl;
	}
	
	//Update Robot#3 structure - only if at least a match was found
	if (numberOfContoursR3 >= 1) {
		RobotThree.onTheField = true;
		int newX = abs(posR3.x+(posR3.width/2));
		int newY = abs(posR3.y+(posR3.height/2));

		//Apply position filter - filter out "small jumps"
		if (POSITION_FILTER_THRESHOLD <= abs(newX - RobotThree.x) ||
				POSITION_FILTER_THRESHOLD <= abs(newY - RobotThree.y)) {
			allPositionsFiltered = false;
			RobotThree.x = newX;
			RobotThree.y = newY;

			//Draw robot identifier
			cvCircle(frame, cvPoint(RobotThree.x, RobotThree.y), 2, CV_RGB(0, 255, 0), 2);
			cvPutText(frame, "Robot#3", cvPoint(RobotThree.x+10, RobotThree.y+10), &font, CV_RGB(0, 255, 0));

			//Draw orientation arrow
			cvLine(frame, cvPoint(RobotThree.phys_x_old/MAPPING_SCALE, RobotThree.phys_y_old/MAPPING_SCALE),cvPoint(RobotThree.x, RobotThree.y), CV_RGB(127, 0, 127));
			//cout << "R3 degrees: " << RobotThree.angle << endl;
			mapAndSendPosition(&RobotThree);
		} else {
			//cout << "Robot#3 Position filtered!" << endl;
		}
	} else {
		RobotThree.onTheField = false;
		//cout << "Multiple positions for Robot#3 - NOT sending position" << endl;
	}

	//Show results
	if(displayControlPanelR1)
		cvShowImage("Contours-Robot#1", colorMaskR1);
	if(displayControlPanelR2)
		cvShowImage("Contours-Robot#2", colorMaskR2);
	if(displayControlPanelR3)
		cvShowImage("Contours-Robot#3", colorMaskR3);
	if(displayVideo)
		cvShowImage("Video", frame);

	//Clean up
	cvReleaseImage(&colorMaskR1);
	cvReleaseImage(&colorMaskR2);
	cvReleaseImage(&colorMaskR3);

	//15 FPS
	//cvWriteFrame(writer, frame); /*Video recording*/
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


void ColorMatcher::setDefaultValues()
{
	NB_ROBOTS = 3;
	
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
	FIELD_EDGE_SIZE = 10; //Just a standard value - GUI adjustable
	POSITION_FILTER_THRESHOLD = 5; //Just a standard value - GUI adjustable
	ENABLE_GAUSSIAN_SMOOTH = 1; //Just a standard value - GUI adjustable
	FORCE_CONVEX_SHAPES = 0; //Just a standard value - GUI adjustable
	BOUND_SIZE = 8; //Min. size of the bounding rect on the contours - standard value - GUI adjustable
	POSITION_TIME = 0; //Used to limit the amount of positions send in order to not overload the  stack
	
	CAM_ORIENTATION = 90; //Orientation of camera - GUI adjustable
	
	//Set for red - these are the values that worked best for us in our last setup.
	//They will be different in any other setup. GUI adjustable
	H_VAL_R1 = 0;
	S_VAL_R1 = 0;
	V_VAL_R1 = 151;
	H_VAL_R1_MAX = 134;
	S_VAL_R1_MAX = 163;
	V_VAL_R1_MAX = 255;
	//Set for blue
	H_VAL_R2 = 160;
	S_VAL_R2 = 0;
	V_VAL_R2 = 0;
	H_VAL_R2_MAX = 255;
	S_VAL_R2_MAX = 255;
	V_VAL_R2_MAX = 147;
	//Set for green
	H_VAL_R3 = 0;
	S_VAL_R3 = 151;
	V_VAL_R3 = 0;
	H_VAL_R3_MAX = 130;
	S_VAL_R3_MAX = 255;
	V_VAL_R3_MAX = 182;
	
	RobotOne.x = -1;
	RobotOne.y = -1;
	RobotTwo.x = -1;
	RobotTwo.y = -1;
	RobotThree.x = -1;
	RobotThree.y = -1;
	RobotOne.robotNumber = 1;
	RobotTwo.robotNumber = 2;
	RobotThree.robotNumber = 3;
	RobotOne.angle = 181;
	RobotTwo.angle = 181;
	RobotThree.angle = 181;
	RobotOne.positions_array_counter = 0;
	RobotTwo.positions_array_counter = 0;	
	RobotThree.positions_array_counter = 0;	
	RobotOne.onTheField = false;
	RobotTwo.onTheField = false;
	RobotThree.onTheField = false;
}

ColorMatcher::ColorMatcher(bool withWebcam, bool video, bool controlPanels)
{
	webcamUsed = withWebcam;
	displayVideo = video;
	displayControlPanelR1 = controlPanels;
	displayControlPanelR2 = controlPanels;
	displayControlPanelR3 = controlPanels;
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
