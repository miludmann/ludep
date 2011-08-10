//
//  ARUpdaterViewController.m
//  AR.Updater
//
//  Created by Robert Ryll on 10-05-14.
//  Copyright Parrot SA 2011. All rights reserved.
//

#import "MenuUpdater.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "plf.h"

@interface MenuUpdater ()

enum {
	cs_idle,
	cs_restart,
	cs_
	
} checkstate;

enum {
	s_connect,
	s_repair,
	s_check,
	s_send,
	s_restart,
	s_install,
	s_recheck
} state;

enum {
	ss_user,
	ss_app
} substate;

enum {
	ss_repair_one,
	ss_repair_two,
} substate_repair;

enum {
	r_none,
	r_progress,
	r_problem,
	r_fail,
	r_pass
};

NSInteger firmwareSize;
NSInteger sendedSize;
BOOL streamOpen;
BOOL end;

- (void)stateUpdate:(int)newState Result:(int)result Message:(NSString*)message;
- (void)stopSendWithStatus:(NSString *)statusString;
- (void)checkStream;

@end

@implementation MenuUpdater
@synthesize firmwareVersion;
@synthesize firmwarePath;
@synthesize firmwareFileName;
@synthesize repairPath;
@synthesize repairFileName;
@synthesize repairVersion;
@synthesize bootldrPath;
@synthesize bootldrFileName;
@synthesize droneFirmwareVersion, errorLog;
@synthesize localIP;

@synthesize networkStream = _networkStream;
@synthesize fileStream = _fileStream;
@synthesize bufferOffset = _bufferOffset;
@synthesize bufferLimit = _bufferLimit;

#pragma mark init
- (id) initWithController:(MenuController*)menuController
{
	NSArray *targetArray = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"UIDeviceFamily"];
	compiledForIPad = NO;
	self.localIP = nil;
	
	for (int i = 0; i < [targetArray count]; i++)
	{
		NSNumber* num = (NSNumber*)[targetArray objectAtIndex:i];
		int value;
		[num getValue:&value];
		if (2 == value)
		{
			compiledForIPad = YES;
			break;
		}
	}
	
	usingIPad = NO;
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		usingIPad = YES;
	}
	
	using2xInterface = NO;
	if ([UIScreen instancesRespondToSelector:@selector(scale)]) {
		CGFloat scale = [[UIScreen mainScreen] scale];
		if (scale > 1.0) {
			using2xInterface = YES;
		}
	}	
	
	if (usingIPad && compiledForIPad)
	{
		self = [super initWithNibName:@"MenuUpdater-iPad" bundle:nil];
	}
	else
	{
		self = [super initWithNibName:@"MenuUpdater" bundle:nil];
	}
	
	if (self)
	{
		controller = menuController;
	}
	return self;
}

