package command;

import ch.aplu.xboxcontroller.XboxController;
import ch.aplu.xboxcontroller.XboxControllerAdapter;

public class Controller{
	
	// ATTRIBUTES
	//-----------
	private Computer comp;
	private XboxController xc;
	private CommandsRegulator cr;

	private double leftDirection, rightDirection;
	private double leftMagnitude, rightMagnitude;
	
	// CONSTRUCTORS
	//-------------
	public Controller(Computer c){
		setComp(c);
		setXc(new XboxController());
		setController();
		
		setCr(new CommandsRegulator(this));
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
					getComp().getBrick().sendMessage("cal");
			}
			
			public void buttonB(boolean pressed){
				if ( pressed )
					getComp().getBrick().sendMessage("stop");
			}
			
			public void buttonX(boolean pressed){
				if ( pressed )
					getComp().getBrick().sendMessage("ref");
			}
			
			public void buttonY(boolean pressed){
				if ( pressed )
					getComp().getBrick().sendMessage("comp");
				// activate/deactivate compass
			}
		});
	}
	
	
	public double joyDistance(double a, double b){
		return Math.abs(a-b);
	}
	
	/*
	public void controlLeftMagnitude(double m){
		double tmp = Math.abs(m - getLeftMagnitude());
		
		if ( m <= 25 ){
			if ( getLeftMagnitude() != 0 )
			{
				inDeadZoneLeft();
				setLeftMagnitude(0);				
			}
		}
		else
		{
			if ( tmp >= 5 )
			{
				setLeftMagnitude(4*(m-25)/3);
				refreshDrivingParameters();
			}
		}
	}
	*/
	public void controlLeftMagnitude(double m){
		
		if ( m <= 25 ){
			setLeftMagnitude(0);				
		}
		else
		{
			setLeftMagnitude(4*(m-25)/3);
		}
		
		refreshDrivingParameters();
	}
	
	/*
	public void controlLeftDirection(double d){
		double tmp = Math.abs(d-getLeftDirection());
		
		if ( tmp >= 5 )
		{
			setLeftDirection(d);
			refreshDrivingParameters();
		}
	}
	*/
	public void controlLeftDirection(double d){

		setLeftDirection(d);
		refreshDrivingParameters();
	}
	
	/*
	public void controlRightMagnitude(double m){
		double tmp = Math.abs(m - getRightMagnitude());
		
		if ( m <= 25 ){
			if ( getRightMagnitude() != 0 )
			{
				inDeadZoneRight();
				setRightMagnitude(0);				
			}
		}
		else
		{
			if ( tmp >= 10 )
			{
				setRightMagnitude(4*(m-25)/3);
				refreshDrivingParameters();
			}
		}
	}
	*/
	
	public void controlRightMagnitude(double m){
		
		if ( m <= 25 ){
			setRightMagnitude(0);				
		}
		else
		{
			setRightMagnitude(4*(m-25)/3);
		}
		
		refreshDrivingParameters();
	}
	
	/*
	public void controlRightDirection(double d){
		double tmp = Math.abs(d-getRightDirection());
		
		if ( tmp >= 20 )
		{
			setRightDirection(d);
			refreshDrivingParameters();
		}
	}
	*/
	
	public void controlRightDirection(double d){

		setRightDirection(d);
		refreshDrivingParameters();
	}
	
	/*
	public void inDeadZoneLeft(){
		if ( getLeftMagnitude() != 0 )
		{
			getComp().getBrick().sendDirections(0, 0);
		}
	}
	
	public void inDeadZoneRight(){
		if ( getRightMagnitude() != 0 )
		{
			getComp().getBrick().sendRotation(0);
		}
	}
	*/
	
	public void refreshDrivingParameters(){
		/*
		getComp().getBrick().sendCommand((int) getLeftDirection(),
				 (int) getLeftMagnitude(),
				 (int) (getRightMagnitude()*Math.cos(2*Math.PI/360*(450-getRightDirection())%360)));
		*/
		
		getCr().refreshData((int) getLeftDirection(),
				 (int) getLeftMagnitude(),
				 (int) (getRightMagnitude()*Math.cos(2*Math.PI/360*(450-getRightDirection())%360)));
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

	public void setCr(CommandsRegulator cr) {
		this.cr = cr;
	}

	public CommandsRegulator getCr() {
		return cr;
	}
}