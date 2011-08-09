package command;

import java.awt.Point;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;

import Control.Controller;
import Flock.FlockHandler;
import GUI.MainGUI;
import Server.IServerCom;
import Server.ServerCom;

//! Class to run the program on the computer
public class Computer implements IServerCom {

	private ArrayList<Brick> brickList;
	private Brick brickInControl;
	private Controller cont;
	private MainGUI mg;
	private ServerCom sc;
	private FlockHandler fh;
	
	
	//! Constructor
	public Computer(){
		
		setBrickList(new ArrayList<Brick>());
		
		getBrickList().add(new Brick(1, this, "Branson", "0016530DB4A2"));
		getBrickList().add(new Brick(2, this, "Jambon", "0016530DB4FC"));
		getBrickList().add(new Brick(3, this, "Pampa", "0016530BEA38"));
		
		setCont(new Controller(this));
		
		setMg(new MainGUI(this));
		getMg().generateGUI();
		setBrickInControl(null);
		
		setFh(new FlockHandler(this, getBrickList()));
		
		restartServer();
	}
	
	
	// METHODS
	//--------
	public void restartServer()
	{
		setSc(new ServerCom(4242, this));
	}
	
	
	public void msgBricks(String s){
		
		if ( getBrickList().size() == 0 )
			return;
		
		for (Brick b : getBrickList()) {
			b.sendMessage(s);
		}
	}
	
	public void msgNotControlledBricks(String s){
		
		if ( null != getBrickInControl() )
			return;
		
		for (Brick b : getBrickList()) {
			if ( ! b.equals(getBrickInControl()) )
				b.sendMessage(s);
		}
	}
	
	public void msgControlledBrick(String s){
		getBrickInControl().sendMessage(s);
	}
	
	public void send(String s){
		if ( s.equalsIgnoreCase("stop"))
		{
			stopProgram();
		}
		
		if ( null != getBrickInControl() )
		{
			msgControlledBrick(s);
		}
		else
		{
			msgBricks(s);
		}
	}

	public void switchBrick(){		
		for (Brick b : getBrickList()) {
			
			if ( null == getBrickInControl() && b.isConnectionEstablished() ){
				setBrickInControl(b);
				
				if ( getSc() != null )
					getSc().sendMessage("leader " + b.getId());
				
				return;
			}
			
			if ( b.equals(getBrickInControl()) )
				setBrickInControl(null);
		}
		
		if ( getSc() != null )
			getSc().sendMessage("leader -1");
	}
	
	public void stopProgram(){
		msgBricks("stop");
		System.exit(0);
	}

	public void treatmessage(String s) {
		//TODO: treat message from client
		// System.out.println("Received from client: " + s);
		
		String splitMessage[] = s.split(" ");
		
		int id, posX, posY, degRobot, degDrone;
		boolean isOnLimit;
		
		id = Integer.parseInt(splitMessage[1]);
		posX = Integer.parseInt(splitMessage[4]);
		posY = Integer.parseInt(splitMessage[6]);
		degRobot = Integer.parseInt(splitMessage[8]);
		degDrone = Integer.parseInt(splitMessage[10]);
		isOnLimit = Boolean.parseBoolean(splitMessage[12]);
		
		for (Brick b : getBrickList()) {
			if ( b.getId() == id)
			{
				b.setPosition(new Point(posX, posY));
				b.setOnEdge(isOnLimit);
				//if ( deg != 181 )
				//	b.setDirection(deg);
				return;
			}
		}
	}

	// MAIN
	//-----
	public static void main(String[] args) throws IOException {
		
		Computer cmp = new Computer();
		
		
		// ============================================
		// Part where we treat all the messages we send 
		// ============================================

		InputStreamReader istream = new InputStreamReader(System.in) ;
		BufferedReader bufRead = new BufferedReader(istream) ;
		
		String input = "";
		
		System.out.println("Everything's ready...");
		
		while(true){

			try {
			     input = bufRead.readLine();
			}
			catch (IOException err) {
			     System.out.println("Error reading line");
			}

			cmp.send(input);

		}
		// ============================================
		// End of the input treatment
		// ============================================
    }

	
	// GETTERS - SETTERS
	//------------------
	public ArrayList<Brick> getBrickList() {
		return brickList;
	}


	public void setBrickList(ArrayList<Brick> brickList) {
		this.brickList = brickList;
	}

	public Controller getCont() {
		return cont;
	}


	public void setCont(Controller cont) {
		this.cont = cont;
	}


	public MainGUI getMg() {
		return mg;
	}


	public void setMg(MainGUI mg) {
		this.mg = mg;
	}


	public void setBrickInControl(Brick brickInControl) {
		
		if ( brickInControl == this.brickInControl)
			return;
		
		this.brickInControl = brickInControl;
		
		mg.refresh();
	}


	public Brick getBrickInControl() {
		return brickInControl;
	}


	public void setSc(ServerCom sc) {
		this.sc = sc;
	}


	public ServerCom getSc() {
		return sc;
	}


	public void setFh(FlockHandler fh) {
		this.fh = fh;
	}


	public FlockHandler getFh() {
		return fh;
	}
}
