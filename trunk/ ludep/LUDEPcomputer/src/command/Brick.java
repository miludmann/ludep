package command;

import java.awt.Point;

import lejos.pc.comm.NXTCommFactory;
import lejos.pc.comm.NXTInfo;
import GUI.BrickGUI;
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
	private Point position;
	private Point goToPostion;
	private BrickGUI brickGUI;
	private int id;
	private boolean flag;
	private int direction;
	private BrickChecker bc;
	private boolean isOnEdge;
	
	private int stateBehavior = 0;
	
	// CONSTRUCTORS
	//-------------
	public Brick(int id, Computer c, String name, String macAddress){
		
		setCmp(c);
		setId(id);
		setName(name);
		setAddress(macAddress);
		setPosition(null);
		setDirection(90);
		setOnEdge(false);

		setBrickGUI(new BrickGUI(this));
	}

	// METHODS
	//--------
	
	public void connect(){

		if ( isConnectionEstablished() ) return;
		flag = false;
		
		setM_info(new NXTInfo(NXTCommFactory.BLUETOOTH, getName(), getAddress()));
		
		setmF(new MessageFramework());
		getmF().setMessageListener(this);
		getmF().ConnectToNXT(m_info);
				
		if ( isConnectionEstablished() )
		{
			idProcess();
		}
		
		getCmp().getMg().refresh();
	}
	
	public void idProcess(){
		sendMessage("ID " + getId());
		
		long time = System.currentTimeMillis();
		
		while ( System.currentTimeMillis() - time < 3000
				&& !flag ) {}
		
		if (!flag)
			getmF().close();
	}
	
	
	public void analyseMsg(String s) {
		if (s.equalsIgnoreCase("Brick connected"))
		{
			setFlag(true);
			setBc(new BrickChecker(this));
		}
		
		if (getBc() != null) getBc().resetCount();
		
		if ( null != getCmp().getMg() ){
			getBrickGUI().setText(s);
		}
	}

	public void connectionStatus(boolean bool) {
		setConnectionEstablished(bool);

		if(! bool)
			getBrickGUI().brickDisconnected();
	}
	
	public void receivedNewMessage(LIMessage msg) {
		analyseMsg(msg.getPayload());
		// comment or not to see the messages of the brick 
		// System.out.println("BRICK: " + msg.getPayload());
	}

	public void sendMessage(String s){
		if ( isConnectionEstablished() )
			getmF().SendMessage(new LIMessage(LIMessageType.Command, s));
	}
	
	public void moveCart(int x, int y)
	{
		sendMessage("moveC " + x + " " + y);
	}
	
	public void moveCart(int x, int y, int speed)
	{
		sendMessage("moveC " + x + " " + y + " " + speed);
	}
	
	public void move(int angle, int length)
	{
		sendMessage("move " + angle + " " + length);
	}
	
	public void move(int angle, int length, int speed)
	{
		sendMessage("move " + angle + " " + length + " " + speed);
	}
	
	public void setLeader(){
		getCmp().setBrickInControl(this);
	}
	
	public void closeConnection()
	{
		getmF().close();
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

	public void setConnectionEstablished(boolean connectionEstablished) {
		this.connectionEstablished = connectionEstablished;
		
		if ( ! connectionEstablished )
		{
			setBc(null);
			setFlag(false);
			setPosition(null);
			getCmp().switchBrick();
		}
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

	public void setBrickGUI(BrickGUI brickGUI) {
		this.brickGUI = brickGUI;
	}

	public BrickGUI getBrickGUI() {
		return brickGUI;
	}

	public boolean isFlag() {
		return flag;
	}

	public void setFlag(boolean flag) {
		this.flag = flag;
		
		if (flag)
			getBrickGUI().brickConnected();
	}

	public void setPosition(Point position) {
		this.position = position;
		
		if (position == null)
			sendMessage("s");
	}

	public Point getPosition() {
		return position;
	}

	public void setDirection(int direction) {
		this.direction = direction;
	}

	public int getDirection() {
		return direction;
	}

	public void setBc(BrickChecker bc) {
		this.bc = bc;
	}

	public BrickChecker getBc() {
		return bc;
	}

	public boolean isOnEdge() {
		return isOnEdge;
	}

	public void setOnEdge(boolean isOnEdge) {
		this.isOnEdge = isOnEdge;
	}

	public Point getGoToPostion() {
		return goToPostion;
	}

	public void setGoToPostion(Point goToPostion) {
		this.goToPostion = goToPostion;
		
		if (goToPostion != null)
		{
			moveCart(goToPostion.x-getPosition().x, 
					 goToPostion.y-getPosition().y);
		}
	}

	public int getStateBehavior() {
		return stateBehavior;
	}

	public void setStateBehavior(int stateBehavior) {
		
		if (  this.stateBehavior != stateBehavior )
			sendMessage("bip " + stateBehavior);
		
		this.stateBehavior = stateBehavior;
	}
}
