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

const double PI = 3.141592;
int CONNECTION = 0; //Socket connection switch
int CAM_DISTANCE_TO_FIELD = 274; //Physically meassured
const int CAM_WIDTH = 640;
const int CAM_HEIGHT = 480;
const float CAM_FOV_ANGLE = 22.34; //Previously calculated based on cam distance to field and length of cam view
float MAPPING_SCALE = 0;
int FIELD_EDGE_SIZE = 5; //Just a standard value - GUI adjustable
int POSITION_FILTER_THRESHOLD = 5; //Just a standard value - GUI adjustable
int ENABLE_GAUSSIAN_SMOOTH = 1; //Just a standard value - GUI adjustable
int FORCE_CONVEX_SHAPES = 0; //Just a standard value - GUI adjustable
int BOUND_SIZE = 20; //Min. size of the bounding rect on the contours - standard value - GUI adjustable
int position_time = 0; //Used to limit the amount of positions send in order to not overload the  stack
int positions_x[10]; //Holds position history used in orientation calculations
int positions_y[10];
int positions_array_counter = 0;

//Set for red - these are the values that worked best for us in our last setup.
//They will be different in any other setup. GUI adjustable
int H_VAL_R1 = 0;
int S_VAL_R1 = 0;
int V_VAL_R1 = 145;
int H_VAL_R1_MAX = 105;
int S_VAL_R1_MAX = 180;
int V_VAL_R1_MAX = 255;
//Set for blue
int H_VAL_R2 = 149;
int S_VAL_R2 = 240;
int V_VAL_R2 = 125;
int H_VAL_R2_MAX = 255;
int S_VAL_R2_MAX = 255;
int V_VAL_R2_MAX = 200;

Client *client;

//A structure for each robot
Robot RobotOne;
Robot RobotTwo;