- (void) viewDidLoad
{
	NSString *plistPath = [[NSBundle mainBundle] pathForResource:@"Firmware" ofType:@"plist"];
	NSDictionary *plistDict = [NSDictionary dictionaryWithContentsOfFile:plistPath];
	self.firmwareFileName = [plistDict objectForKey:@"FirmwareFileName"];
	self.repairFileName = [plistDict objectForKey:@"RepairFileName"];
	self.bootldrFileName = [plistDict objectForKey:@"BootldrFileName"];
	self.repairVersion = [plistDict objectForKey:@"RepairVersion"];
	
	self.firmwarePath = [[NSBundle mainBundle] pathForResource:firmwareFileName ofType:@"plf"];
	self.repairPath = [[NSBundle mainBundle] pathForResource:repairFileName ofType:@"bin"];
	self.bootldrPath = [[NSBundle mainBundle] pathForResource:bootldrFileName ofType:@"bin"];
	plf_phdr plf_header;
	

	if(plf_get_header([self.firmwarePath cStringUsingEncoding:NSUTF8StringEncoding], &plf_header) != 0)
		memset(&plf_header, 0, sizeof(plf_phdr));
	
	self.firmwareVersion = [NSString stringWithFormat:@"%d.%d.%d", plf_header.p_ver, plf_header.p_edit, plf_header.p_ext];
	
	NSDictionary *firmwareAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:firmwarePath error:nil];
	NSNumber *firmwareSizeNumber = [firmwareAttributes objectForKey:NSFileSize];
	firmwareSize = [firmwareSizeNumber unsignedIntegerValue];
	sendedSize = 0;
	
	firmwareVersionLabel.text = [NSString stringWithFormat:@"v%s", [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] cStringUsingEncoding:NSUTF8StringEncoding]];
		
	NSString *steps[] = {
		NSLocalizedString(@"Connecting", @""),
		NSLocalizedString(@"Checking/updating bootloader", @""),
		NSLocalizedString(@"Checking version", @""),
		NSLocalizedString(@"Sending file", @""),
		NSLocalizedString(@"Restarting AR.Drone", @""),
		NSLocalizedString(@"Installing", @""),
		NSLocalizedString(@"Checking again", @"")
	};
	
	for (NSInteger i = 0; i < STEP_LINE_NR; i++) {
		stepLabel[i] = [UILabel new];
		stepLabel[i].backgroundColor = [UIColor clearColor];
		stepLabel[i].textColor = NORMAL_COLOR;
		if (usingIPad && compiledForIPad)
		{
			stepLabel[i].frame = CGRectMake(IPAD_LINE_H + IPAD_OFFSET_X, IPAD_STEP_LINE_Y + IPAD_LINE_H * i, IPAD_SCREEN_H - IPAD_LINE_H, IPAD_LINE_H);
			stepLabel[i].text = steps[i];
			[self.view addSubview:stepLabel[i]];
			
			stepImageView[i] = [UIImageView new];
			stepImageView[i].frame = CGRectMake(IPAD_OFFSET_X, IPAD_STEP_LINE_Y + i * IPAD_LINE_H, IPAD_LINE_H, IPAD_LINE_H);
			stepImageView[i].image = [UIImage imageNamed:@"Empty-iPad.png"];
			[self.view addSubview:stepImageView[i]];
		}
		else
		{
			stepLabel[i].frame = CGRectMake(LINE_H, STEP_LINE_Y + LINE_H * i, SCREEN_H - LINE_H, LINE_H);
			stepLabel[i].text = steps[i];
			[self.view addSubview:stepLabel[i]];
			
			stepImageView[i] = [UIImageView new];
			stepImageView[i].frame = CGRectMake(0, STEP_LINE_Y + i * LINE_H, LINE_H, LINE_H);
			stepImageView[i].image = [UIImage imageNamed:@"Empty.png"];
			[self.view addSubview:stepImageView[i]];
		}
		if(i > s_check)
		{
			stepLabel[i].hidden = YES;
			stepImageView[i].hidden = YES;	
		}
	}
	
	[okButton setTitle:NSLocalizedString(@"Ok", @"Button title") forState:UIControlStateNormal];
	
	[cancelButton setTitle:NSLocalizedString(@"Cancel", @"Button title") forState:UIControlStateNormal];

	stepIndicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle: UIActivityIndicatorViewStyleWhite];
	stepIndicator.hidesWhenStopped = YES;
	[self.view addSubview:stepIndicator];
	[stepIndicator release];
	
	state = s_connect;
	substate = ss_app;
	substate_repair = ss_repair_one;
	//end = NO;
	okButton.hidden = YES;
	cancelButton.hidden = YES;
	sendProgressView.hidden = YES;
	[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Trying to connect to AR.Drone", @"")];

	droneRestarted = NO;
	retryCounter = 0;
	[self performSelectorInBackground:@selector(initConnection) withObject:nil];
}

#pragma mark flow
- (void)executeCommandOut:(ARDRONE_COMMAND_OUT)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	switch(commandId)
	{
		case ARDRONE_COMMAND_RUN:
			self.localIP = [NSString stringWithCString:(char*)parameter encoding:NSUTF8StringEncoding];
			break;
			
		default:
			break;
	}
}

