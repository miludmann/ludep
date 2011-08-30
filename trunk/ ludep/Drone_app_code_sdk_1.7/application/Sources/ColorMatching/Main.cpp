/**
 * Main to test ColorMatcher with a usb webcam
 * Michaël Ludmann (michael@ludep.com)
 * Aarhus University
 * 30/08/11
 */

#include <stdio.h>// includes C standard input/output definitions'
#include <stdlib.h>
#include <iostream>
#include "colorMatcher.hpp" //include common datastructurs used in this server
#include <math.h>


int main( int argc, char** argv ) {
	
	ColorMatcher* colorMatcher = new ColorMatcher(true, true);
	IplImage* frame;
	
	while ((frame = colorMatcher->getCurrentFrame()) != NULL) {
		colorMatcher->analyzeFrame(frame);
	} //main loop

	return 0;
}
