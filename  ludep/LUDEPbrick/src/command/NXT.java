package command;

import java.io.IOException;

import lejos.nxt.Button;
import tools.MessageComputer;

public class NXT{
	
	// ATTRIBUTES
	//-----------
	private ComputerLink cl;
	private ControlMotor cm;
	private Interpretator interp;
	private boolean isRunning;
	
	// CONSTRUCTORS
	//-------------
	public NXT(){
		setCl(new ComputerLink(this));
		setCm(new ControlMotor(this));
		setInterp(new Interpretator(getCm()));
		setRunning(true);
	}
	
	// METHODS
	//--------
	public void interpretMSG(String s){
				
		MessageComputer mc = new MessageComputer(s);
		
		if ( mc.getFragment(0).equalsIgnoreCase("stop") ) {
			setRunning(false);
		}
		
		getInterp().treatMessage(mc);
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

	public boolean isRunning() {
		return isRunning;
	}

	public void setRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public void setInterp(Interpretator interp) {
		this.interp = interp;
	}

	public Interpretator getInterp() {
		return interp;
	}

	// MAIN
	//-----
	public static void main(String[] args) throws IOException {
		
		NXT brick = new NXT();
		
		while(!Button.ESCAPE.isPressed() && brick.isRunning()){
		}
		
		System.exit(1);
	}
}