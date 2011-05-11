package command;

import lejos.nxt.Motor;
import lejos.nxt.MotorPort;
import tools.MessageComputer;

/**
 * The class in charge to be in control of the motors.
 * Receives direction commands;
 * Allocate the right power for each motor.
 */
public class ControlMotor {
	
	// ATTRIBUTES
	//-----------

	/**
	 * Pointer for the NXT
	 */
	private NXT nxt;
	/**
	 * Power used for the motors
	 */
	private int powerA, powerB, powerC;
	/**
	 * Rotation added by the user besides the direction
	 */
	private int rotationPower;
	/**
	 * Angle sampled by the compass
	 */
	private int compassAngle;
	/**
	 * Angle used as a reference by the compass
	 */
	private int refAngle;
	/**
	 * Activation status of the compass
	 */
	private boolean isCompassActivated;
	/**
	 * Change the orientation of the angles
	 * (clockwise or counterclockwise)
	 */
	private boolean invertedMotors;

	
	
	// CONSTRUCTORS
	//-------------
	/**
	 * Constructor of the class
	 * @param nxt The next the motors are linked to
	 */
	public ControlMotor(NXT nxt){
		setNxt(nxt);
		
		Motor.A.regulateSpeed(true);
		Motor.B.regulateSpeed(true);
		Motor.C.regulateSpeed(true);
		
		setPowerA(0);
		setPowerB(0);
		setPowerC(0);
		
		setRotationPower(0);
		setCompassAngle(0);
		setRefAngle(0);
		setCompassActivated(false);
		setInvertedMotors(true);
	}

	// METHODS
	//--------
	/**
	 * Settles the power for each different motors 
	 * so as to move the bot within the right direction
	 * @param angle angle you want to move the bot
	 * @param power power desired for the bot's movement
	 */
	public void angleMotors(int angle, int power){
		
		int halfDisk, relAngle, factorPower;
		int powerA, powerB, powerC;
		
		powerA = 0;
		powerB = 0;
		powerC = 0;
		
		if ( isInvertedMotors() )
			angle = 180-angle;
		
		if ( isCompassActivated() )
			angle += getCompassAngle() + 360 - getRefAngle();
		
		angle %= 360;
		halfDisk = angle/180;
		angle %= 180;
						
		if ( angle >= 0 && angle < 60 ){
						
			relAngle = angle;
			factorPower = 10000*relAngle/60; 
			
			powerA += power*factorPower/10000;
			powerB -= power;
			powerC += power*(10000-factorPower)/10000;
			
		} else if ( angle >= 60 && angle < 120 ) {
						
			relAngle = angle - 60;
			factorPower = 10000*relAngle/60;
			
			powerA += power;
			powerB -= power*(10000-factorPower)/10000;
			powerC -= power*factorPower/10000;
			
		} else if ( angle >= 120 && angle < 180 ) {
					
			relAngle = angle - 120;
			factorPower = 10000*relAngle/60;
						
			powerA += power*(10000-factorPower)/10000;
			powerB += power*factorPower/10000;
			powerC -= power;
		}
		
		if ( halfDisk == 1 ){
			powerA *= -1;
			powerB *= -1;
			powerC *= -1;
		}
		
		setPowerA(powerA);
		setPowerB(powerB);
		setPowerC(powerC);
	}
	