void startConnections() {
	//char RobotOneaddress[18] = "00:16:53:04:32:11"; //Red target
	init_connection(&RobotOne);
	//char RobotTwoaddress[18] = "00:16:53:07:1E:15"; //Green Target
	//init_connection(&RobotTwo);

}
void stopConnection() {
	close(RobotOne.socket);
	close(RobotTwo.socket);
	delete client;
}
void calculateMappingScale(){
	//This math is explained in the report
	float lengthOfFov = (1 / tan( (180-90-CAM_FOV_ANGLE) * PI / 180 )) * CAM_DISTANCE_TO_FIELD;
	MAPPING_SCALE = lengthOfFov * 2 / CAM_WIDTH;
}
void mapPosition(Robot* rpos) {
	//Store previous position
	rpos->phys_x_old = rpos->phys_x;
	rpos->phys_y_old = rpos->phys_y;
	//Scale new position
	rpos->phys_x = (int)(MAPPING_SCALE*rpos->x);
	rpos->phys_y = (int)(MAPPING_SCALE*rpos->y);

	//In our test we only tested orientation on one robot.
	if (rpos->robotNumber == 2) {
		//Fill in the position history
		positions_x[positions_array_counter % 10] = rpos->phys_x;
		positions_y[positions_array_counter % 10] = rpos->phys_y;
		positions_array_counter++;

		////Calculate orientation////
		int tmp_phys_x_old=0;
		int tmp_phys_y_old=0;
		int tmp_phys_x_new=0;
		int tmp_phys_y_new=0;
		for (int i = 0; i <= 4; i++) {
			cout << i << endl;
			tmp_phys_x_old += positions_x[i];
			tmp_phys_y_old += positions_y[i];
		}
		for (int i = 5; i <= 9; i++) {
			tmp_phys_x_new += positions_x[i];
			tmp_phys_y_new += positions_y[i];
		}

		//First find vector old->new
		float vec_x = (float)tmp_phys_x_new/5 - tmp_phys_x_old/5;
		float vec_y = (float)tmp_phys_y_new/5 - tmp_phys_y_old/5;

		float xaxis_x = vec_x;
		float xaxis_y = 0;

		//Find angle between vector and x-axis (orientation)
		float angle = get_angle_between_in_radians(vec_x, vec_y, xaxis_x, xaxis_y);

		//Convert to degrees
		angle = angle * 180 / M_PI;

		//Update robot structure
		rpos->angle = (int) angle;
	}
	/////////////////////////////

}
void sendPosition(Robot* robot, Robot* enemy){
	//Warn robot to no drive outside field
	char buffer[256];
	bzero(buffer, 255);

	if (robot->phys_x < 0 + FIELD_EDGE_SIZE ||
		robot->phys_x > CAM_WIDTH*MAPPING_SCALE - FIELD_EDGE_SIZE ||
		robot->phys_y < 0 + 10 + FIELD_EDGE_SIZE || /*+10 = physical field not big enough, thus reduce virtual size*/
		robot->phys_y > CAM_HEIGHT*MAPPING_SCALE - FIELD_EDGE_SIZE)
	{
		//const char pos[sizeof(int)*4] = {-1, -1, -1, robot->angle};
		sprintf(buffer, "#%d says: I'm lost, with a %d angle\n", robot->robotNumber, robot->angle);
		printf(buffer);
		if(write(RobotOne.socket, buffer, strlen(buffer))<0)
			perror("ERROR writing to socket");
		cout << robot->robotNumber << ": Edge warning sent!" << endl;
	} else  {
		//char msg[sizeof(int)*4] = {robot->phys_x, robot->phys_y, robot->robotNumber, robot->angle};
		sprintf(buffer, "#%d says: I'm at {%d, %d}, with a %d angle\n", robot->robotNumber, robot->phys_x, robot->phys_y, robot->angle);
		printf(buffer);
		if(write(RobotOne.socket, buffer, strlen(buffer))<0)
			perror("ERROR writing to socket");
		cout << robot->robotNumber << ": Sent position (" << robot->phys_x << ", " << robot->phys_y << ")" << endl;
	}
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
int main( int argc, char** argv ) {
	if(client == NULL) client = new Client(argc, argv);
	RobotOne.robotNumber = 1;
	RobotTwo.robotNumber = 2;
	RobotOne.angle = 0;
	RobotTwo.angle = 0;

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
	cvNamedWindow("Settings-RedTarget");
	cvNamedWindow("Settings-GreenTarget");
	cvNamedWindow("Video");
	cvNamedWindow("Contours-RedTarget");
	cvNamedWindow("Contours-GreenTarget");

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 2);

	//Create trackbars for settings
	cvCreateTrackbar("Field Edge size", "Settings", &FIELD_EDGE_SIZE, 100, setEdgeSize);
	cvCreateTrackbar("Cam distance", "Settings", &CAM_DISTANCE_TO_FIELD, 500, setDistance);
	cvCreateTrackbar("Position filter threshold","Settings",&POSITION_FILTER_THRESHOLD,100,setPositionFilterThreshold);
	cvCreateTrackbar("Bound min size","Settings",&BOUND_SIZE,200,setBoundSize);
	cvCreateTrackbar("Smoothing (Gaussian)","Settings",&ENABLE_GAUSSIAN_SMOOTH,1,setGaussian);
	cvCreateTrackbar("Require convex shapes","Settings",&FORCE_CONVEX_SHAPES,1,setConvex);
	cvCreateTrackbar("Socket I/O","Settings",0,1,toggle);
	cvCreateTrackbar("RedTarget: B Val","Settings-RedTarget",&H_VAL_R1,255,setHValR1);
	cvCreateTrackbar("RedTarget: G Val","Settings-RedTarget",&S_VAL_R1,255,setSValR1);
	cvCreateTrackbar("RedTarget: R Val","Settings-RedTarget",&V_VAL_R1,255,setVValR1);
	cvCreateTrackbar("RedTarget: B Val Max","Settings-RedTarget",&H_VAL_R1_MAX,255,setHValR1Max);
	cvCreateTrackbar("RedTarget: G Val Max","Settings-RedTarget",&S_VAL_R1_MAX,255,setSValR1Max);
	cvCreateTrackbar("RedTarget: R Val Max","Settings-RedTarget",&V_VAL_R1_MAX,255,setVValR1Max);
	cvCreateTrackbar("GreenTarget: B Val","Settings-GreenTarget",&H_VAL_R2,255,setHValR2);
	cvCreateTrackbar("GreenTarget: G Val","Settings-GreenTarget",&S_VAL_R2,255,setSValR2);
	cvCreateTrackbar("GreenTarget: R Val","Settings-GreenTarget",&V_VAL_R2,255,setVValR2);
	cvCreateTrackbar("GreenTarget: B Val Max","Settings-GreenTarget",&H_VAL_R2_MAX,255,setHValR2Max);
	cvCreateTrackbar("GreenTarget: G Val Max","Settings-GreenTarget",&S_VAL_R2_MAX,255,setSValR2Max);
	cvCreateTrackbar("GreenTarget: R Val Max","Settings-GreenTarget",&V_VAL_R2_MAX,255,setVValR2Max);
	////////////////////////////////////////////////////


	//////////////////// MATCHING //////////////////////
	while ((frame = cvQueryFrame(capture)) != NULL) {
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

		//Find countours for RedTarget
		int numberOfContours = cvFindContours(colorMaskR1, storage, &first_contour);
		CvRect bound;
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

					//save this for future use
					pos = cvBoundingRect(c, 0);
				} else {
					//If we filter something out
					numberOfContours--;
				}
			}
		}
		//Reset variables
		cvClearMemStorage(storage);
		first_contour = NULL;

		//Find contours for GreenTarget
		int numberOfContoursR2 = cvFindContours(colorMaskR2, storage, &first_contour);
		CvRect boundR2;
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
							CV_RGB(0, 255, 0)
					);

					//save this for future use
					posR2 = cvBoundingRect(c, 0);
				} else {
					numberOfContoursR2--;
				}
			}
		}
		//Control variable to determine if we should update robots positions
		bool bothPositionsFiltered = true;

		//Update RedTarget structure - only if single match was found
		if (numberOfContours == 1) {
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
				cvPutText(frame, "RedTarget", cvPoint(RobotOne.x+10, RobotOne.y+10), &font, CV_RGB(255, 0, 0));

			} else {
				cout << "RedTarget Position filtered!" << endl;
			}
		} else {
			//cout << "Multiple positions for RedTarget - NOT sending position" << endl;
		}

		//Update GreenTarget structure - only if single match was found
		if (numberOfContoursR2 == 1) {
			int newX = abs(posR2.x+(posR2.width/2));
			int newY = abs(posR2.y+(posR2.height/2));

			//Apply position filter - filter out "small jumps"
			if (POSITION_FILTER_THRESHOLD <= abs(newX - RobotTwo.x) ||
					POSITION_FILTER_THRESHOLD <= abs(newY - RobotTwo.y)) {
				bothPositionsFiltered = false;
				RobotTwo.x = newX;
				RobotTwo.y = newY;

				//Draw robot identifier
				cvCircle(frame, cvPoint(RobotTwo.x, RobotTwo.y), 2, CV_RGB(0, 255, 0), 2);
				cvPutText(frame, "GreenTarget", cvPoint(RobotTwo.x+10, RobotTwo.y+10), &font, CV_RGB(0, 255, 0));

				//Draw orientation arrow
				cvLine(frame, cvPoint(RobotTwo.phys_x_old/MAPPING_SCALE, RobotTwo.phys_y_old/MAPPING_SCALE),cvPoint(RobotTwo.x, RobotTwo.y), CV_RGB(255, 0, 0));
				cout << "R2 degrees: " << RobotTwo.angle << endl;

			} else {
				cout << "GreenTarget Position filtered!" << endl;
			}
		} else {
			//cout << "Multiple positions for GreenTarget - NOT sending position" << endl;
		}

		if(!bothPositionsFiltered) {
			//Do mapping
			mapPositions();

			//Send pos to robots
			sendPostionToRobots();
		}

		//Show results
		cvShowImage("Contours-RedTarget", colorMaskR1);
		cvShowImage("Contours-GreenTarget", colorMaskR2);
		cvShowImage("Video", frame);

		//Clean up
		cvReleaseImage(&colorMaskR1);
		cvReleaseImage(&colorMaskR2);

		//30 FPS
		//cvWriteFrame(writer, frame); /*Video recording*/
		cvWaitKey(1000/30);
	} //main loop
	////////////////////////////////////////////////////


	//Clean up
	cvReleaseCapture(&capture);
	//cvReleaseVideoWriter(&writer); /*Video recording*/
	cvDestroyAllWindows();
	return 0;
}
