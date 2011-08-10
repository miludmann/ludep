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
//#include "xbee/xbee.hpp"
#include <sys/time.h>
#include <fstream>
#include <time.h>


using namespace std;

//Globablly Accessable Variables:
extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
extern int32_t ALTITUDE; //Altitude of drone in mm (defined in Navdata/navdata.c)
extern float32_t PSI; //Current Direction of Drone (-180deg to 180deg) (defined in Navdata/navdata.c)
extern NavDataContainer globalNavData; //Navdata (defined in Navdata/NavDatacontainer.cpp)
extern int newFrameAvailable; //if navdata indicates a new frame (defined in Navdata/navdata.c)

//Xbee *xbee;

extern "C" {	
  extern void ardrone_at_set_detect_typeC(CAD_TYPE detect_type)
  {
    char msg[64];
    sprintf( &msg[0], "%d", detect_type);
    ardrone_at_set_toy_configuration( "detect:detect_type", msg );
  } 
}

//Thread to run separately:
void *Planner_Thread(void *params)
{
  Planner *self = (Planner *)params;
  //xbee = new Xbee(7, 12.0);
  while(frontImgStream == NULL) {usleep(100000);} //wait for initialization
  cvStartWindowThread();
  cvNamedWindow(cvAlgWindow, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(cvBotAlgWindow, CV_WINDOW_AUTOSIZE);
  
  //******** Add Your Initialization Below this Line *********

  ardrone_at_set_detect_typeC(CAD_TYPE_COCARDE);
  int x=0, y=0;
  int nb_detected=0;
  const int SPEED_X = 2000, SPEED_Y = 2000;
  const int XMAX = 920, YMAX = 920;
  const int MARGIN = 50;
  const int XC_INF = XMAX/2 - MARGIN, XC_SUP = XMAX/2 + MARGIN, YC_INF = YMAX/2 - MARGIN, YC_SUP = YMAX/2 + MARGIN;
  const int MIDDLE = XMAX/2;
  clock_t init = clock(), final;
  double time_lost = 0;
  bool x_ok = true, y_ok = true;

  //******** Add Your Initialization Above this Line *********

  while(self->running) //Continue to loop until system is closed
  {
    if(self->enabled)
    {
      if(!newFrameAvailable)
      {
        usleep(100000);
        if(!newFrameAvailable) printf("\n    Uh-Oh!  No New Frames \n\n");
	//If you would like to handle this situation, put it here.
        continue; //planner will only run when new data is available
      }
      newFrameAvailable = 0; //reset
      //xbee->updateFrontDeriv(); //Update xbee (if applicable)
      
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
      self->dpitch = 0; // No forward motion
      self->droll = 0; // No side-to-side motion
      self->dyaw = 0; // No turning motion
      self->dgaz = ALTITUDE - 1600; // Adjust altitude by difference from goal.
      self->hover = 0;

      //printf("Navdata - altitude: %d \n", globalNavData.altitude);
      //printf("Navdata - nb_detected: %d \n", globalNavData.vd_nb_detected);
      //printf("Navdata - xc[0], yc[0]: (%d, %d) \n", globalNavData.vd_xc[0],globalNavData.vd_yc[0]);
      
      nb_detected = globalNavData.vd_nb_detected;
      
      if(nb_detected >= 1){
      	  printf("Navdata - nb_detected: %d \n", nb_detected);
      	  printf("Navdata - xc[0], yc[0]: (%d, %d) \n", globalNavData.vd_xc[0],globalNavData.vd_yc[0]);
      	  init = clock();
		  x=globalNavData.vd_xc[0];
		  y=globalNavData.vd_yc[0];
		  
		  if(x<XC_INF || x>XC_SUP){
			  self->droll = SPEED_X * (x - MIDDLE)/MIDDLE;
			  x_ok = false;
		  } else {
		  	  x_ok = true;
		  }
		  
		  if(y<YC_INF || y>YC_SUP){
			  self->dpitch = SPEED_Y * (y - MIDDLE)/MIDDLE;
			  y_ok = false;
		  } else {
		  	  y_ok = true;
		  }
		  //ardrone_at_set_led_animation(BLINK_ORANGE, 10, (float) 0.01);
      } else {
      	  final = clock() - init;
      	  time_lost = (double)final / ((double)CLOCKS_PER_SEC);
      	  //printf("Target lost for %f seconds\n", time_lost);
      	  
      	  if(time_lost > 0 && time_lost < (float) 1.5){
      	  	  printf("*************Looking for target**********************\n");
      	  	  x=globalNavData.vd_xc[0];
			  y=globalNavData.vd_yc[0];
			  
			  if(x<XC_INF || x>XC_SUP){
				  self->droll = SPEED_X/2 * (x - MIDDLE)/abs(x- MIDDLE);
				  x_ok = false;
			  } else {
				  x_ok = true;
			  }
			  
			  if(y<YC_INF || y>YC_SUP){
				  self->dpitch = SPEED_Y/2 * (y - MIDDLE)/abs(y - MIDDLE);
				  y_ok = false;
			  } else {
				  y_ok = true;
			  }	  
      	  } else {
      	  	  printf("TARGET LOST\n");  
      	  	  self->hover = 1;
      	  }
      }
      
      self->droll > 0 ? printf("RIGHT\n") : printf("LEFT\n");
      self->dpitch > 0 ? printf("BACKWARD\n") : printf("FORWARD\n");

      /*
      if((x_ok && y_ok) || nb_detected == 0){
      	  self->hover = 1;
      }
      */
      
      //printf("hover  = %d\n", self->hover);
      	  
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
  cvDestroyWindow(cvAlgWindow);
  cvDestroyWindow(cvBotAlgWindow);
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

