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
	private Computer cmp;
	
	// CONSTRUCTORS
	//-------------
	public Brick(Computer c, String name, String macAddress){
		
		setCmp(c);
		setM_info(new NXTInfo(NXTCommFactory.BLUETOOTH, name, macAddress));
		setmF(new MessageFramework());
		getmF().addMessageListener(this);
		getmF().ConnectToNXT(m_info);
	}

	// METHODS
	//--------
	@Override
	public void recievedNewMessage(LIMessage msg) {
		System.out.println("The brick sent_ " + msg.getPayload());
		String[] splitmsg = msg.getPayload().split(" ");
		
		if ( splitmsg.length == 4 && splitmsg[0].equals("MOT"))
		{
			getCmp().getVb().getJl4().setText(splitmsg[1]);
			getCmp().getVb().getJl5().setText(splitmsg[2]);
			getCmp().getVb().getJl6().setText(splitmsg[3]);
		}
		
	}

	public void sendMessage(String s){
		getmF().SendMessage(new LIMessage(LIMessageType.Command, s));
		// System.out.println(s);
	}
	
	public void sendDirections(int angle, int power){
		sendMessage("m " + angle + " " + power);
	}
	
	public void sendRotation(int power){
		sendMessage("r " + power);
	}
	
	public void sendCommand(int angle, int power, int pRot){
		sendMessage("x " + angle
				   + " " + power
				   + " " + pRot);
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

	public Computer getCmp() {
		return cmp;
	}

	public void setCmp(Computer cmp) {
		this.cmp = cmp;
	}
	
}
