package command;

import java.io.IOException;

import lejos.nxt.Button;
import lejos.nxt.MotorPort;
import tools.MessageComputer;

public class NXT{
	
	// ATTRIBUTES
	//-----------
	private ComputerLink cl;
	private ControlMotor cm;
	
	// CONSTRUCTORS
	//-------------
	public NXT(){
		setCl(new ComputerLink(this));
		setCm(new ControlMotor(this));
	}
	
	// METHODS
	//--------
	public void interpretMSG(String s){
		
		MessageComputer mc = new MessageComputer(s);
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
	
	// MAIN
	//-----
	public static void main(String[] args) throws IOException {
		
		NXT brick = new NXT();
				
		while(!Button.ESCAPE.isPressed()){
		
		}
	}

}
