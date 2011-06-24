package command;

import java.io.IOException;

import lejos.nxt.Button;
import tools.MessageComputer;

public class NXT{
	
	// ATTRIBUTES
	//-----------
	private int id;
	private ComputerLink cl;
	private Interpretator interp;
	private boolean isRunning;
	
	// CONSTRUCTORS
	//-------------
	public NXT(){
		setId(-1);
		setCl(new ComputerLink(this));
		setInterp(new Interpretator(this));
		setRunning(true);
	}
	
	// METHODS
	//--------
	public void interpretMSG(String s){
				
		MessageComputer mc = new MessageComputer(s);
		
		if ( mc.getFragment(0).equalsIgnoreCase("ID") && getId() < 0 ) {
			setId(Integer.parseInt(mc.getFragment(1)));
			getCl().sendMessage("ID " + getId());
		}
		
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

	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return id;
	}
}