- (void) initConnection
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if(self.localIP != nil)
	{
	   NSLog(@"local ip %@", localIP);
		if(![self checkFTP:@"version.txt"] && retryCounter < FTP_MAX_RETRIES)
		{
			NSLog(@"checkFTP");
			retryCounter++;
			[self initConnection];
		}
		else if(retryCounter < FTP_MAX_RETRIES)
		{
			NSLog(@"get version text");
			retryCounter = 0;
			[self performSelectorOnMainThread:@selector(getVersionText) withObject:nil waitUntilDone:NO];
		}
		else
		{
			[self stateUpdate:s_connect Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Wifi not available, please connect your %@ to your AR.Drone", @""), [[UIDevice currentDevice] model]]];
		}
	}
	else 
	{
		[self stateUpdate:s_connect Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Wifi not available, please connect your %@ to your AR.Drone", @""), [[UIDevice currentDevice] model]]];
	}

	[pool release];
}
		 
- (void)stateUpdate:(int)newState Result:(int)result Message:(NSString*)message {
	statusLabel.text = message;
	[stepIndicator stopAnimating];
	statusLabel.textColor = NORMAL_COLOR;
	if (newState > state) {
		if (usingIPad && compiledForIPad)
		{
			stepImageView[state].image = [UIImage imageNamed:@"Pass-iPad.png"];
		}
		else
		{
			stepImageView[state].image = [UIImage imageNamed:@"Pass.png"];
		}

	}
	state = newState;
	switch (result) {
		case r_none:
			if (state == s_send) {
				substate = ss_user;
				okButton.hidden = NO;
				cancelButton.hidden = NO;
			}
			break;
			
		case r_progress:
			if (usingIPad && compiledForIPad)
			{
				stepIndicator.frame = CGRectMake(IPAD_OFFSET_X + IPAD_INDICATOR_M, IPAD_STEP_LINE_Y + state * IPAD_LINE_H + IPAD_INDICATOR_M, IPAD_INDICATOR_S, IPAD_INDICATOR_S);
			}
			else
			{
				stepIndicator.frame = CGRectMake(INDICATOR_M, STEP_LINE_Y + state * LINE_H + INDICATOR_M, INDICATOR_S, INDICATOR_S);
			}
			[stepIndicator startAnimating];
			if (state == s_send) 
			{
				substate = ss_app;
				okButton.hidden = YES;
				cancelButton.hidden = YES;
				sendProgressView.hidden = NO;
				[sendProgressView setProgress:0.0];
				sendedSize = 0;
				for(int i = s_check ; i < STEP_LINE_NR ; i++)
				{
					stepLabel[i].hidden = NO;
					stepImageView[i].hidden = NO;
				}
			}
			
			if (state == s_install) 
			{
				sendProgressView.hidden = YES;
			}
			break;
		case r_problem:
			statusLabel.textColor = PROBLEM_COLOR;
			if (usingIPad && compiledForIPad)
			{
				stepImageView[state].image = [UIImage imageNamed:@"Problem-iPad.png"];
			}
			else
			{
				stepImageView[state].image = [UIImage imageNamed:@"Problem.png"];
			}
			okButton.hidden = YES;
			cancelButton.hidden = YES;
			sendProgressView.hidden = YES;				
			break;
		case r_fail:
			//end = YES;
			statusLabel.textColor = FAIL_COLOR;
			if (usingIPad && compiledForIPad)
			{
				stepImageView[state].image = [UIImage imageNamed:@"Fail-iPad.png"];
			}
			else
			{
				stepImageView[state].image = [UIImage imageNamed:@"Fail.png"];
			}
			okButton.hidden = YES;
			cancelButton.hidden = YES;
			sendProgressView.hidden = YES;				
			break;
		case r_pass:
			//end = YES;
			statusLabel.textColor = PASS_COLOR;
			if (usingIPad && compiledForIPad)
			{
				stepImageView[state].image = [UIImage imageNamed:@"Pass-iPad.png"];
			}
			else
			{
				stepImageView[state].image = [UIImage imageNamed:@"Pass.png"];
			}
			break;
		default:
			break;
	}
}

- (void) repair
{
	int socket_desc;
	struct sockaddr_in address;
	
	socket_desc=socket(AF_INET,SOCK_STREAM,0);
	if (socket_desc>-1)
	{
		char buffer[1024];
		int size;
		printf("Socket created\n");
	
		/* type of socket created in socket() */
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr([self.localIP cStringUsingEncoding:NSUTF8StringEncoding]);
		
		/* TELNET_PORT is the port to use for connections */
		address.sin_port = htons(TELNET_PORT);
		/* connect the socket to the port specified above */
		connect(socket_desc, (struct sockaddr *)&address, sizeof(struct sockaddr));
		
		// Change access right to XR 
		sprintf(buffer, "cd `find /data -name \"repair\" -exec dirname {} \\;` && chmod 755 repair\n");
		size = send(socket_desc, buffer, strlen(buffer), 0);
		if(size != strlen(buffer))
		{
			printf("Cannot change access right ...\n");
			return;
		}
		
		// execute repair binary file 
		sprintf(buffer, "cd `find /data -name \"repair\" -exec dirname {} \\;` && ./repair\n");
		size = send(socket_desc, buffer, strlen(buffer), 0);
		if(size != strlen(buffer))
		{
			printf("Cannot execute binary to repair AR.Drone ...\n");
			return;
		}
		
		close(socket_desc);
	}
	else
	{
		perror("Can't create socket");
	}
}

- (void)droneConnected:(NSString*)droneFirmwareVersion2 {
/*	if (end) 
	{
		NSLog(@"EEEEEEENNNNNNNNNNDDDDDDDD\n");
		return;
	}
*/	
	if (state == s_connect) 
	{
		if([self.droneFirmwareVersion compare:self.repairVersion options:NSNumericSearch] == -1)
		{
			[self stateUpdate:s_repair Result:r_progress Message:NSLocalizedString(@"Checking/Updating bootloader", @"")];
			
			assert(self.networkStream == nil);
			assert(self.fileStream == nil);
			streamOpen = NO;
			
			self.fileStream = [NSInputStream inputStreamWithFileAtPath:repairPath];
			assert(self.fileStream != nil);
			[self.fileStream open];
			
			NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, self.localIP, FTP_REPAIR_PORT, [NSString stringWithFormat:@"%@", self.repairFileName]]];

			CFWriteStreamRef ftpStream = CFWriteStreamCreateWithFTPURL(NULL, (CFURLRef) url);
			assert(ftpStream != NULL);
		
			self.networkStream = (NSOutputStream *) ftpStream;
			self.networkStream.delegate = self;
			[self.networkStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
			[self.networkStream open];
			
			CFRelease(ftpStream);
			[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		}
		else
		{
			[self stateUpdate:s_repair Result:r_pass Message:@""];
			[self droneConnected:droneFirmwareVersion2];
		}
	}
	else if(state == s_repair) {
		[self stateUpdate:s_check Result:r_none Message:@""];
		switch ([firmwareVersion compare:droneFirmwareVersion2 options:NSNumericSearch]) {
			case -1:
				[self stateUpdate:s_check Result:r_problem Message:[NSString stringWithFormat:NSLocalizedString(@"AR.Drone firmware %@\n%@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Please update this application", @""), NSLocalizedString(@"Launching Application...", @"")]];
				[self performSelector:@selector(cancelAction:) withObject:nil afterDelay:TIME_BEFORE_LAUNCH];
				break;
			case 0:
				[self stateUpdate:s_check Result:r_pass Message:[NSString stringWithFormat:NSLocalizedString(@"AR.Drone firmware %@\n%@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"AR.Drone up to date", @""), NSLocalizedString(@"Launching Application...", @"")]];
				[self performSelector:@selector(cancelAction:) withObject:nil afterDelay:TIME_BEFORE_LAUNCH];
				break;
			case 1:
				[self stateUpdate:s_send Result:r_none Message:[NSString stringWithFormat:NSLocalizedString(@"AR.Drone firmware %@\n%@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Firmware update available", @""), NSLocalizedString(@"Do you want to update the AR.Drone ?", @"")]];
				break;
		}
	}
	
	if (state == s_install) 
	{
		[self stateUpdate:s_recheck Result:r_none Message:@""];
		if ([firmwareVersion isEqualToString:droneFirmwareVersion2]) 
		{
			[self stateUpdate:s_recheck Result:r_pass Message:[NSString stringWithFormat:NSLocalizedString(@"AR.Drone firmware %@\n%@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Update installed", @""), NSLocalizedString(@"Launching Application...", @"")]];
			[self performSelector:@selector(cancelAction:) withObject:nil afterDelay:TIME_BEFORE_LAUNCH];
		} 
		else 
		{
			//check errors
			if([self checkFTP:@"err.log"])
				[self getErrorLog];
			else
				[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"AR.Drone firmware %@\n%@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Update process failed", @""), @""]];
		}
	}
}

#pragma mark FTP

- (void)getVersionText
{
	NSLog(FTP_ADDRESS, self.localIP, FTP_PORT, @"version.txt");
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, self.localIP, FTP_PORT, @"version.txt"]];
	NSURLRequest *theRequest = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:4.0];
	
	if(state == s_connect)
		[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Trying to connect to AR.Drone", @"")];
	else {
		[self stateUpdate:s_install Result:r_pass Message:NSLocalizedString(@"Checking again", @"")];
		//end = NO;
	}
	
	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;	
	
	NSData *data = [NSURLConnection sendSynchronousRequest:theRequest returningResponse:nil error:nil];

	if(data != nil)
	{
		NSLog(@"retrieved version");
		
		NSString *version = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
		self.droneFirmwareVersion = version;
		[version release];
		
		self.droneFirmwareVersion = [self.droneFirmwareVersion stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];
		if(state != s_recheck)
			[self droneConnected:self.droneFirmwareVersion];
	}
	else 
	{
		NSLog(@"can't get version.txt");
		[self stateUpdate:state Result:r_fail Message:NSLocalizedString(@"Cannot connect to AR.Drone", @"")];
	}
}

