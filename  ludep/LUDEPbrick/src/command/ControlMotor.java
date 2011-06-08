package command;

import lejos.nxt.Motor;
import lejos.nxt.MotorPort;

/**
 * The class in charge to be in control of the motors.
 * Receives direction commands;
 * Allocate the right power for each motor.
 */
public class ControlMotor extends Thread{
	
	// ATTRIBUTES
	//-----------
	private NXT nxt; ///< Pointer for the NXT
	private double powerA, powerB, powerC; ///< Power used for the motors
	private double rotationPower; ///< Rotation added by the user besides the direction
	private double wheelDiameter; ///< Diameter of the wheels in millimeters
	private boolean regulateSpeed; ///< regulation of the motors' speed
	private int idCmd; ///< id of a cmd (used for multi cmds)
	private double factorP; ///< p factor for the pid
	private double factorI; ///< i factor for the pid
	
	// CONSTRUCTORS
	//-------------
	/**
	 * Constructor of the class
	 * @param nxt The next the motors are linked to
	 */
	public ControlMotor(NXT nxt){
		setNxt(nxt);
		
		// set regulateMotor to true for each motor
		changeMotorsRegulation(true);
		
		setPowerA(0);
		setPowerB(0);
		setPowerC(0);
		
		setRotationPower(0);
		setWheelDiameter(62);
		setIdCmd(0);
		setFactorP(0);
		setFactorI(0);
	}

	// METHODS
	//--------
	public void run()
	{
		while (true) {}
	}
	
