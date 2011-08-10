/******************************
 * Flight Planner Skeleton
 * Cooper Bills (csb88@cornell.edu)
 * Cornell University
 * 1/4/11
 ******************************/
#define cvAlgWindow "AlgorithmView"
#define cvBotAlgWindow "BottomAlgorithmView"

extern "C" {
#include <VP_Os/vp_os_print.h>
#include <ardrone_api.h>
#include <ardrone_tool/ardrone_tool_configuration.h>
#include <ardrone_tool/ardrone_tool.h>
}

#include <stdio.h>
#include <pthread.h>
#include "planner.hpp"
#include "Navdata/NavDataContainer.hpp"
#include "Tools/coopertools.hpp"
#include <sys/time.h>
#include <fstream>
#include <time.h>
#include <opencv/cv.h>

#include <cmath>

using namespace std;

//Globablly Accessable Variables:
extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
extern int32_t ALTITUDE; //Altitude of drone in mm (defined in Navdata/navdata.c)
extern float32_t PSI; //Current Direction of Drone (-180deg to 180deg) (defined in Navdata/navdata.c)
extern NavDataContainer globalNavData; //Navdata (defined in Navdata/NavDatacontainer.cpp)
extern int newFrameAvailable; //if navdata indicates a new frame (defined in Navdata/navdata.c)
extern const int TICKS_PER_SEC = 247500;

extern "C" {	
extern void ardrone_at_set_detect_typeC(CAD_TYPE detect_type)
{
    char msg[64];
    sprintf( &msg[0], "%d", detect_type);
    ardrone_at_cad(detect_type, 200);
    //ardrone_at_set_toy_configuration( "detect:detect_type", msg );
}
}

void setDirection(bool angleIsInDegree, int32_t angle, int32_t speed, int32_t *dpitch, int32_t *droll)
{
	if(angleIsInDegree)
	{
		*dpitch = - (int32_t) (sin(angle * M_PI / 180) * (double) speed);
		*droll = (int32_t) (cos(angle * M_PI / 180) * (double) speed);
	}	
	else
	{
		*dpitch = - (int32_t) (sin(angle) * (double) speed);
		*droll = (int32_t) (cos(angle) * (double) speed);
	}
}
	
void convertToPolarCoordinates(int xCoordinate, int yCoordinate, double *radius, double *theta)
{
	*radius = sqrt(xCoordinate * xCoordinate + yCoordinate * yCoordinate);	
	*theta = atan2(yCoordinate, xCoordinate);
}

