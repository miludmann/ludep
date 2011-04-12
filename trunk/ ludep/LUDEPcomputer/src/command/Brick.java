package command;

import lejos.pc.comm.NXTCommFactory;
import lejos.pc.comm.NXTInfo;
import MessageComponent.LIMessage;
import MessageComponent.LIMessageType;
import MessageComponent.MessageFramework;
import MessageComponent.MessageListenerInterface;

public class Brick implements MessageListenerInterface {
	
	// ATTRIBUTES
	//-----------
	private MessageFramework mF;
	private NXTInfo m_info;
	
	// CONSTRUCTORS
	//-------------
	public Brick(String name, String macAddress){
		
		setM_info(new NXTInfo(NXTCommFactory.BLUETOOTH, name, macAddress));
		setmF(new MessageFramework());
		getmF().addMessageListener(this);
		getmF().ConnectToNXT(m_info);
	}

	// METHODS
	//--------
	@Override
	public void recievedNewMessage(LIMessage msg) {
		System.out.println("The Brick just sent: " + msg.getPayload());		
	}

	public void sendMessage(String s){
		getmF().SendMessage(new LIMessage(LIMessageType.Command, s));
		System.out.println(s);
	}
	
	public void sendDirections(int angle, int power){
		sendMessage("m " + angle + " " + power);
	}
	
	public void sendRotation(int power){
		sendMessage("r " + power);
	}
	
	// GETTERS - SETTERS
	//------------------
	public void setmF(MessageFramework mF) {
		this.mF = mF;
	}

	public MessageFramework getmF() {
		return mF;
	}

	public void setM_info(NXTInfo m_info) {
		this.m_info = m_info;
	}

	public NXTInfo getM_info() {
		return m_info;
	}
}
