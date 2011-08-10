#include <cv.h> // includes OpenCV definitions
#include <highgui.h> // includes highGUI definitions
#include <stdio.h>// includes C standard input/output definitions'
#include <stdlib.h>
#include <iostream>
#include "colorMatcher.h" //include common datastructurs used in this server
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "client.hpp"

using namespace std;

int CONNECTION = 0; //Socket connection switch
int CAM_DISTANCE_TO_FIELD = 274; //Physically meassured
const int CAM_WIDTH = 640;
const int CAM_HEIGHT = 480;
const float CAM_FOV_ANGLE = 22.34; //Previously calculated based on cam distance to field and length of cam view
float MAPPING_SCALE = 0;
int FIELD_EDGE_SIZE = 2; //Just a standard value - GUI adjustable
int POSITION_FILTER_THRESHOLD = 6; //Just a standard value - GUI adjustable
int ENABLE_GAUSSIAN_SMOOTH = 1; //Just a standard value - GUI adjustable
int FORCE_CONVEX_SHAPES = 0; //Just a standard value - GUI adjustable
int BOUND_SIZE = 35; //Min. size of the bounding rect on the contours - standard value - GUI adjustable
int position_time = 0; //Used to limit the amount of positions send in order to not overload the  stack

int camOrientation = 90; //Orientation of camera - GUI adjustable

//Set for red - these are the values that worked best for us in our last setup.
//They will be different in any other setup. GUI adjustable
int H_VAL_R1 = 0;
int S_VAL_R1 = 0;
int V_VAL_R1 = 145;
int H_VAL_R1_MAX = 105;
int S_VAL_R1_MAX = 180;
int V_VAL_R1_MAX = 255;
//Set for blue
int H_VAL_R2 = 145;
int S_VAL_R2 = 0;
int V_VAL_R2 = 0;
int H_VAL_R2_MAX = 255;
int S_VAL_R2_MAX = 217;
int V_VAL_R2_MAX = 123;
//Set for green
int H_VAL_R3 = 33;
int S_VAL_R3 = 210;
int V_VAL_R3 = 21;
int H_VAL_R3_MAX = 224;
int S_VAL_R3_MAX = 255;
int V_VAL_R3_MAX = 182;

Client *client;
CvFont font;
	
//A structure for each robot
Robot RobotOne;
Robot RobotTwo;

void startConnections() {
	if(client == NULL) client = new Client(IPHOST, PORTNO);
}
void stopConnection() {
	close(RobotOne.socket);
	close(RobotTwo.socket);
	delete client;
}
void calculateMappingScale(){
	//This math is explained in the report
	float lengthOfFov = (1 / tan( (180-90-CAM_FOV_ANGLE) * M_PI / 180 )) * CAM_DISTANCE_TO_FIELD;
	MAPPING_SCALE = lengthOfFov * 2 / CAM_WIDTH;
}
void mapPosition(Robot* rpos) {
	//Store previous position
	rpos->phys_x_old = rpos->phys_x;
	rpos->phys_y_old = rpos->phys_y;
	//Scale new position
	rpos->phys_x = (int)(MAPPING_SCALE*rpos->x);
	rpos->phys_y = (int)(MAPPING_SCALE*rpos->y);
	//cout << rpos->robotNumber << ": x, y = {" << rpos->phys_x << ", " << rpos->phys_y << "}" << endl;
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

	/////////////////////////////

}
void sendPosition(Robot* robot, Robot* enemy){
	//Warn robot to no drive outside field
	char buffer[256];
	bzero(buffer, 255);
	int pos_x_NewCoord = 10*(robot->phys_x - CAM_WIDTH/2 * MAPPING_SCALE); //convert to mm
	int pos_y_NewCoord = -10*(robot->phys_y - CAM_HEIGHT/2 * MAPPING_SCALE);
	bool edge_warning = robot->phys_x < 0 + FIELD_EDGE_SIZE ||
		robot->phys_x > CAM_WIDTH*MAPPING_SCALE - FIELD_EDGE_SIZE ||
		robot->phys_y < 0 + 10 + FIELD_EDGE_SIZE || /*+10 = physical field not big enough, thus reduce virtual size*/
		robot->phys_y > CAM_HEIGHT*MAPPING_SCALE - FIELD_EDGE_SIZE;
		
    sprintf(buffer, "# %d : { %d , %d }, %d rbt, %d cam, %s ew\n", robot->robotNumber, pos_x_NewCoord, pos_y_NewCoord, robot->angle, camOrientation, (edge_warning)?"true":"false");
	printf(buffer);
	client->SendMessage(buffer);
}

void mapPositions() {
	mapPosition(&RobotOne);
	mapPosition(&RobotTwo);
}
void sendPostionToRobots() {
	if (CONNECTION) {
			sendPosition(&RobotOne, &RobotTwo);
			sendPosition(&RobotTwo, &RobotOne);
	}
}

