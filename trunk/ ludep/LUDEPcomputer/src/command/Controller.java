package command;

import ch.aplu.xboxcontroller.XboxController;
import ch.aplu.xboxcontroller.XboxControllerAdapter;

public class Controller{
	
	// ATTRIBUTES
	//-----------
	private Computer comp;
	private ControllerRegulator cr;
	private XboxController xc;

	private double leftDirection, rightDirection;
	private double leftMagnitude, rightMagnitude;
	
	// CONSTRUCTORS
	//-------------
	public Controller(Computer c){
		setComp(c);
		setXc(new XboxController());
		setController();
		
		setCr(new ControllerRegulator(this));
		getCr().start();
	}
	
	// METHODS
	//--------
	public void setController(){
		setLeftDirection(0);
		setLeftMagnitude(0);
		setRightDirection(0);
		setRightMagnitude(0);
		
		if (!getXc().isConnected())
		{
			System.out.println("Xbox controller not connected.");
			xc.release();
			return;
		}
		
		
		xc.addXboxControllerListener(new XboxControllerAdapter()
		{
			public void leftThumbDirection(double direction) {
				if ( joyDistance(direction, getLeftDirection()) > 10 ) 
					controlLeftDirection(direction);
								
			}

			public void leftThumbMagnitude(double magnitude) {
				if ( joyDistance(100*magnitude, getLeftMagnitude()) > 5 ) 
					controlLeftMagnitude(100*magnitude);
			}
			
			public void rightThumbDirection(double direction) {
				if ( joyDistance(direction, getRightDirection()) > 10 ) 
					controlRightDirection(direction);
			}

			public void rightThumbMagnitude(double magnitude) {
				if ( joyDistance(100*magnitude, getRightMagnitude()) > 5 ) 
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
	}

	public void controlLeftDirection(double d){

		if ( joyDistance(d, getLeftDirection()) >= 5 )
		{
			setLeftDirection(d);
		}
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
	}
	
	public void controlRightDirection(double d){

		if ( joyDistance(d, getRightDirection()) >= 5 )
		{
			setRightDirection(d);
		}
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

	public void setCr(ControllerRegulator cr) {
		this.cr = cr;
	}

	public ControllerRegulator getCr() {
		return cr;
	}
}
