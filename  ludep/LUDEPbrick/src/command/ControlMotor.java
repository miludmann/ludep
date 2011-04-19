package command;

import lejos.nxt.Motor;
import lejos.nxt.MotorPort;
import tools.MessageComputer;


public class ControlMotor {
	
	// ATTRIBUTES
	//-----------

	private NXT nxt;
	
	private int powerA, powerB, powerC;
	private int rotationPower;
	private int compassAngle;
	private int refAngle;
	private boolean isCompassActivated;
	
	/*
	* power - power from 0-100
	* mode - 1=forward, 2=backward, 3=stop, 4=float
	*/
	
	// CONSTRUCTORS
	//-------------

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
	}

	// METHODS
	//--------
	
	
	public void angleMotors(int angle, int power){
		
		int halfDisk, relAngle, factorPower;
		int powerA, powerB, powerC;
		
		powerA = 0;
		powerB = 0;
		powerC = 0;
		
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
	
	public void commandMotors(MessageComputer mc){
		
		int nbFrag = mc.nbFragments();
		String si = null;
		
		for ( int i = 0; i < nbFrag; i++ )
		{
			si = mc.getFragment(i);
			
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
		}
		refreshMotors("abc");
	}
	
	public void refreshMotors(String s)
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
		
		if ( s.indexOf("a")>=0 )
			MotorPort.A.controlMotor(pa, 1);
		
		if ( s.indexOf("b")>=0 )
			MotorPort.B.controlMotor(pb, 1);
		
		if ( s.indexOf("c")>=0 )
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
}