	/**
	 * Move the robot with a certain angle and length
	 * @param angle
	 * @param length
	 */
	public void moveAngLen(int angle, int length)
	{
		sendMSG("Start cmd " + getIdCmd());
		
		int idCurrentCmd = getIdCmd();
		
		angle = (angle%360+360)%360;
		
		// Dealing with 90 and 270 is not compatible with our code
		// We could make a 'if' but it would double the code for just
		// a minor matter. So we're using a 'dirty' way as you may say
		if ( angle == 90 || angle == 270 )
			angle++;
		
		double dir = angle*2*Math.PI/360;
		
		double dx = length*Math.cos(dir);
		double dy = length*Math.sin(dir);
		
		double powA, powB, powC, maxPow, rotA, rotB, rotC;
		
		double angleA = Motor.A.getTachoCount();
		double angleB = Motor.B.getTachoCount();
		double angleC = Motor.C.getTachoCount();
		
		powA = 2*dx/3;
		powB = -(dy/Math.sqrt(3))-(dx/3);
		powC = (dy/Math.sqrt(3))-(dx/3);
		
		maxPow = Math.max(Math.abs(powA), Math.max(Math.abs(powB), Math.abs(powC)));
		
		powA *= 100/maxPow;
		powB *= 100/maxPow;
		powC *= 100/maxPow;
		
		rotA = 360*dx/(getWheelDiameter()*Math.PI);
		rotB = powB/powA*rotA;
		rotC = powC/powA*rotA;
		
		double bufferA1 = Motor.A.getTachoCount() - (angleA + rotA);
		double bufferB1 = Motor.B.getTachoCount() - (angleB + rotB);
		double bufferC1 = Motor.C.getTachoCount() - (angleC + rotC);
		
		double signA = Math.signum(bufferA1);
		double signB = Math.signum(bufferB1);
		double signC = Math.signum(bufferC1);
			
		bufferA1 = Math.abs(bufferA1);
		bufferB1 = Math.abs(bufferB1);
		bufferC1 = Math.abs(bufferC1);
		
		/*
		double bufferA2 = bufferA1;
		double bufferB2 = bufferB1;
		double bufferC2 = bufferC1;
		*/
		
		double initTime, endTime, nbIter;
				
		double noRot, noRotInt;
		
		// When two of those flags goes to false, we stop the movement
		boolean flagA = (bufferA1 != 0);
		boolean flagB = (bufferB1 != 0);
		boolean flagC = (bufferA1 != 0);
		
		int paFinal, pbFinal, pcFinal;

		paFinal = (int) powA;
		pbFinal = (int) powB;
		pcFinal = (int) powC;
		
		// Checking if the sum of the powers are equal to zero
		// If not, there might be some rotation, so we correct it
		if ( paFinal + pbFinal + pcFinal != 0){
			
			if ( Math.abs(paFinal) < Math.abs(pbFinal) 
					&& Math.abs(paFinal) < Math.abs(pcFinal) )
			{
				paFinal = (int) Math.signum(paFinal)*(Math.abs(paFinal)+1);
			}
			
			if ( Math.abs(pbFinal) < Math.abs(pcFinal) 
					&& Math.abs(pbFinal) < Math.abs(paFinal) )
			{
				pbFinal = (int) Math.signum(pbFinal)*(Math.abs(pbFinal)+1);
			}
			
			if ( Math.abs(pcFinal) < Math.abs(pbFinal) 
					&& Math.abs(pcFinal) < Math.abs(paFinal) )
			{
				pcFinal = (int) Math.signum(pcFinal)*(Math.abs(pcFinal)+1);
			}
		}

		setPowerA(paFinal);
		setPowerB(pbFinal);
		setPowerC(pcFinal);
		
		noRotInt = 0;
		
		initTime = System.currentTimeMillis();
		nbIter = 0;
		
		sendMSG("Powers "
				+ getPowerA() + " "
				+ getPowerB() + " "
				+ getPowerC());
		
		while ( (flagA && flagB) || (flagA && flagC) || (flagB && flagC) )
		{
			if ( idCurrentCmd != getIdCmd() )
			{
				this.getNxt().getCl().sendMessage("Aborted cmd " + idCurrentCmd);
				
				setPowerA(0);
				setPowerB(0);
				setPowerC(0);
				refreshMotors();
				
				return;
			}
			nbIter++;
			
			noRot = Motor.A.getTachoCount()-angleA +
					Motor.B.getTachoCount()-angleB +
					Motor.C.getTachoCount()-angleC;
			
			noRotInt += noRot;
			
			setRotationPower((int) (-noRot*getFactorP()-noRotInt*getFactorI()));
			
			bufferA1 = Motor.A.getTachoCount() - (angleA + rotA);
			bufferB1 = Motor.B.getTachoCount() - (angleB + rotB);
			bufferC1 = Motor.C.getTachoCount() - (angleC + rotC);
			
			if ( Math.abs(bufferA1) <= 2 || Math.signum(bufferA1)*signA < 0 )
			{
				flagA = false;
			}
			
			if ( Math.abs(bufferB1) <= 2 || Math.signum(bufferB1)*signB < 0 )
			{
				flagB = false;
			}
			
			if ( Math.abs(bufferC1) <= 2 || Math.signum(bufferC1)*signC < 0 )
			{
				flagC = false;
			}
				
			bufferA1 = Math.abs(bufferA1);
			bufferB1 = Math.abs(bufferB1);
			bufferC1 = Math.abs(bufferC1);
			
			
			refreshMotors();
			
			/*
			this.getNxt().getCl().sendMessage(
					"buffers "
					+ (int) (1000*bufferA1/bufferA2) + " "
					+ (int) (1000*bufferB1/bufferB2) + " "
					+ (int) (1000*bufferC1/bufferC2) + " "
					+ (int) bufferA1 + " "
					+ (int) bufferA2 + " "
					+ flagA + " "
					+ (int) bufferB1 + " "
					+ (int) bufferB2 + " "
					+ flagB + " "
					+ (int) bufferC1 + " "
					+ (int) bufferC2 + " "
					+ flagC + " "
					+ "SumRot " + (int) noRot + " "
					+ "SumRotInt " + (int) noRotInt);
			*/
			/*
			this.getNxt().getCl().sendMessage(
					"Motors% "
					+ (int) (1000*(Motor.A.getTachoCount()-angleA)/rotA) + " "
					+ (int) (1000*(Motor.B.getTachoCount()-angleB)/rotB) + " "
					+ (int) (1000*(Motor.C.getTachoCount()-angleC)/rotC) + " "
					+ (int) bufferA2 + " "
					+ (int) bufferB2 + " "
					+ (int) bufferC2 );
			*/
		}
		
		endTime = System.currentTimeMillis();
		this.getNxt().getCl().sendMessage("DT " + (endTime-initTime)/nbIter);
		
		setPowerA(0);
		setPowerB(0);
		setPowerC(0);
		setRotationPower(0);
		
		refreshMotors();
		
		this.getNxt().getCl().sendMessage("Finished cmd " + idCurrentCmd);
		incID();
	}
	
