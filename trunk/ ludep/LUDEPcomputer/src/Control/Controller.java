package Control;


import ch.aplu.xboxcontroller.XboxController;
import ch.aplu.xboxcontroller.XboxControllerAdapter;

import command.Computer;


public class Controller{

	// ATTRIBUTES
	//-----------
	private Computer comp;
	private XboxController xc;
	
	private ControllerRegulator cr;

	private double leftDirection, rightDirection;
	private double leftMagnitude, rightMagnitude;

	private double lastLD, lastLM, lastRD, lastRM;

	// CONSTRUCTORS
	//-------------
	public Controller(Computer c){
		setComp(c);
		setXc(new XboxController());
		setController();
		setCr(new ControllerRegulator(this));
	}

	// METHODS
	//--------
	public void setController(){
		setLeftDirection(0);
		setLeftMagnitude(0);
		setRightDirection(0);
		setRightMagnitude(0);

		setLastLM(0);
		setLastLD(0);
		setLastRM(0);
		setLastRD(0);

		if (!getXc().isConnected())
		{
			System.out.println("Xbox controller not connected.");
			xc.release();
			return;
		}


		xc.addXboxControllerListener(new XboxControllerAdapter()
		{
			public void leftThumbDirection(double direction) {
				controlLeftDirection(direction);
			}

			public void leftThumbMagnitude(double magnitude) {
				controlLeftMagnitude(100*magnitude);
			}

			public void rightThumbDirection(double direction) {
				controlRightDirection(direction);
			}

			public void rightThumbMagnitude(double magnitude) {
				controlRightMagnitude(100*magnitude);
			}

			public void buttonA(boolean pressed){
				if ( pressed )
					getComp().send("draw");
			}

			public void buttonB(boolean pressed){
				if ( pressed )
					getComp().send("stop");
			}

			public void buttonX(boolean pressed){
				if ( pressed )
					getComp().switchBrick();
			}

			public void buttonY(boolean pressed){
				if ( pressed )
					getComp().send("reg switch");
			}

			public void leftShoulder(boolean pressed){
				if ( pressed )
					getComp().send("+");
			}

			public void rightShoulder(boolean pressed){
				if ( pressed )
					getComp().send("+");
			}
		});
	}


	public double joyDistance(double a, double b){
		return Math.abs(a-b);
	}

	public void controlLeftMagnitude(double m){

		if ( m <= 25 ){
			if ( getLeftMagnitude() == 0 ) return;
			setLeftMagnitude(0);				
		}
		else
		{
			setLeftMagnitude(4*(m-25)/3);
		}
		// refreshDrivingParameters();
	}

	public void controlLeftDirection(double d){

		if ( joyDistance(d, getLeftDirection()) >= 5 )
		{
			setLeftDirection(d);
		}
		// refreshDrivingParameters();
	}

	public void controlRightMagnitude(double m){

		if ( m <= 25 ){
			if( getRightMagnitude() == 0 ) return;
			setRightMagnitude(0);				
		}
		else
		{
			setRightMagnitude(4*(m-25)/3);
		}
		// refreshDrivingParameters();
	}

	public void controlRightDirection(double d){

		if ( joyDistance(d, getRightDirection()) >= 5 )
		{
			setRightDirection(d);
		}
		// refreshDrivingParameters();
	}

	public boolean valueHaveChanged(){
		return
		getLastLD()!= getLeftDirection()
		|| getLastLM()!= getLeftMagnitude()
		|| getLastRD()!= getRightDirection()
		|| getLastRM()!= getRightMagnitude();
	}

	public void refreshDrivingParameters(){

		if ( ! valueHaveChanged() ) return;

		setLastLD(getLeftDirection());
		setLastLM(getLeftMagnitude());
		setLastRD(getRightDirection());
		setLastRM(getRightMagnitude());

		System.out.println(
				(int) getLastLD() + " " +
				(int) getLastLM() + " " +
				(int) (getLastRM()*Math.cos(2*Math.PI/360*(450-getLastRD())%360)));

		getComp().send("x " +
				(int) getLastLD() + " " +
				(int) getLastLM() + " " +
				(int) (getLastRM()*Math.cos(2*Math.PI/360*(450-getLastRD())%360)));
	}

	// GETTERS - SETTERS
	//------------------
	public void setXc(XboxController xc) {
		this.xc = xc;
	}

	public XboxController getXc() {
		return xc;
	}

	public void setComp(Computer comp) {
		this.comp = comp;
	}

	public Computer getComp() {
		return comp;
	}

	public double getLeftDirection() {
		return leftDirection;
	}

	public void setLeftDirection(double leftDirection) {
		this.leftDirection = leftDirection;
	}

	public double getRightDirection() {
		return rightDirection;
	}

	public void setRightDirection(double rightDirection) {
		this.rightDirection = rightDirection;
	}

	public double getLeftMagnitude() {
		return leftMagnitude;
	}

	public void setLeftMagnitude(double leftMagnitude) {
		this.leftMagnitude = leftMagnitude;
	}

	public double getRightMagnitude() {
		return rightMagnitude;
	}

	public void setRightMagnitude(double rightMagnitude) {
		this.rightMagnitude = rightMagnitude;
	}

	public double getLastLD() {
		return lastLD;
	}

	public void setLastLD(double lastLD) {
		this.lastLD = lastLD;
	}

	public double getLastLM() {
		return lastLM;
	}

	public void setLastLM(double lastLM) {
		this.lastLM = lastLM;
	}

	public double getLastRD() {
		return lastRD;
	}

	public void setLastRD(double lastRD) {
		this.lastRD = lastRD;
	}

	public double getLastRM() {
		return lastRM;
	}

	public void setLastRM(double lastRM) {
		this.lastRM = lastRM;
	}

	public void setCr(ControllerRegulator cr) {
		this.cr = cr;
	}

	public ControllerRegulator getCr() {
		return cr;
	}
}
