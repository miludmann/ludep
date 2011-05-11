package command;

import java.io.IOException;

import lejos.nxt.Button;
import lejos.nxt.SensorPort;
import lejos.nxt.addon.CompassSensor;
import tools.MessageComputer;

public class NXT{
	
	// ATTRIBUTES
	//-----------
	private ComputerLink cl;
	private ControlMotor cm;
	private boolean isCalibrating;
	private boolean isRunning;
	
	// CONSTRUCTORS
	//-------------
	public NXT(){
		setCl(new ComputerLink(this));
		setCm(new ControlMotor(this));
		setCalibrating(false);
		setRunning(true);
	}
	
	// METHODS
	//--------
	public void interpretMSG(String s){
		
		MessageComputer mc = new MessageComputer(s);
		
		if ( mc.getFragment(0).equalsIgnoreCase("cal") )
		{
			setCalibrating(!this.isCalibrating());
		} else if ( mc.getFragment(0).equalsIgnoreCase("stop") ) {
			setCalibrating(false);
			setRunning(false);
		}
		
		getCm().commandMotors(mc);
	}
	
	// GETTERS - SETTERS
	//------------------
	public void setCl(ComputerLink cl) {
		this.cl = cl;
	}

	public ComputerLink getCl() {
		return cl;
	}

	public void setCm(ControlMotor cm) {
		this.cm = cm;
	}

	public ControlMotor getCm() {
		return cm;
	}
	
	public boolean isCalibrating() {
		return isCalibrating;
	}

	public void setCalibrating(boolean isCalibrating) {
		this.isCalibrating = isCalibrating;
	}

	public boolean isRunning() {
		return isRunning;
	}

	public void setRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}
	
	public void calibrateCompass(CompassSensor cs)
	{
		if ( this.getCm().isCompassActivated() )
		{
			getCl().sendMessage("Calibration started...");
			cs.startCalibration();
			
			while(!Button.ENTER.isPressed()
					&& isCalibrating()
					&& isRunning()){}
			
			cs.stopCalibration();
			getCl().sendMessage("Calibration finished...");
		}
	}

	// MAIN
	//-----
	public static void main(String[] args) throws IOException {
		
		NXT brick = new NXT();
		int tmp = 0;
		
		CompassSensor cs = new CompassSensor(SensorPort.S2);
				
		while(!Button.ESCAPE.isPressed() && brick.isRunning()){
			
			if ( brick.isCalibrating() )
				brick.calibrateCompass(cs);
									
			tmp = (int) cs.getDegreesCartesian();
			
			brick.getCm().setCompassAngle(tmp);
		}
	}

}