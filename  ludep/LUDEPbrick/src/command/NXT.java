package command;

import java.io.IOException;

import lejos.nxt.Button;
import lejos.nxt.addon.CompassSensor;
import tools.MessageComputer;

public class NXT{
	
	// ATTRIBUTES
	//-----------
	private int id;
	private ComputerLink cl;
	private MessageRegulator mr;
	private CompassSensor cs;
	private Pulsor pulsor;
	private boolean isRunning;
	
	// CONSTRUCTORS
	//-------------
	public NXT(){
		setId(-1);
		setCl(new ComputerLink(this));
		setMr(new MessageRegulator(this));
		getMr().start();
		setPulsor(new Pulsor(this));
		setRunning(true);
		
		// setCs(new CompassSensor(SensorPort.S1));
	}
	
	// METHODS
	//--------
	public void interpretMSG(String s){
				
		MessageComputer mc = new MessageComputer(s);
		
		if ( mc.getFragment(0).equalsIgnoreCase("ID") && getId() < 0 ) {
			setId(Integer.parseInt(mc.getFragment(1)));
			getCl().sendMessage("Brick connected");
		}
		
		if ( mc.getFragment(0).equalsIgnoreCase("stop") ) {
			setRunning(false);
		}
		
		getMr().treatMessage(mc);
	}
	
	// GETTERS - SETTERS
	//------------------
	public void setCl(ComputerLink cl) {
		this.cl = cl;
	}

	public ComputerLink getCl() {
		return cl;
	}

	public boolean isRunning() {
		return isRunning;
	}

	public void setRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public MessageRegulator getMr() {
		return mr;
	}

	public void setMr(MessageRegulator mr) {
		this.mr = mr;
	}

	// MAIN
	//-----
	public static void main(String[] args) throws IOException {
		
		NXT brick = new NXT();
		
		while(!Button.ESCAPE.isPressed() && brick.isRunning()){
		}
		
		System.exit(1);
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return id;
	}

	public void setCs(CompassSensor cs) {
		this.cs = cs;
	}

	public CompassSensor getCs() {
		return cs;
	}
	
	public void setPulsor(Pulsor pulsor) {
		this.pulsor = pulsor;
	}

	public Pulsor getPulsor() {
		return pulsor;
	}
}