void setHValR1(int pos) {
	H_VAL_R1 = pos;
}
void setSValR1(int pos) {
	S_VAL_R1 = pos;
}
void setVValR1(int pos) {
	V_VAL_R1 = pos;
}
void setHValR1Max(int pos) {
	H_VAL_R1_MAX = pos;
}
void setSValR1Max(int pos) {
	S_VAL_R1_MAX = pos;
}
void setVValR1Max(int pos) {
	V_VAL_R1_MAX = pos;
}
void setHValR2(int pos) {
	H_VAL_R2 = pos;
}
void setSValR2(int pos) {
	S_VAL_R2 = pos;
}
void setVValR2(int pos) {
	V_VAL_R2 = pos;
}
void setHValR2Max(int pos) {
	H_VAL_R2_MAX = pos;
}
void setSValR2Max(int pos) {
	S_VAL_R2_MAX = pos;
}
void setVValR2Max(int pos) {
	V_VAL_R2_MAX = pos;
}

void setBoundSize(int pos) {
	BOUND_SIZE = pos;
}
void setDistance(int pos) {
	if (CAM_DISTANCE_TO_FIELD != pos) {
		CAM_DISTANCE_TO_FIELD = pos;
		calculateMappingScale();
	}
}
void setPositionFilterThreshold(int pos) {
	POSITION_FILTER_THRESHOLD = pos;
}
void setGaussian(int pos) {
	ENABLE_GAUSSIAN_SMOOTH = pos;
}
void setConvex(int pos) {
	FORCE_CONVEX_SHAPES = pos;
}
void setEdgeSize(int pos) {
	FIELD_EDGE_SIZE = pos;
}
void toggle(int pos) {
	if (pos == 0) {
		stopConnection();
	} else if (pos == 1) {
		startConnections();
	}
	CONNECTION = pos;
}

void analyzeFrame(IplImage* frame)
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

	//Explained in report
	cvInRangeS(frame, cvScalar(H_VAL_R1, S_VAL_R1, V_VAL_R1), cvScalar(H_VAL_R1_MAX, S_VAL_R1_MAX, V_VAL_R1_MAX), colorMaskR1);
	cvInRangeS(frame, cvScalar(H_VAL_R2, S_VAL_R2, V_VAL_R2), cvScalar(H_VAL_R2_MAX, S_VAL_R2_MAX, V_VAL_R2_MAX), colorMaskR2);

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
	//Control variable to determine if we should update robots positions
	bool bothPositionsFiltered = true;

	//Update Robot#1 structure - only if single match was found
	if (numberOfContours >= 1) {
		int newX = abs(pos.x+(pos.width/2));
		int newY = abs(pos.y+(pos.height/2));

		//Apply position filter - filter out "small jumps"
		if (POSITION_FILTER_THRESHOLD <= abs(newX - RobotOne.x) ||
			POSITION_FILTER_THRESHOLD <= abs(newY - RobotOne.y)) {
			bothPositionsFiltered = false;
			RobotOne.x = newX;
			RobotOne.y = newY;

			//Draw robot identifier
			cvCircle(frame, cvPoint(RobotOne.x, RobotOne.y), 2, CV_RGB(255, 0, 0), 2);
			cvPutText(frame, "Robot#1", cvPoint(RobotOne.x+10, RobotOne.y+10), &font, CV_RGB(255, 0, 0));
			
			//Draw orientation arrow
			cvLine(frame, cvPoint(RobotOne.phys_x_old/MAPPING_SCALE, RobotOne.phys_y_old/MAPPING_SCALE),cvPoint(RobotOne.x, RobotOne.y), CV_RGB(0, 0, 255));
			//cout << "R1 degrees: " << RobotOne.angle << endl;

		} else {
			//cout << "Robot#1 Position filtered!" << endl;
		}
	} else {
		//cout << "Multiple positions for Robot#1 - NOT sending position" << endl;
	}

	//Update Robot#2 structure - only if single match was found
	if (numberOfContoursR2 >= 1) {
		int newX = abs(posR2.x+(posR2.width/2));
		int newY = abs(posR2.y+(posR2.height/2));

		//Apply position filter - filter out "small jumps"
		if (POSITION_FILTER_THRESHOLD <= abs(newX - RobotTwo.x) ||
				POSITION_FILTER_THRESHOLD <= abs(newY - RobotTwo.y)) {
			bothPositionsFiltered = false;
			RobotTwo.x = newX;
			RobotTwo.y = newY;

			//Draw robot identifier
			cvCircle(frame, cvPoint(RobotTwo.x, RobotTwo.y), 2, CV_RGB(0, 0, 255), 2);
			cvPutText(frame, "Robot#2", cvPoint(RobotTwo.x+10, RobotTwo.y+10), &font, CV_RGB(0, 0, 255));

			//Draw orientation arrow
			cvLine(frame, cvPoint(RobotTwo.phys_x_old/MAPPING_SCALE, RobotTwo.phys_y_old/MAPPING_SCALE),cvPoint(RobotTwo.x, RobotTwo.y), CV_RGB(255, 0, 0));
			//cout << "R2 degrees: " << RobotTwo.angle << endl;

		} else {
			//cout << "Robot#2 Position filtered!" << endl;
		}
	} else {
		//cout << "Multiple positions for Robot#2 - NOT sending position" << endl;
	}

	if(!bothPositionsFiltered) {
		//Do mapping
		mapPositions();

		//Send pos to robots
		sendPostionToRobots();
	}

	//Show results
	//cvShowImage("Contours-Robot#1", colorMaskR1);
	//cvShowImage("Contours-Robot#2", colorMaskR2);
	cvShowImage("Video", frame);

	//Clean up
	cvReleaseImage(&colorMaskR1);
	cvReleaseImage(&colorMaskR2);

	//15 FPS
	//cvWriteFrame(writer, frame); /*Video recording*/
	cvWaitKey(1000/15);	
}

