package command;

import tools.MessageComputer;

public class Interpretator{

	// ATTRIBUTES
	//-----------
	private ControlMotor cm;
	private MessageRegulator mr;
	
	// CONSTRUCTORS
	//-------------
	public Interpretator(ControlMotor cm) {
		setCm(cm);
		setMr(new MessageRegulator(getCm()));
		getMr().start();
	}
	
	// METHODS
	//--------
	public void treatMessage(MessageComputer mc){
		getCm().incID();
		getMr().addMessage(mc);
	}

	public void sendMessageCmp(String s){
		getCm().getNxt().getCl().sendMessage(s);
	}

	// GETTERS - SETTERS
	//------------------
	public void setCm(ControlMotor cm) {
		this.cm = cm;
	}

	public ControlMotor getCm() {
		return cm;
	}

	public void setMr(MessageRegulator mr) {
		this.mr = mr;
	}

	public MessageRegulator getMr() {
		return mr;
	}

}
