//
//  AppDelegate.m
//  FreeFlight
//
//  Created by Frédéric D'HAEYER on 16/10/09.
//  Copyright Parrot SA 2009. All rights reserved.
//
#import "AppDelegate.h"
#import "EAGLView.h"
#import "MenuUpdater.h"
#import "ES1Renderer.h"

@implementation AppDelegate
@synthesize window;
@synthesize menuController;

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	NSLog(@"app did finish launching");
	application.idleTimerDisabled = YES;

	// Setup the menu controller
	menuController.delegate = self;
	NSLog(@"menu controller view %@", menuController.view);
	
	was_in_game = NO;
	
	// Setup the ARDrone
	ARDroneHUDConfiguration hudconfiguration = {YES, NO, YES};
	drone = [[ARDrone alloc] initWithFrame:menuController.view.frame withState:was_in_game withDelegate:menuController withHUDConfiguration:&hudconfiguration];
	
	// Setup the OpenGL view
	glView = [[EAGLView alloc] initWithFrame:menuController.view.frame];
	[glView setDrone:drone];
	[glView changeState:was_in_game];

	[menuController.view addSubview:drone.view];
	[menuController changeState:was_in_game];

	[window addSubview:menuController.view];
	[window addSubview:glView];
	[window bringSubviewToFront:menuController.view];
 	[window makeKeyAndVisible];
}

#pragma mark -
#pragma mark Drone protocol implementation
- (void)changeState:(BOOL)inGame
{
	was_in_game = inGame;
	
	if(inGame)
	{
		int value;
		[drone setScreenOrientationRight:(menuController.interfaceOrientation == UIInterfaceOrientationLandscapeRight)];
#ifdef DISPLAY_DART
		value = ARDRONE_VIDEO_CHANNEL_HORI;
		[drone setDefaultConfigurationForKey:ARDRONE_CONFIG_KEY_VIDEO_CHANNEL withValue:&value];
		
		value = ARDRONE_CAMERA_DETECTION_H_ORIENTED_COCARDE;
#else
		value = ARDRONE_CAMERA_DETECTION_NONE;
#endif
		[drone setDefaultConfigurationForKey:ARDRONE_CONFIG_KEY_DETECT_TYPE withValue:&value];
		
		value = 0;
		[drone setDefaultConfigurationForKey:ARDRONE_CONFIG_KEY_CONTROL_LEVEL withValue:&value];
		
	}
	
	[drone changeState:inGame];
	[glView changeState:inGame];
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	// Become inactive
	if(was_in_game)
	{
		[drone changeState:NO];
		[glView changeState:NO];
	}
	else
	{
		[menuController changeState:NO];
	}
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	if(was_in_game)
	{
		[drone changeState:YES];
		[glView changeState:YES];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	printf("%s : %d\n", __FUNCTION__, was_in_game);
	
	if(was_in_game)
	{
		[drone changeState:NO];
		[glView changeState:NO];
	}
	else
	{
		[menuController changeState:NO];
	}
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN_WITH_PARAM)commandIn fromSender:(id)sender refreshSettings:(BOOL)refresh
{
	
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	
}

- (void)setDefaultConfigurationForKey:(ARDRONE_CONFIG_KEYS)key withValue:(void *)value
{
	
}

- (BOOL)checkState
{
	BOOL result = NO;
	
	if(was_in_game)
	{
		result = [drone checkState];
	}
	else
	{
		//result = [menuController checkState];
	}
	
	return result;
}

@end
