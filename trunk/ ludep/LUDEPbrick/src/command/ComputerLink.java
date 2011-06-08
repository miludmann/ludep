package command;

import Networking.LIMessage;
import Networking.LIMessageType;
import Networking.MessageFramework;
import Networking.MessageListenerInterface;

public class ComputerLink implements MessageListenerInterface{
	
	// ATTRIBUTES
	//-----------
	private MessageFramework mfw = MessageFramework.getInstance();
	private NXT nxt;

	// CONSTRUCTORS
	//-------------
	
	public ComputerLink(){
		setMfw(MessageFramework.getInstance());
	    getMfw().addMessageListener(this);
	    getMfw().StartListen();
	}
	
	public ComputerLink(NXT nxt) {
		this();
		setNxt(nxt);
	}


	// METHODS
	//--------
	public void receivedNewMessage(LIMessage msg) {
		getNxt().interpretMSG(msg.getPayload());
	}

	public void sendMessage(String s){
		getMfw().SendMessage(new LIMessage(LIMessageType.Command, s));
	}
	
	// GETTERS - SETTERS
	//------------------
	public void setMfw(MessageFramework mfw) {
		this.mfw = mfw;
	}

	public MessageFramework getMfw() {
		return mfw;
	}

	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}

	public NXT getNxt() {
		return nxt;
	}
}