int main( int argc, char** argv ) {
	
	RobotOne.robotNumber = 1;
	RobotTwo.robotNumber = 2;
	RobotOne.angle = 181;
	RobotTwo.angle = 181;
	RobotOne.positions_array_counter = 0;
	RobotTwo.positions_array_counter = 0;

	//Calculate the current mapping scale
	calculateMappingScale();

	//Initialize the  web cam
	CvCapture* capture = cvCreateCameraCapture(0);
	if (capture == 0) {
		cout << "failed to open video src\n";
		return 0;
	}

	//Prepare image structures
	IplImage *frame = cvQueryFrame(capture);

	//////////////////// VIDEO RECORDING ////////////////
//	CvSize size = cvSize(640, 480);
//	double fps = 30.0;
//	CvVideoWriter* writer = cvCreateVideoWriter(
//			"/home/mark/Lego-color-track-marker-only.avi",
//			CV_FOURCC('D', 'I', 'V', 'X'),
//			fps,
//			size
//	);
	/////////////////////////////////////////////////////

	//////////////////// GUI SETUP //////////////////////
	//Create windows
	cvNamedWindow("Settings");
	cvNamedWindow("Settings-Robot#1");
	cvNamedWindow("Settings-Robot#2");
	cvNamedWindow("Video", CV_WINDOW_NORMAL);
	//cvNamedWindow("Contours-Robot#1");
	//cvNamedWindow("Contours-Robot#2");

	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 2);

	//Create trackbars for settings
	cvCreateTrackbar("Field Edge size", "Settings", &FIELD_EDGE_SIZE, 100, setEdgeSize);
	cvCreateTrackbar("Cam distance", "Settings", &CAM_DISTANCE_TO_FIELD, 500, setDistance);
	cvCreateTrackbar("Position filter threshold","Settings",&POSITION_FILTER_THRESHOLD,100,setPositionFilterThreshold);
	cvCreateTrackbar("Bound min size","Settings",&BOUND_SIZE,200,setBoundSize);
	cvCreateTrackbar("Smoothing (Gaussian)","Settings",&ENABLE_GAUSSIAN_SMOOTH,1,setGaussian);
	cvCreateTrackbar("Require convex shapes","Settings",&FORCE_CONVEX_SHAPES,1,setConvex);
	cvCreateTrackbar("Socket I/O","Settings",0,1,toggle);
	cvCreateTrackbar("Robot#1: B Val","Settings-Robot#1",&H_VAL_R1,255,setHValR1);
	cvCreateTrackbar("Robot#1: G Val","Settings-Robot#1",&S_VAL_R1,255,setSValR1);
	cvCreateTrackbar("Robot#1: R Val","Settings-Robot#1",&V_VAL_R1,255,setVValR1);
	cvCreateTrackbar("Robot#1: B Val Max","Settings-Robot#1",&H_VAL_R1_MAX,255,setHValR1Max);
	cvCreateTrackbar("Robot#1: G Val Max","Settings-Robot#1",&S_VAL_R1_MAX,255,setSValR1Max);
	cvCreateTrackbar("Robot#1: R Val Max","Settings-Robot#1",&V_VAL_R1_MAX,255,setVValR1Max);
	cvCreateTrackbar("Robot#2: B Val","Settings-Robot#2",&H_VAL_R2,255,setHValR2);
	cvCreateTrackbar("Robot#2: G Val","Settings-Robot#2",&S_VAL_R2,255,setSValR2);
	cvCreateTrackbar("Robot#2: R Val","Settings-Robot#2",&V_VAL_R2,255,setVValR2);
	cvCreateTrackbar("Robot#2: B Val Max","Settings-Robot#2",&H_VAL_R2_MAX,255,setHValR2Max);
	cvCreateTrackbar("Robot#2: G Val Max","Settings-Robot#2",&S_VAL_R2_MAX,255,setSValR2Max);
	cvCreateTrackbar("Robot#2: R Val Max","Settings-Robot#2",&V_VAL_R2_MAX,255,setVValR2Max);
	////////////////////////////////////////////////////


	//////////////////// MATCHING //////////////////////
	while ((frame = cvQueryFrame(capture)) != NULL) {
		analyzeFrame(frame);
	} //main loop
	////////////////////////////////////////////////////


	//Clean up
	cvReleaseCapture(&capture);
	//cvReleaseVideoWriter(&writer); /*Video recording*/
	cvDestroyAllWindows();
	return 0;
}
