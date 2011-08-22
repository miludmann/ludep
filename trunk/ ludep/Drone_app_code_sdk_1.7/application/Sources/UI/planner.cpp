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
#include "ColorMatching/colorMatcher.hpp"
#include "Navdata/NavDataContainer.hpp"
#include "Tools/coopertools.hpp"
#include <sys/time.h>
#include <fstream>
#include <time.h>
#include <opencv/cv.h>
#include <deque>
#include <iostream>
#include <iomanip>

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

/*
 * Translate the coordinate system by moving the origin (0, 0) from the top right to the center of the image
 * And iversion of the y-axis
 * @note: keep consistency in the units (everything in mm or in pixels...)
 */
void cornerToCenterCoordinates(int *x, int *y, int xmiddle, int ymiddle)
{
	*x = *x - xmiddle;
	*y = -(*y - ymiddle);
}

/*
 * @param value: value (with no real unit or pixels) to convert
 * @param fov: Camera FOV (Field Of View) in degrees
 * @param altitude: altitude of the drone when the value is got
 * @param middle: half of the field of view measured in the same 'unit' as the original value
 * @return: the original value converted in millimeters
 */
int convertToMM(int value, int fov, int altitude, int middle)
{
	 return (int) ((double) (value * altitude) * tan(fov / 2 * M_PI / 180) / middle);
}

/*
 * @param: tiltAngle in degrees
 */
int convertToRealCoordinate(int coordinateRead, int altitude, double tiltAngle)
{
	double alpha = atan2(altitude, abs(coordinateRead));
	tiltAngle *= M_PI / 180; // convert in radian
	if((tiltAngle > 0 && coordinateRead < 0) || (tiltAngle < 0 && coordinateRead > 0))
		return (int) ((double) coordinateRead * sin(alpha) / sin(alpha - abs(tiltAngle)) - (double) altitude * sin(tiltAngle));
	else
		return (int) ((double) coordinateRead * sin(alpha) / cos(abs(tiltAngle)) - (double) altitude * sin(tiltAngle));
}

