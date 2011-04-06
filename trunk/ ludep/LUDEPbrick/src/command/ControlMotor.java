package command;

import tools.MessageComputer;
import lejos.nxt.MotorPort;


public class ControlMotor {
	
	// ATTRIBUTES
	//-----------

	private NXT nxt;
	private int powerA, powerB, powerC;
	private int modeA, modeB, modeC;
	
	/*
	* power - power from 0-100
	* mode - 1=forward, 2=backward, 3=stop, 4=float
	*/
	
	// CONSTRUCTORS
	//-------------

	public ControlMotor(NXT nxt){
		setNxt(nxt);
		
		setModeA(3);
		setModeB(3);
		setModeC(3);
		
		setPowerA(0);
		setPowerB(0);
		setPowerC(0);
	}

	// METHODS
	//--------
	
	public void commandMotors(MessageComputer mc){
		
		String s0 = mc.getFragment(0);
		
		if(mc.nbFragments() == 2){
			
			int i1 = Integer.parseInt(mc.getFragment(1));
						
			if ( s0.indexOf("A") >= 0 )
				motA(i1, 2);
			
			if ( s0.indexOf("B") >= 0 )
				motB(i1, 2);
			
			if ( s0.indexOf("C") >= 0 )
				motC(i1, 2);
						
			if ( s0.indexOf("a") >= 0 )
				motA(i1, 1);
			
			if ( s0.indexOf("b") >= 0 )
				motB(i1, 1);
			
			if ( s0.indexOf("c") >= 0 )
				motC(i1, 1);
		}
		
		if(mc.nbFragments() == 1){
				
			if ( s0.equalsIgnoreCase("s") ){
				motA(0, 3);
				motB(0, 3);
				motC(0, 3);
			}
			
			if ( s0.equalsIgnoreCase("f") ){
				motA(0, 4);
				motB(0, 4);
				motC(0, 4);
			}
			

		}
	}
	
	public void motA(int power, int mode){
		setPowerA(power);
		setModeA(mode);
		refreshA();	
	}

	public void motB(int power, int mode){
		setPowerB(power);
		setModeB(mode);
		refreshB();
	}

	public void motC(int power, int mode){
		setPowerC(power);
		setModeC(mode);
		refreshC();
	}
	
	public void refreshA(){
		MotorPort.A.controlMotor(getPowerA(), getModeA());
	}

	public void refreshB(){
		MotorPort.B.controlMotor(getPowerB(), getModeB());
	}

	public void refreshC(){
		MotorPort.C.controlMotor(getPowerC(), getModeC());
	}

	
	
	// GETTERS - SETTERS
	//------------------
	
	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}

	public NXT getNxt() {
		return nxt;
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

	public int getModeA() {
		return modeA;
	}

	public void setModeA(int modeA) {
		this.modeA = modeA;
	}

	public int getModeB() {
		return modeB;
	}

	public void setModeB(int modeB) {
		this.modeB = modeB;
	}

	public int getModeC() {
		return modeC;
	}

	public void setModeC(int modeC) {
		this.modeC = modeC;
	}
}
