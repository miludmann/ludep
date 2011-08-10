#ifndef _PLANNER_H_
#define _PLANNER_H_

#include <opencv/highgui.h>
#include <pthread.h>
#include <vector>

extern IplImage* frontImgStream; 
extern IplImage* bottomImgStream;

using namespace std;

class Planner
{
	public:
	  Planner();
	  ~Planner();
	  
	  int plannerThreadId, videoThreadId;
	  pthread_t plannerThread, videoThread;
	  bool running; //If the system is running    
	  bool enabled; //If the algorithms are enabled (should run)
	  int32_t dpitch, dyaw, droll, dgaz, hover; //Current Algorithm Results
	  int32_t dpitch_final, dyaw_final, droll_final, dgaz_final, hover_final; //Final Algorithm Results
	  int32_t angleInDegree, speed;
	  double kp, ki, kd; // PID parameters
	  bool reset; // If we want to reset planner parameters
	  int leaderNumber;
	  int NB_ROBOTS;
	  vector<int> leaderPosition;
};
    
#endif