int convertToRealCoordinate_old(int coordinateRead, int altitude, double tiltAngle)
{
	return (int) ((double) coordinateRead / cos(tiltAngle * M_PI / 180) - (double) altitude * tan(tiltAngle* M_PI / 180));
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
    //cvNamedWindow(cvAlgWindow, CV_WINDOW_AUTOSIZE); +++++++++++++++++
    //cvNamedWindow(cvBotAlgWindow, CV_WINDOW_AUTOSIZE); +++++++++++++++++

    //******** Add Your Initialization Below this Line *********
    
	// Embedded detection tests    
	int FOV = 64; // Camera field of view (depends on the camera used)
    int XMIDDLE = 88, YMIDDLE = 77; // Half of the camera resolution on each axis
	
    int xval=XMIDDLE, yval=YMIDDLE;
    int xvalPrevious=XMIDDLE, yvalPrevious=YMIDDLE;
    double radius = 0, theta = 0;
    double euler_theta = 0.0, euler_phi = 0.0;
    double euler_thetaPrevious = 0.0, euler_phiPrevious = 0.0;
    deque <double> listPhi;
   
    clock_t init = clock(), final;
    double time_lost = 0;

    // Altitude PID
    const double alt_Kp = 15, alt_Ki = 2, alt_Kd = 0, ALT_OFFSET = 1800, ALTp = 0;
    int alt_error = 0, alt_integral = 0, alt_lasterror = 0, alt_derivative = 0;
    double altitudeMove;

    // Tracking PID
    self->kp = 3, self->ki = 0, self->kd = 10;
    
    int radiusError = 0, radiusIntegral = 0, radiusLastError = 0, radiusDerivative = 0;
    double radiusMove = 0;
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
    //fprintf(moveFile, "time euler_theta euler_phi altitudeMove radiusMove radiusError radiusIntegral radiusDerivative, radius, theta\n");
    fprintf(moveFile, "time altitude euler_theta euler_phi xval_tmp yval_tmp xval yval xnav ynav kp ki kd\n");
    
    // Camera analysis
    IplImage* testFrame = 0;
    testFrame = cvCreateImage(cvSize(bottomImgStream->width, bottomImgStream->height), IPL_DEPTH_8U, bottomImgStream->nChannels);
    std::vector<cv::Vec3f> circles;
	cv::Mat bottomMatDraw, bottomHSV;
	std::vector<cv::Mat> bottomV;
	std::vector<cv::Vec3f>::const_iterator itc;
	
	//Image analysis with blob detection  
	self->leaderNumber = -1;
	self->leaderPosition = vector<int> (2, -1);

    //******** Add Your Initialization Above this Line *********
    
    while(self->running) //Continue to loop until system is closed
    {
    	if(self->reset) // activate resetting of parameters (especially PID errors)
    	{
    		radiusError = 0;
    		radiusIntegral = 0;
    		radiusLastError = 0;
            radiusDerivative = 0; 
            self->reset = false;
            cout << "Parameters reset !" << endl;
    	}
        if(self->enabled)
        {    	
        	if(!newFrameAvailable)
            {
                //usleep(100000);
                //if(!newFrameAvailable) printf("Uh-Oh!  No New Frames\n");
                //If you would like to handle this situation, put it here.
                //continue; //planner will only run when new data is available
            } else {
	
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
            altitudeMove = alt_Kp * alt_error + alt_Ki * alt_integral + alt_Kd * alt_derivative;
            if(abs(altitudeMove) > 4000) altitudeMove = altitudeMove/abs(altitudeMove) * 4000; // We don't want changes to be too violent
            self->dgaz = ALTp + altitudeMove;
            alt_lasterror = alt_error;
            
    
            // Registering angles of inclination
            euler_theta = globalNavData.euler_theta/1000 - globalNavData.trim_theta;
            euler_phi = globalNavData.euler_phi/1000 - globalNavData.trim_phi;
            
            if(abs((int) euler_theta) > 180) euler_theta = euler_theta/abs((int) euler_theta) * 180;
            if(abs((int) euler_phi) > 180) euler_phi = euler_phi/abs((int) euler_phi) * 180;
            euler_phiPrevious = euler_phi;
            euler_thetaPrevious = euler_theta;
            
            //printf("Altitude = %d ; phi = %-.1f\n", ALTITUDE, euler_phi);
			
            /**
             * PID controler
             */
             //cout << "Position of leader (" << self->leaderNumber << ") = {" << self->leaderPosition[0] << ", " << self->leaderPosition[1] << "}\n";
             if(-1 != self->leaderPosition[0] && -1 != self->leaderPosition[1]){
                init = clock();

				xval = self->leaderPosition[0];
				yval = self->leaderPosition[1];

                if((xval == 0 || yval == 0) || (xval == xvalPrevious && yval == yvalPrevious)) continue; // bug in navdata received
                xvalPrevious = xval;
                yvalPrevious = yval;
                int xnav = xval, ynav = yval;
                cornerToCenterCoordinates(&xval, &yval, XMIDDLE, YMIDDLE);
                xval = convertToMM(xval, FOV, ALTITUDE, XMIDDLE);
                yval = convertToMM(yval, FOV, ALTITUDE, YMIDDLE);
                //cout << "x = " << xval << "; y = " << yval << "; phi = " << euler_phi << "; theta = " << euler_theta << endl;
                int xval_tmp = xval, yval_tmp = yval;
                xval = convertToRealCoordinate_old(xval, ALTITUDE, euler_phi); 		//correct tilting
                yval = convertToRealCoordinate_old(yval, ALTITUDE, - euler_theta);	//correct tilting
                cout << "x = " << xval << ", y = " << yval << endl;
                                
                // Convert from cartesian coordinates to polar coordinates
                convertToPolarCoordinates(xval, yval, &radius, &theta);
                //printf("xc[0], yc[0], r, theta: (%d, %d); (%-.1f, %-.1f) e_theta = %-.1f ; e_phi = %-.1f\n", xval, yval, radius, (theta* 180 / M_PI), euler_theta, euler_phi);
                
                if(sqrt(xvalPrevious * xvalPrevious + yvalPrevious * yvalPrevious) <= 30) // We are really close to the target (radius in pixels < constant)
                	self->hover =1; // now hover using the embedded hovering algorithm
                else {
					// Proportionnal Integrator Controller
					// Polar coordinates PID
					radiusError = radius; // we want radius = 0
					radiusIntegral = dampen * radiusIntegral + radiusError;
					radiusDerivative = radiusError - radiusLastError;
					radiusMove = self->kp * radiusError + self->ki * radiusIntegral + self->kd * radiusDerivative;
					if(abs(radiusMove) > 25000) radiusMove = radiusMove/abs(radiusMove) * 25000;
					radiusLastError = radiusError;
					setDirection(false, theta, radiusMove, &self->dpitch, &self->droll);
	  
					// Save datalog
					final = clock() - dataClk;
					dataClkPrint = (double)final/ ((double)TICKS_PER_SEC);
					//fprintf(moveFile, "%-.2f %f %f %d %d %d %d %d %d %d\n", dataClkPrint, euler_theta, euler_phi, (int) altitudeMove,(int) radiusMove, (int) self->kp * radiusError, (int) self->ki * radiusIntegral, (int) self->kd * radiusDerivative, (int) radius, (int) (theta* 180 / M_PI));
					fprintf(moveFile, "%-.2f %d %f %f %d %d %d %d %d %d %-.1f %-.1f %-.1f\n", dataClkPrint, ALTITUDE, euler_theta, euler_phi, xval_tmp, yval_tmp, xval, yval, xnav, ynav, self->kp, self->ki, self->kd);
					
					//ardrone_at_set_led_animation(BLINK_ORANGE, 10, (float) 0.01);
                }
            } else {
                final = clock() - init;
                time_lost = (double)final / ((double)TICKS_PER_SEC);
                //printf("Target lost for %f seconds\n", time_lost);

                if(time_lost > 0 && time_lost < 1){
                    setDirection(false, theta, radiusMove, &self->dpitch, &self->droll); // Apply same correction as before, trying to find the roundel again
                    self->hover = 1;
                } else {
                    self->hover = 1;
                }
                cout << "Hovering" << endl;
            }

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
        //usleep(50000);
    }

    //De-initialization
    fclose(moveFile);
    cvDestroyWindow(cvAlgWindow);
    cvDestroyWindow(cvBotAlgWindow);
    cvDestroyWindow("DroneView");
    cout << "PID parameters : {Kp = " << setprecision(1) << self->kp << "; Ki = " << self->ki << "; Kd = " << self->kd << endl;
    cout << "Planner thread returned" << endl;

    return 0;
}

void* Video_Thread(void *params)
{
    Planner *self = (Planner *)params;
    
    while(bottomImgStream == NULL) {usleep(100000);} //wait for initialization
   	cout << "Starting thread Video_Thread ++++++++++++++++" << endl;

	ColorMatcher* colorMatcher = new ColorMatcher(false, true); // Params: Video source from webcam, DisplayVideo, DisplayAdvancedPanels 
	IplImage* frame;
	
	self->NB_ROBOTS = colorMatcher->getRobotsNumber();

	cout << "ColorMatcher created" << endl;
	while ((frame = bottomImgStream) != NULL && self->running) {
		colorMatcher->setAltitude(ALTITUDE); //update camera's altitude, so the mapping scale keeps being right
		colorMatcher->setOrientation(PSI); //Orientation of the camera, sent to the server
		colorMatcher->analyzeFrame(frame);
		self->leaderNumber = colorMatcher->getLeaderNumber();
		self->leaderPosition = colorMatcher->getLeaderPosition();
		//cout << "Position of leader (" << self->leaderNumber << ") = {" << self->leaderPosition[0] << ", " << self->leaderPosition[1] << "}\n";
		pthread_yield();
	} 
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

    //Create the Planner and Video Threads
    videoThreadId = pthread_create( &videoThread, NULL, Video_Thread, (void*) this);
    plannerThreadId = pthread_create( &plannerThread, NULL, Planner_Thread, (void*) this);
}

Planner::~Planner()
{
    //destructor
    cout << "Destroying planner" << endl;
    running = false;
    cvDestroyAllWindows();
}