//Thread to run separately:
void *Planner_Thread(void *params)
{
    Planner *self = (Planner *)params;
    self->reset = false;
    self->angleInDegree = 0;
    self->speed = 0;
    while(frontImgStream == NULL) {usleep(100000);} //wait for initialization
    cvStartWindowThread();
    cvNamedWindow(cvAlgWindow, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(cvBotAlgWindow, CV_WINDOW_AUTOSIZE);

    //******** Add Your Initialization Below this Line *********

    ardrone_at_set_detect_typeC(CAD_TYPE_COCARDE);
    const int XMIDDLE = 88, YMIDDLE = 77;
    int xval=XMIDDLE, yval=YMIDDLE;
    double radius = 0, theta = 0;
    int nb_detected=0;
    //const int SPEED_X = 3500, SPEED_Y = 3500;
    //const int XMAX = 1000;
    //const int MARGIN = 50;
    //const int XC_INF = XMAX/2 - MARGIN, XC_SUP = XMAX/2 + MARGIN, YC_INF = YMAX/2 - MARGIN, YC_SUP = YMAX/2 + MARGIN;
    //const int MIDDLE = XMAX/2;
    clock_t init = clock(), final;
    double time_lost = 0;

    // Altitude PID
    const double alt_Kp = 15, alt_Ki = 2, alt_Kd = 0, ALT_OFFSET = 1800, ALTp = 0;
    int alt_error = 0, alt_integral = 0, alt_lasterror = 0, alt_derivative = 0;
    double alt_move;

    // Tracking PID
    self->kp = 21, self->ki = 0.5, self->kd = 80;
    int xerror=0, yerror=0, xintegral = 0, yintegral = 0, xlasterror = 0,
            ylasterror = 0, xderivative = 0, yderivative = 0;
    double xmove = 0, ymove = 0;
    double dampen = 2/3;

    self->dpitch = 0; // No forward motion
    self->droll = 0; // No side-to-side motion
    self->dyaw = 0; // No turning motion
    self->dgaz = ALTITUDE - 1600; // Adjust altitude by difference from goal.
    self->hover = 0;

    // Saving PID data for latter plotting
    FILE * moveFile;
    clock_t dataClk = clock();
    double dataClkPrint = 0;
    moveFile = fopen ("moveData.txt","w");
    fprintf(moveFile, "time xmove ymove altitude xerror xintegral yerror yintegral\n");

    // Camera analysis
    IplImage* testFrame = 0;
    testFrame = cvCreateImage(cvSize(bottomImgStream->width, bottomImgStream->height), IPL_DEPTH_8U, bottomImgStream->nChannels);
    std::vector<cv::Vec3f> circles;
	cv::Mat bottomMatDraw, bottomHSV;
	std::vector<cv::Mat> bottomV;
	std::vector<cv::Vec3f>::const_iterator itc;
	int detectionEfficiency = 0;

    //******** Add Your Initialization Above this Line *********

    while(self->running) //Continue to loop until system is closed
    {
    	if(self->reset) // activate resetting of parameters (especially PID errors)
    	{
    		xerror=0;
    		yerror=0;
    		xintegral = 0;
    		yintegral = 0; 
    		xlasterror = 0;
            ylasterror = 0;
            xderivative = 0; 
            yderivative = 0;
            self->reset = false;
            printf("Parameters reset !\n");
    	}
        if(self->enabled)
        {    	
        	if(!newFrameAvailable)
            {
                usleep(100000);
                if(!newFrameAvailable) printf("\n    Uh-Oh!  No New Frames\n");
                //If you would like to handle this situation, put it here.
                continue; //planner will only run when new data is available
            }
            newFrameAvailable = 0; //reset

            //******** Edit Below this Line **********

            /* When algorithms are enabled, This loop will loop indefinitely
              (until algorithms are disabled and manual control is
              regained).  The goal of this loop is to take the input from
              the drone (images, sensors, etc.), process it, then output
              control values.  There are 4 axis of control on our drones:
              pitch (forward/backward), roll (left/right), yaw (turning),
              and gaz (up/down).  An external thread reads dXXX_final and
              sends the command to the drone for us. */

            // Commands are values from -25000 to 25000:
            // Pitch - Forward is Negative.
            // Roll - Right is Positive.
            // Yaw - Right is Positive.
            // Gaz - Up is Negative.

            // For example, we want the drone to remain in place, but hover ~1.5m:
            // A simple proportional controller:
            //self->dpitch = 0; // No forward motion
            //self->droll = 0; // No side-to-side motion
            self->dyaw = 0; // No turning motion
            self->hover = 0;

            // Altitude PID
            alt_error = ALTITUDE - ALT_OFFSET;
            alt_integral = dampen * alt_integral + alt_error;
            alt_derivative = alt_error - alt_lasterror;
            alt_move = alt_Kp * alt_error + alt_Ki * alt_integral + alt_Kd * alt_derivative;
            if(abs(alt_move) > 25000) alt_move = alt_move/abs(alt_move) * 25000;
            self->dgaz = ALTp + alt_move;
            alt_lasterror = alt_error;
            
            /**
             * Image Analysis
             */
            circles.clear();
             // check for errors:
            if(!bottomImgStream)
                continue;
            cv::Mat bottomMat(bottomImgStream, false); //convert Ipl image to a matrix bottomMat
            cv::GaussianBlur(bottomMat, bottomMat, cv::Size(5,5),1.5); 
            //cv::cvtColor(bottomMat, bottomHSV, CV_BGR2HSV); // Convert to HSV
            //cv::split(bottomHSV, bottomV); // Split HSV into a vector of three matrixes
            //bottomMatDraw = bottomV[2]; // Select the last matrix
            cv::cvtColor(bottomMat, bottomMatDraw, CV_BGR2GRAY); // convert 3-channel image to 1-channel gray image
			//cv::equalizeHist(bottomMatDraw, bottomMatDraw); // Equalize the histogram on this matrix to improve contrast
            cv::HoughCircles(bottomMatDraw, // frame to be analysed
            				circles, 		// Vector returned containing detected circles parameters
            				CV_HOUGH_GRADIENT, // Two-pass circle detection method
            				2, 				// accumulator resolution  (image size / 2)
            				50, 			// min. distance between two circles
            				200, 			// Canny high threshold (low thresh. = high / 2)
            				75, 			// Minimum number of votes to pass to consider a new candidate
            				5, 75); 		// Min and max radius for circles to detect
            itc = circles.begin();
            while(itc!=circles.end()){ // Draw a shape on the image to show pattern recognition
            	//printf("Tag found at coordinate {%d, %d}\n", (int) (*itc)[0], (int) (*itc)[1]);
            	cv::circle(bottomMatDraw,
            		cv::Point{(*itc)[0], (*itc)[1]},
            		(*itc)[2],
            		cv::Scalar(1),
            		2);
				++itc;
            }
            cv::imshow(cvBotAlgWindow, bottomMatDraw);            
            
            /**
             * PID controler
             */
            //printf("Navdata - altitude: %d \n", globalNavData.altitude);
            //printf("Navdata - nb_detected: %d \n", globalNavData.vd_nb_detected);
            //printf("Navdata - xc[0], yc[0]: (%d, %d) \n", globalNavData.vd_xc[0],globalNavData.vd_yc[0]);

            nb_detected = globalNavData.vd_nb_detected;
            
            if(nb_detected < circles.size()){
            	//printf("CUSTOM ALGO DETECTED 1 MORE   ++++++++++++++++++++++\n");
            	detectionEfficiency++;
            }
            if(nb_detected > circles.size()){
            	printf("EMBEDDED ALGO DETECTED 1 MORE ----------------------\n");
            	detectionEfficiency--;
            }     	

            if(circles.size() >= 1){
                //printf("Navdata - nb_detected: %d \n", nb_detected);
                //printf("Navdata - xc[0], yc[0]: (%d, %d) \n", globalNavData.vd_xc[0],globalNavData.vd_yc[0]);
                init = clock();
                itc = circles.begin();
                xval=(*itc)[0];
                yval=(*itc)[1];
                
                // Convert from cartesian coordinates to polar coordinates
                // with change of origin (translated to the center)
                // and inversion of y-axis
                convertToPolarCoordinates(xval - XMIDDLE, -(yval - YMIDDLE), &radius, &theta);
                printf("Polar coordinates of tag: r = %-.1f, theta = %-.1f (degrees)\n", radius, theta * 180 / M_PI);
                
                //printf("Tag found at coordinate {%d, %d}\n", (int) (*itc)[0], (int) (*itc)[1]);

                // Proportionnal integrator controller

                // Horizontal PID
                /*
                if(xerror * (xval - MIDDLE) < 0 ){
                   xerror = xval - MIDDLE;
                   xintegral = 0;
                } else {
                   xerror = xval - MIDDLE;
                   xintegral = dampen*xintegral + xerror;
                }*/
                xerror = xval - XMIDDLE;
                xintegral = dampen * xintegral + xerror;
                xderivative = xerror - xlasterror;
                xmove = self->kp * xerror + self->ki * xintegral + self->kd * xderivative;
                if(abs(xmove) > 25000) xmove = xmove/abs(xmove) * 25000;
                //self->droll = 0 + xmove;
                xlasterror = xerror;

                // Vertical PID
                /*
                if(yerror * (yval - MIDDLE) < 0){
                   yerror = yval - MIDDLE;
                   yintegral = 0;
                } else {
                   yerror = yval - MIDDLE;
                   yintegral = dampen*yintegral + yerror;
                }*/
                yerror = yval - YMIDDLE;
                yintegral = dampen * yintegral + yerror;
                yderivative = yerror - ylasterror;
                ymove = self->kp * yerror + self->ki * yintegral + self->kd * yderivative;
                if(abs(ymove) > 25000) ymove = ymove/abs(ymove) * 25000;
                //self->dpitch = 0 + ymove;
                ylasterror = yerror;

		
                //ardrone_at_set_led_animation(BLINK_ORANGE, 10, (float) 0.01);
            } else {
                final = clock() - init;
                time_lost = (double)final / ((double)TICKS_PER_SEC);
                //printf("Target lost for %f seconds\n", time_lost);

                if(time_lost > 0 && time_lost < 1){
                    //printf("*************Looking for target**********************\n");
                    xval=globalNavData.vd_xc[0];
                    yval=globalNavData.vd_yc[0];
                    /*
                     // Proportionnal controller
                     xerror = xval - MIDDLE;
                     xmove = self->kp * xerror;
                     self->droll = 0 + xmove;

                     yerror = yval - MIDDLE;
                     ymove = self->kp * yerror;
                     self->dpitch = 0 + ymove;
                     */
                    self->hover = 1;
                } else {
                    //printf("TARGET LOST\n");
                    self->hover = 1;
                }
            }
            //self->droll = 0;
            //self->dpitch = 0;
            //self->hover = 1;


            //self->droll > 0 ? printf("RIGHT\n") : printf("LEFT\n");
            //self->dpitch > 0 ? printf("BACKWARD\n") : printf("FORWARD\n");


            //printf("hover  = %d\n", self->hover);

            final = clock() - dataClk;
            dataClkPrint = (double)final/ ((double)TICKS_PER_SEC);
            fprintf(moveFile, "%-.2f %d %d %d %d %d %d %d\n",dataClkPrint,(int) xmove, (int) ymove, (int) alt_move, (int) self->kp * xerror, (int) self->ki * xintegral, (int) self->kp * yerror, (int) self->ki * yintegral);

            setDirection(false, self->angleInDegree, self->speed, &self->dpitch, &self->droll);
            //printf("droll %d, dpitch %d\n", self->droll, self->dpitch);
            self->hover = 0;

            //******** Edit Above this Line **********

            //Apply commands all at once
            self->dgaz_final = self->dgaz;
            self->dyaw_final = self->dyaw;
            self->droll_final = self->droll;
            self->dpitch_final = self->dpitch;
            self->hover_final = self->hover;
        } //end if enabled
        else
        {

            pthread_yield();
        }
        usleep(50000);
    }

    //De-initialization
    fclose(moveFile);
    cvDestroyWindow(cvAlgWindow);
    cvDestroyWindow(cvBotAlgWindow);
    printf("Detection efficency = %d\n", detectionEfficiency);
    printf("PID parameters : {Kp = %-.1f ; Ki = %-.1f ; Kd = %-.1f}\n", self->kp, self->ki, self->kd);
    printf("thread returned\n");

    return 0;
}


Planner::Planner()
{
    dpitch_final = 0;
    dyaw_final = 0;
    droll_final = 0;
    dgaz_final = 0;
    dpitch = 0;
    dyaw = 0;
    droll = 0;
    dgaz = 0;
    enabled = false;
    running = true;

    //Create the Planner Thread
    threadid = pthread_create( &plannerthread, NULL, Planner_Thread, (void*) this);
}

Planner::~Planner()
{
    //destructor
    running = false;
}