	/**
	 * Move the robot from (xInit, yInit) to (xEnd, yEnd)
	 * supposing that the initial position of the bot is (0,0)
	 * @param xEnd arrival abscissa
	 * @param yEnd arrival ordinate
	 */
	public void moveCart(int xEnd, int yEnd)
	{
		int length, angle;
		
		length = (int) Math.sqrt(xEnd*xEnd+yEnd*yEnd);
		angle = (int) (180/Math.PI*Math.atan2(yEnd, xEnd));
				
		moveAngLen(angle, length);
	}
		
	/**
	 * Refresh the parameters given to the motors
	 */
	public void refreshMotors()
	{
		double rot = getRotationPower();
		
		double pa = getPowerA()+rot;
		double pb = getPowerB()+rot;
		double pc = getPowerC()+rot;
		
		double maxp = Math.max(Math.max(Math.abs(pa), Math.abs(pb)), Math.abs(pc));
		
		if ( maxp > 100 )
		{
			pa = 100*pa/maxp;
			pb = 100*pb/maxp;
			pc = 100*pc/maxp;
		}
				
		MotorPort.A.controlMotor((int) pa, 1);
		MotorPort.B.controlMotor((int) pb, 1);
		MotorPort.C.controlMotor((int) pc, 1);
	}

	public void changeMotorsRegulation()
	{
		changeMotorsRegulation(!isRegulateSpeed());
	}
	
	public void changeMotorsRegulation(boolean b)
	{
		setRegulateSpeed(b);
		
		Motor.A.regulateSpeed(b);
		Motor.B.regulateSpeed(b);
		Motor.C.regulateSpeed(b);
	}
	
	public void drawRectangle()
	{
		moveCart(500,500);
		moveCart(-500, 500);
		moveCart(-500, -500);
		moveCart(500, -500);
	}
	
	public void incID()
	{
		setIdCmd(getIdCmd()+1);
	}
	
	public void sendMSG(String s)
	{
		this.getNxt().getCl().sendMessage(s);
	}
	
	// GETTERS - SETTERS
	//------------------
	public NXT getNxt() {
		return nxt;
	}

	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}

	public double getPowerA() {
		return powerA;
	}

	public void setPowerA(double powerA) {
		this.powerA = powerA;
	}

	public double getPowerB() {
		return powerB;
	}

	public void setPowerB(double powerB) {
		this.powerB = powerB;
	}

	public double getPowerC() {
		return powerC;
	}

	public void setPowerC(double powerC) {
		this.powerC = powerC;
	}

	public double getRotationPower() {
		return rotationPower;
	}

	public void setRotationPower(double rotationPower) {
		this.rotationPower = rotationPower;
	}

	public void setWheelDiameter(double wheelDiameter) {
		this.wheelDiameter = wheelDiameter;
	}

	public double getWheelDiameter() {
		return wheelDiameter;
	}

	public void setRegulateSpeed(boolean regulateSpeed) {
		this.regulateSpeed = regulateSpeed;
	}

	public boolean isRegulateSpeed() {
		return regulateSpeed;
	}

	public void setIdCmd(int idCmd) {
		this.idCmd = idCmd;
	}

	public int getIdCmd() {
		return idCmd;
	}

	public void setFactorP(double factorP) {
		this.factorP = factorP;
	}

	public double getFactorP() {
		return factorP;
	}

	public void setFactorI(double factorI) {
		this.factorI = factorI;
	}

	public double getFactorI() {
		return factorI;
	}
}
