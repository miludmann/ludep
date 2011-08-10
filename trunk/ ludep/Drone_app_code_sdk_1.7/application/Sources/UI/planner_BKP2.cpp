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
    int32_t enemyColors = ARDRONE_DETECTION_COLOR_ORANGE_BLUE;
    ARDRONE_TOOL_CONFIGURATION_ADDEVENT (enemy_colors, &enemyColors, NULL);
    int32_t detectV = TAG_TYPE_MASK (TAG_TYPE_ROUNDEL);
    ARDRONE_TOOL_CONFIGURATION_ADDEVENT (detections_select_v, &detectV, NULL);
    //ardrone_at_set_detect_typeC(CAD_TYPE_COCARDE);
    
	// Embedded detection tests
	int embeddedTrackingEnabled = true;
	int frameMissed = 0, nbFramesMissed = 0, detectedByDrone = 0, detectedByCV = 0, detectedWhileMissed = 0;    
    
	int FOV = 54; // Camera field of view (depends on the camera used)
    int XMIDDLE = 88, YMIDDLE = 77; // Half of the camera resolution on each axis
    if(embeddedTrackingEnabled){
		XMIDDLE = 500;
		YMIDDLE = 500;
	}
	
    int xval=XMIDDLE, yval=YMIDDLE;
    int xvalPrevious=XMIDDLE, yvalPrevious=YMIDDLE;
    double radius = 0, theta = 0;
    int nb_detected=0;
    double euler_theta = 0.0, euler_phi = 0.0;
    double euler_thetaPrevious = 0.0, euler_phiPrevious = 0.0;
    deque <double> listPhi;
    double averagePhi = 0, sum = 0;
    int xval_old, yval_old;
    
   
    clock_t init = clock(), final;
    double time_lost = 0;

    // Altitude PID
    const double alt_Kp = 15, alt_Ki = 2, alt_Kd = 0, ALT_OFFSET = 1800, ALTp = 0;
    int alt_error = 0, alt_integral = 0, alt_lasterror = 0, alt_derivative = 0;
    double altitudeMove;

    // Tracking PID
    self->kp = 3.5, self->ki = 1.0, self->kd = 35;
    if(!embeddedTrackingEnabled) {
    	self->kp *= 5.8;
    	self->ki *= 5.8;
    	self->kd *= 5.8;
    }
    
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
    fprintf(moveFile, "time altitude euler_theta euler_phi xval_tmp yval_tmp xval yval xnav ynav kp ki kd xval_old yval_old\n");
    
    // Camera analysis
    IplImage* testFrame = 0;
    testFrame = cvCreateImage(cvSize(bottomImgStream->width, bottomImgStream->height), IPL_DEPTH_8U, bottomImgStream->nChannels);
    std::vector<cv::Vec3f> circles;
	cv::Mat bottomMatDraw, bottomHSV;
	std::vector<cv::Mat> bottomV;
	std::vector<cv::Vec3f>::const_iterator itc;
	
	//Image analysis with blob detection
      
	self->leaderNumber = -1;

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
            	frameMissed = 1;
            	nbFramesMissed++;
                //usleep(100000);
                //if(!newFrameAvailable) printf("Uh-Oh!  No New Frames\n");
                //If you would like to handle this situation, put it here.
                //continue; //planner will only run when new data is available
            } else {
            	frameMissed = 0;	
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
            
            /**
             * Image Analysis
             */
             /*
            circles.clear();
            // check for errors:
            if(!bottomImgStream)
                continue;
            cv::Mat bottomMat(bottomImgStream, false); //convert Ipl image to a matrix bottomMat
            //cv::GaussianBlur(bottomMat, bottomMat, cv::Size(5,5),1.5); 
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
            */
            // Keep a record of both algorithms efficiency (embedded and OpenCV)
            nb_detected = globalNavData.vd_nb_detected;  
            //printf("Navdata - nb_detected: %d \n", globalNavData.vd_nb_detected);

            if(nb_detected >= 1) {
            	detectedByDrone++;
            }
            
            if(nb_detected >= 1 && frameMissed)  detectedWhileMissed++;
            //if(circles.size() >= 1) detectedByCV = detectedByCV + 1 - frameMissed;
           
            
            euler_theta = globalNavData.euler_theta/1000 - globalNavData.trim_theta;
            euler_phi = globalNavData.euler_phi/1000 - globalNavData.trim_phi;
            
            if(abs((int) euler_theta) > 180) euler_theta = euler_theta/abs((int) euler_theta) * 180;
            if(abs((int) euler_phi) > 180) euler_phi = euler_phi/abs((int) euler_phi) * 180;
            //if((int) euler_phi != (int) euler_phiPrevious || (int) euler_theta != (int) euler_thetaPrevious)
            	//printf("Theta = %-.1f ; Phi = %-.1f\n",  euler_theta, euler_phi);

            euler_phiPrevious = euler_phi;
            euler_thetaPrevious = euler_theta;
            
            listPhi.push_front(euler_phi);
            if(listPhi.size() > 1)
            {
            	listPhi.pop_back();
            }
            sum = 0;
            for(int i = 0; i < 	listPhi.size(); i++)
            	sum += listPhi.at(i);
            averagePhi = sum / listPhi.size();
             
            //printf("Altitude = %d ; phi = %-.1f\n", ALTITUDE, euler_phi);
			
            /**
             * PID controler
             */

            if(/*(!embeddedTrackingEnabled && circles.size() >= 1) ||*/ (embeddedTrackingEnabled && nb_detected >= 1)){
                init = clock();
                //itc = circles.begin();
                if(!embeddedTrackingEnabled) {
                	xval = (*itc)[0];
                	yval = (*itc)[1];
                } else {
                	xval = globalNavData.vd_xc[0];
                	yval = globalNavData.vd_yc[0];
                }
                
                if((xval == 0 || yval == 0) || (xval == xvalPrevious && yval == yvalPrevious)) continue; // bug in navdata received
                xvalPrevious = xval;
                yvalPrevious = yval;
                int xnav = xval, ynav = yval;
                cornerToCenterCoordinates(&xval, &yval, XMIDDLE, YMIDDLE);
                xval = convertToMM(xval, FOV, ALTITUDE, XMIDDLE);
                yval = convertToMM(yval, FOV, ALTITUDE, YMIDDLE);
                cout << "x = " << xval << "; y = " << yval << "; phi = " << euler_phi << "; theta = " << euler_theta << endl;
                int xval_tmp = xval, yval_tmp = yval;
                xval = convertToRealCoordinate(xval, ALTITUDE, euler_phi);
                yval = convertToRealCoordinate(yval, ALTITUDE, - euler_theta);
                cout << "x =  " << xval << ", y = " << yval << "\n" << endl;
                xval_old = convertToRealCoordinate_old(xval_tmp, ALTITUDE, euler_phi);
                yval_old = convertToRealCoordinate_old(yval_tmp, ALTITUDE, - euler_theta);
                                
                // Convert from cartesian coordinates to polar coordinates
                convertToPolarCoordinates(xval, yval, &radius, &theta);
                //printf("xc[0], yc[0], r, theta: (%d, %d); (%-.1f, %-.1f) e_theta = %-.1f ; e_phi = %-.1f\n", xval, yval, radius, (theta* 180 / M_PI), euler_theta, euler_phi);
                
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
				fprintf(moveFile, "%-.2f %d %f %f %d %d %d %d %d %d %-.1f %-.1f %-.1f %d %d\n", dataClkPrint, ALTITUDE, euler_theta, averagePhi, xval_tmp, yval_tmp, xval, yval, xnav, ynav, self->kp, self->ki, self->kd, xval_old, yval_old);
				
                //ardrone_at_set_led_animation(BLINK_ORANGE, 10, (float) 0.01);
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
    printf("Detection efficency : drone = %d ; OpenCV = %d ; nbFramesMissed = %d ; detectedWhileMissed = %d ; efficiency = %-.1f %% \n", detectedByDrone, detectedByCV, nbFramesMissed, detectedWhileMissed, (100 * ((float) detectedByCV - (float) detectedByDrone)/((float) detectedByCV)));
    cout << "PID parameters : {Kp = " << setprecision(1) << self->kp << "; Ki = " << self->ki << "; Kd = " << self->kd << endl;
    cout << "Planner thread returned" << endl;

    return 0;
}

void* Video_Thread(void *params)
{
    Planner *self = (Planner *)params;
    
    while(bottomImgStream == NULL) {usleep(100000);} //wait for initialization
   	cout << "Starting thread Video_Thread ++++++++++++++++" << endl;

	ColorMatcher* colorMatcher = new ColorMatcher(false, true, false); // Params: Video source from webcam, DisplayVideo, DisplayAdvancedPanels 
	IplImage* frame;
	
	self->NB_ROBOTS = colorMatcher->NB_ROBOTS;

	cout << "ColorMatcher created" << endl;
	while ((frame = bottomImgStream) != NULL && self->running) {
		colorMatcher->setAltitude(ALTITUDE); //update camera's altitude, so the mapping scale keeps being right
		colorMatcher->setOrientation(PSI); //Orientation of the camera, sent to the server
		colorMatcher->analyzeFrame(frame);
		self->leaderNumber = colorMatcher->getLeaderNumber();
		self->robotsPositions = colorMatcher->getPositions();
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
    plannerThreadId = pthread_create( &plannerThread, NULL, Planner_Thread, (void*) this);
    videoThreadId = pthread_create( &videoThread, NULL, Video_Thread, (void*) this);
}

Planner::~Planner()
{
    //destructor
    cout << "Destroying planner" << endl;
    running = false;
    cvDestroyAllWindows();
}

