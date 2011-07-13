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
	
	private String name;
	private String address;
	private MessageFramework mF;
	private NXTInfo m_info;
	private Computer cmp;
	private boolean connectionEstablished;
	private int id;
	
	// CONSTRUCTORS
	//-------------
	public Brick(Computer c, String name, String macAddress){
		
		setName(name);
		setAddress(macAddress);
		
		setCmp(c);
		setM_info(new NXTInfo(NXTCommFactory.BLUETOOTH, name, macAddress));
		setmF(new MessageFramework());
		getmF().addMessageListener(this);
		setConnectionEstablished(getmF().ConnectToNXT(m_info));
		setId(-1);
		
		if ( isConnectionEstablished() )
			idProcess();
	}

	// METHODS
	//--------
	public void idProcess(){
		sendMessage("ID " + getCmp().getBrickList().size());
		
		long time = System.currentTimeMillis();
		
		while ( System.currentTimeMillis() - time < 3000
				&& getId() < 0 ) {}
	}
	
	
	public void analyseMsg(String s) {
		String[] splitmessage = s.split(" ");
		
		if ( splitmessage[0].equalsIgnoreCase("ID") && getId()<0 )
		{
			setId(Integer.parseInt(splitmessage[1]));
		}
		
		if ( null != getCmp().getMg() ){
			getCmp().getMg().displayMessageBrick(s);
		}
	}
	
	@Override
	public void receivedNewMessage(LIMessage msg) {
		analyseMsg(msg.getPayload());
		System.out.println("BRICK: " + msg.getPayload());
	}

	public void sendMessage(String s){
		getmF().SendMessage(new LIMessage(LIMessageType.Command, s));		
	}
	
	/*
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
	*/
	
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

	public void setConnectionEstablished(boolean connectionEstablished) {
		this.connectionEstablished = connectionEstablished;
	}

	public boolean isConnectionEstablished() {
		return connectionEstablished;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return id;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getAddress() {
		return address;
	}

	public void setAddress(String address) {
		this.address = address;
	}
}