- (void)getErrorLog
{
	
	NSLog(FTP_ADDRESS, self.localIP, FTP_PORT, @"err.log");
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, self.localIP, FTP_PORT, @"err.log"]];
	NSURLRequest *theRequest = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:4.0];
	
	if(state == s_connect)
		[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Trying to connect to AR.Drone", @"")];
	else if(state == s_restart)
	{
		[self stateUpdate:s_restart Result:r_pass Message:NSLocalizedString(@"Checking again", @"")];
		//end = NO;
	}
	
	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;	
	
	NSData *data = [NSURLConnection sendSynchronousRequest:theRequest returningResponse:nil error:nil];
	
	if(data != nil)
	{
		self.errorLog = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
		self.errorLog = [self.errorLog stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];
		
		NSArray* log = [self.errorLog componentsSeparatedByString:@"="];
		if([log count] > 1)
		{
			self.errorLog = [log objectAtIndex:1];
			[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", NSLocalizedString(@"Update process failed", @""), self.errorLog]];
		}
		else if([log count] == 1)
		{
			self.errorLog = [log objectAtIndex:0];
			[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", NSLocalizedString(@"Update process failed", @""), self.errorLog]];						
		}
	}
	else
	{
		[self stateUpdate:state Result:r_fail Message:NSLocalizedString(@"Cannot connect to AR.Drone", @"")];
	}
}

- (IBAction)okAction:(id)sender 
{	
	assert(self.networkStream == nil);
	assert(self.fileStream == nil);
	streamOpen = NO;
	
	self.fileStream = [NSInputStream inputStreamWithFileAtPath:firmwarePath];
	assert(self.fileStream != nil);
	[self.fileStream open];
		
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, self.localIP, FTP_PORT, [NSString stringWithFormat:@"%@.plf", self.firmwareFileName]]];
	CFWriteStreamRef ftpStream = CFWriteStreamCreateWithFTPURL(NULL, (CFURLRef) url);
	assert(ftpStream != NULL);
	
	self.networkStream = (NSOutputStream *) ftpStream;

	self.networkStream.delegate = self;
	[self.networkStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[self.networkStream open];
	
	CFRelease(ftpStream);
	
	[self stateUpdate:s_send Result:r_progress Message:NSLocalizedString(@"Sending file", @"")];
	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		//sometimes (especially when previous file sending was interrupted) stream is not open without any feedback or error message
		//it's to check is stream for sure open and avoid app stuck
	[self performSelector:@selector(checkStream) withObject:nil afterDelay:1];
}

- (IBAction)cancelAction:(id)sender 
{	
	[controller changeMenu:nil];
}

-(void) startCheckRestart
{
	[self performSelectorInBackground:@selector(checkRestart) withObject:nil];
}

- (void) checkRestart
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if([self checkFTP:@"version.txt"])
	{
		if(!droneRestarted)
		{
			[NSThread sleepForTimeInterval:1.0];
			[self performSelectorInBackground:@selector(checkRestart) withObject:nil];
		}
		else
			[self performSelectorOnMainThread:@selector(getVersionText) withObject:nil waitUntilDone:NO];
	}
	else 
	{
		if(!droneRestarted)
		{
			NSLog(@"lost connection");
			droneRestarted = YES;
			if(state == s_restart)
			{
				[self stateUpdate:s_restart Result:r_pass Message:@""];
				[self stateUpdate:s_install Result:r_progress Message:[NSString stringWithFormat:@"%@...\n %@", NSLocalizedString(@"Installing", @""), NSLocalizedString(@"Please wait, this operation can take several minutes", @"")]];				
			}
			
			[NSThread sleepForTimeInterval:1.0];
			[self performSelectorInBackground:@selector(checkRestart) withObject:nil];
		}
		else 
		{
			[NSThread sleepForTimeInterval:1.0];
			[self performSelectorInBackground:@selector(checkRestart) withObject:nil];
		}
	}
	[pool release];
}

