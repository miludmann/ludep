//
//  MenuUpdater.h
//  Updater
//
//  Created by Robert Ryll on 10-05-14.
//  Copyright Playsoft 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MenuController.h"

#include <CFNetwork/CFNetwork.h>

#if TARGET_CPU_X86 == 1 // We are on iPhone simulator
#define WIFI_ITFNAME "en1"
#endif // TARGET_CPU_X86

#if TARGET_CPU_ARM == 1 // We are on real iPhone
#define WIFI_ITFNAME "en0"
#endif // TARGET_CPU_ARM

@interface MenuUpdater : UIViewController <MenuProtocol,NSStreamDelegate,ARDroneProtocolOut>
{
	MenuController   *controller;
	BOOL running;
	
	BOOL compiledForIPad;
	BOOL usingIPad;
	BOOL using2xInterface;
	
	IBOutlet UILabel *firmwareVersionLabel;
	IBOutlet UILabel *statusLabel;
	
	IBOutlet UIButton	*okButton;
	IBOutlet UIButton	*cancelButton;
	
	IBOutlet UIProgressView *sendProgressView;

	UILabel *stepLabel[STEP_LINE_NR];
	UIImageView *stepImageView[STEP_LINE_NR];
	
	UIActivityIndicatorView *stepIndicator;
	
	NSString *firmwarePath;
	NSString *firmwareFileName;
	NSString *firmwareVersion;
	NSString *repairPath;
	NSString *repairFilename;
	NSString *repairVersion;
	NSString *bootldrPath;
	NSString *bootldrFilename;
	
	NSString *droneFirmwareVersion;
	NSString *errorLog;
	NSString *localIP;

	CFReadStreamRef ftpStreamTest;
    NSOutputStream *_networkStream;
    NSInputStream *_fileStream;
    uint8_t _buffer[SEND_BUFFER_SIZE];
    size_t _bufferOffset;
    size_t _bufferLimit;
		//	ARDrone *drone;
	BOOL droneRestarted;
	NSInteger retryCounter;
}

@property (nonatomic, retain) NSString *firmwarePath;
@property (nonatomic, retain) NSString *firmwareFileName;
@property (nonatomic, retain) NSString *repairPath;
@property (nonatomic, retain) NSString *repairFileName;
@property (nonatomic, retain) NSString *repairVersion;
@property (nonatomic, retain) NSString *bootldrPath;
@property (nonatomic, retain) NSString *bootldrFileName;
@property (nonatomic, retain) NSString *firmwareVersion;
@property (nonatomic, retain) NSString *droneFirmwareVersion;
@property (nonatomic, retain) NSString *localIP;
@property (nonatomic, retain) NSString *errorLog;

@property (nonatomic, retain) NSOutputStream *networkStream;
@property (nonatomic, retain) NSInputStream *fileStream;
@property (nonatomic, readonly) uint8_t *buffer;
@property (nonatomic, assign) size_t bufferOffset;
@property (nonatomic, assign) size_t bufferLimit;

- (void)droneConnected:(NSString*)droneFirmwareVersion;
- (void)getVersionText;
- (void)getErrorLog;
- (void)checkRestart;
- (BOOL)checkFTP:(NSString *)fileName;
- (void)stopReceiveWithStatus:(NSString *)statusString;

- (IBAction)okAction:(id)sender;
- (IBAction)cancelAction:(id)sender;

@end
