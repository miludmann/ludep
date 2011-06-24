package command;

import tools.MessageComputer;

public class Interpretator{

	// ATTRIBUTES
	//-----------
	private NXT nxt;
	private MessageRegulator mr;
	
	// CONSTRUCTORS
	//-------------
	public Interpretator(NXT nxt) {
		setNxt(nxt);
		setMr(new MessageRegulator(this));
		getMr().start();
	}
	
	// METHODS
	//--------
	public void treatMessage(MessageComputer mc){
		getMr().getCm().incID();
		getMr().addMessage(mc);
	}

	// GETTERS - SETTERS
	//------------------
	public void setMr(MessageRegulator mr) {
		this.mr = mr;
	}

	public MessageRegulator getMr() {
		return mr;
	}

	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}

	public NXT getNxt() {
		return nxt;
	}

}