- (BOOL)checkFTP:(NSString *)fileName
{
	NSLog(FTP_ADDRESS, self.localIP, FTP_PORT, fileName);
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, self.localIP, FTP_PORT, fileName]];
	NSURLRequest *theRequest = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:4.0];
	
	if([NSURLConnection sendSynchronousRequest:theRequest returningResponse:nil error:nil] != nil)
	{
		NSLog(@"FTP OK");
		return TRUE;
	}
	else 
	{
		NSLog(@"Can't connect to FTP");
		return FALSE;
	}
}


- (void)checkStream {
	NSLog(@"%s", __FUNCTION__);
	if (!streamOpen && (self.networkStream != nil)) 
	{
		[self stopSendWithStatus:NSLocalizedString(@"Can't send over network", @"")];
	}
}

- (void)stopSendWithStatus:(NSString *)statusString {
	NSLog(@"%s : %@", __FUNCTION__, statusString);
	if (self.networkStream != nil) 
	{
		[self.networkStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		self.networkStream.delegate = nil;
		[self.networkStream close];
		self.networkStream = nil;
	}
	
	if (self.fileStream != nil) 
	{
		[self.fileStream close];
		self.fileStream = nil;
	}
	
	streamOpen = NO;
	if (statusString == nil) 
	{
		if(state == s_repair)
		{
			if(substate_repair == ss_repair_one)
			{
				substate_repair = ss_repair_two;
				
				assert(self.networkStream == nil);
				assert(self.fileStream == nil);
				streamOpen = NO;
				
				NSLog(@"Copy bootldr file\n");
				self.fileStream = [NSInputStream inputStreamWithFileAtPath:bootldrPath];
				assert(self.fileStream != nil);
				[self.fileStream open];
				
				NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, self.localIP, FTP_REPAIR_PORT, [NSString stringWithFormat:@"%@.bin", self.bootldrFileName]]];
				
				CFWriteStreamRef ftpStream = CFWriteStreamCreateWithFTPURL(NULL, (CFURLRef) url);
				assert(ftpStream != NULL);
				
				self.networkStream = (NSOutputStream *) ftpStream;
				self.networkStream.delegate = self;
				[self.networkStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
				[self.networkStream open];
				
				CFRelease(ftpStream);
				[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
				
			}
			else
			{
				NSLog(@"AR.Drone repaired successfully");
				// Send telnet commande
				[self stateUpdate:s_repair Result:r_pass Message:NSLocalizedString(@"AR.Drone repaired successfully\n", @"")];
				[self repair];
				[self droneConnected:self.droneFirmwareVersion];
			}
		}
		else 
		{
			substate = ss_user;
			[self stateUpdate:s_restart Result:r_progress Message:NSLocalizedString(@"Update file sent succesfully\nPlease restart the AR.Drone", @"")];
			[self performSelector:@selector(startCheckRestart) withObject:nil afterDelay:2];
		}
	} 
	else 
	{
		if (!end) 
		{
			[self stateUpdate:s_send Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", statusString, NSLocalizedString(@"Please close application", @"")]];
		}
	}
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
}

- (void)stopReceiveWithStatus:(NSString *)statusString {
	if (self.fileStream != nil) 
	{
		[self.fileStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		self.fileStream.delegate = nil;
		[self.fileStream close];
		self.fileStream = nil;
	}
	
	streamOpen = NO;
	if (statusString != nil) 
	{
		if (!end) 
		{
			[self stateUpdate:s_connect Result:r_fail Message:statusString];
		}
	}
	
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
}


	// Because buffer is declared as an array, you have to use a custom getter.  
	// A synthesised getter doesn't compile.
- (uint8_t *)buffer 
{
	return self->_buffer;
}

- (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)eventCode
{
	assert(aStream == self.networkStream || aStream == self.fileStream );
	switch (eventCode) {
		case NSStreamEventHasBytesAvailable: {
			NSInteger       bytesRead;
			uint8_t         tmpbuffer[32768];
			
				// Pull some data off the network.
			bytesRead = [self.fileStream read:tmpbuffer maxLength:sizeof(tmpbuffer)];
			if (bytesRead == -1) {
				[self stateUpdate:state Result:r_fail Message:NSLocalizedString(@"Cannot connect to AR.Drone", @"")];
			} else if (bytesRead == 0) {			
				[self.fileStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
				self.fileStream.delegate = nil;
				[self.fileStream close];
				self.fileStream = nil;
				streamOpen = NO;
				if(state != s_recheck)
					[self droneConnected:self.droneFirmwareVersion];
			} 
			else 
			{
				if(state == s_recheck)
				{
					self.errorLog = [[NSString alloc] initWithBytes:tmpbuffer length:bytesRead encoding:NSUTF8StringEncoding];
					self.errorLog = [self.errorLog stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];

					NSArray* log = [self.errorLog componentsSeparatedByString:@"="];
					if([log count] > 1)
					{
						self.errorLog = [log objectAtIndex:1];
						[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", NSLocalizedString(@"Update process failed", @""), self.errorLog]];
					}
					else if([log count] == 1)
					{
						self.errorLog = [log objectAtIndex:0];
						[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", NSLocalizedString(@"Update process failed", @""), self.errorLog]];						
					}
				}
				else
				{
					//read version number				
					self.droneFirmwareVersion = [[NSString alloc] initWithBytes:tmpbuffer length:bytesRead encoding:NSUTF8StringEncoding];
					self.droneFirmwareVersion = [self.droneFirmwareVersion stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];
				}
			}
			
		} break;
		case NSStreamEventHasSpaceAvailable:            
				// If we don't have any data buffered, go read the next chunk of data.
			if (self.bufferOffset == self.bufferLimit) {
				NSInteger bytesRead = [self.fileStream read:self.buffer maxLength:SEND_BUFFER_SIZE];                
				if (bytesRead == -1) {
					[self stopSendWithStatus:NSLocalizedString(@"Can't send over network", @"")];
				} else if (bytesRead == 0) {
					[self stopSendWithStatus:nil]; //sending finished, success
				} else {
					self.bufferOffset = 0;
					self.bufferLimit = bytesRead;
				}
			}
				// If we're not out of data completely, send the next chunk.
			if (self.bufferOffset != self.bufferLimit) {
				NSInteger bytesWritten = [self.networkStream write:&self.buffer[self.bufferOffset] maxLength:self.bufferLimit - self.bufferOffset];
				assert(bytesWritten != 0);
				if (bytesWritten == -1) {
					[self stopSendWithStatus:NSLocalizedString(@"Can't send over network", @"")];
				} else {
					self.bufferOffset += bytesWritten;
					sendedSize += bytesWritten;
					[sendProgressView setProgress:1.0 * sendedSize / firmwareSize];
				}
			}
			break;
		case NSStreamEventErrorOccurred:
			if(state == s_connect)
			{
				[self stopReceiveWithStatus:NSLocalizedString(@"Cannot connect to AR.Drone", @"")];
			}
			else
			{
				[self stopSendWithStatus:NSLocalizedString(@"Can't send over network", @"")];
			}
			break;
		case NSStreamEventOpenCompleted:
			streamOpen = YES;
			break;
		case NSStreamEventNone:
		case NSStreamEventEndEncountered:
			break;
				//        case NSStreamEventHasBytesAvailable:
		default:
			assert(NO); //should never happen for the output stream
			break;
	}
}

- (void)refresh:(unsigned int)frameCount
{
	
}

#pragma mark memo

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
}

- (void)dealloc {
	[self stopSendWithStatus:@""];
		//	[drone release];
	
	for (NSInteger i = 0; i < STEP_LINE_NR; i++) 
	{
		[stepLabel[i] release];
		[stepImageView[i] release];
	}
	
	[droneFirmwareVersion release];
	[stepIndicator release];
	[_networkStream release];
	[_fileStream release];
	[super dealloc];
}

@end