	/**
	 * The function that take a message from the computer
	 * and analyse it so as to command the motors
	 * @param mc the message incoming from the computer
	 */
	public void commandMotors(MessageComputer mc){
		
		int nbFrag = mc.nbFragments();
		String si = null;
		
		for ( int i = 0; i < nbFrag; i++ )
		{
			si = mc.getFragment(i);
			
			if ( si.equalsIgnoreCase("g") && (nbFrag > i+2) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				
				relativeMove(i1, i2);
			}
			
			if ( si.equalsIgnoreCase("m") && (nbFrag > i+2) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				
				angleMotors(i1, i2);
			}
			
			if ( si.indexOf("r") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				setRotationPower(i1);
			}
			
			if ( si.equalsIgnoreCase("x") && (nbFrag > i+3) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				int i3 = Integer.parseInt(mc.getFragment(i+3));
				angleMotors(i1, i2);
				setRotationPower(i3);
			}	
			
			if ( si.indexOf("a") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				setPowerA(i1);
			}
			
			if ( si.indexOf("b") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				setPowerB(i1);
			}
			
			if ( si.indexOf("c") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				setPowerC(i1);
			}
			
			if ( si.equalsIgnoreCase("comp") )
			{
				setCompassActivated(!isCompassActivated());
				getNxt().getCl().sendMessage("Compass Use: " + isCompassActivated());
			}
			
			if ( si.equalsIgnoreCase("ref") )
			{
				setRefAngle(getCompassAngle());
				getNxt().getCl().sendMessage("New ref angle: " + getRefAngle());
			}

			if ( si.equalsIgnoreCase("s") )
			{
				setPowerA(0);
				setPowerB(0);
				setPowerC(0);
				setRotationPower(0);
			}
			
			if ( si.equalsIgnoreCase("test") )
			{
				test();
			}
		}
		
		refreshMotors();
		
		int mota, motb, motc;
		
		mota = Motor.A.getTachoCount();
		motb = Motor.B.getTachoCount();
		motc = Motor.C.getTachoCount();
		
		this.getNxt().getCl().sendMessage("MOT " + mota + " " + motb + " " + motc);
		
	}
	
	
	/**
	 * Test function
	 * Call it by sending the string "test" to the brick
	 */
	public void test()
	{
		this.getNxt().getCl().sendMessage("Test running");
	}
	
	/**
	 * Relative movement with angle and direction
	 * @param angle
	 * @param distance
	 */
	public void relativeMove(int angle, int distance)
	{
		int refmota, refmotb, refmotc;
		int mota, motb, motc;
		
		refmota = Motor.A.getTachoCount();
		refmotb = Motor.B.getTachoCount();
		refmotc = Motor.C.getTachoCount();
		
		while ( true )
		{
			mota = Motor.A.getTachoCount();
			motb = Motor.B.getTachoCount();
			motc = Motor.C.getTachoCount();
		}
	}
		
	
	/**
	 * Refresh the parameters given to the motors
	 */
	public void refreshMotors()
	{
		int rot = getRotationPower();
		
		int pa = getPowerA()+rot;
		int pb = getPowerB()+rot;
		int pc = getPowerC()+rot;
		
		int maxp = Math.max(Math.max(Math.abs(pa), Math.abs(pb)), Math.abs(pc));
		
		if ( maxp > 100 )
		{
			pa = 100*pa/maxp;
			pb = 100*pb/maxp;
			pc = 100*pc/maxp;
		}
		
		MotorPort.A.controlMotor(pa, 1);
		MotorPort.B.controlMotor(pb, 1);
		MotorPort.C.controlMotor(pc, 1);
	}
	
	// GETTERS - SETTERS
	//------------------

	public NXT getNxt() {
		return nxt;
	}

	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}

	public int getPowerA() {
		return powerA;
	}

	public void setPowerA(int powerA) {
		this.powerA = powerA;
	}

	public int getPowerB() {
		return powerB;
	}

	public void setPowerB(int powerB) {
		this.powerB = powerB;
	}

	public int getPowerC() {
		return powerC;
	}

	public void setPowerC(int powerC) {
		this.powerC = powerC;
	}

	public int getRotationPower() {
		return rotationPower;
	}

	public void setRotationPower(int rotationPower) {
		this.rotationPower = rotationPower;
	}

	public int getCompassAngle() {
		return compassAngle;
	}

	public void setCompassAngle(int compassAngle) {
		this.compassAngle = compassAngle;
	}

	public int getRefAngle() {
		return refAngle;
	}

	public void setRefAngle(int refAngle) {
		this.refAngle = refAngle;
	}

	public boolean isCompassActivated() {
		return isCompassActivated;
	}

	public void setCompassActivated(boolean isCompassActivated) {
		this.isCompassActivated = isCompassActivated;
	}

	public void setInvertedMotors(boolean invertedMotors) {
		this.invertedMotors = invertedMotors;
	}

	public boolean isInvertedMotors() {
		return invertedMotors;
	}
}
