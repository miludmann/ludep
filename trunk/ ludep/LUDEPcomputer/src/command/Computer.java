package command;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;

import GUI.MainGUI;

//! Class to run the program on the computer
public class Computer {

	private ArrayList<Brick> brickList;
	private ArrayList<String> brickNames;
	private ArrayList<String> brickAddresses;
	private int brickInControl;
	private Controller cont;
	private MainGUI mg;

	//! Constructor
	public Computer(){
		
		setBrickList(new ArrayList<Brick>());
		setBrickNames(new ArrayList<String>());
		setBrickAddresses(new ArrayList<String>());
		
		setMg(new MainGUI(this));
		setBrickInControl(-1);
		
		getBrickNames().add("Branson");
		getBrickAddresses().add("0016530DB4A2");
		
		getBrickNames().add("Jambon");
		getBrickAddresses().add("0016530DB4FC");
		
		generateBrickList();
		setCont(new Controller(this));
	}
	
	
	// METHODS
	//--------
	public void generateBrickList(){
		
		Brick tmp;
		
		for (int i = 0; i < getBrickNames().size(); i++) {
			tmp = new Brick(this,
							getBrickNames().get(i),
							getBrickAddresses().get(i));
			
			if ( tmp.isConnectionEstablished()
					&& tmp.getId() >= 0 )
			{
				if ( getBrickInControl() < 0 )
					setBrickInControl(0);
				
				getBrickList().add(tmp);
			}
		}
		
		getMg().generateGUI();
	}
	
	public Brick getBrick(){
		if (getBrickInControl() < 0
				|| getBrickInControl() >= getBrickList().size())
			return null;
		
		return getBrickList().get(getBrickInControl());
	}
	
	public void msgBricks(String s){
		for (int i = 0; i < getBrickList().size(); i++) {
			getBrickList().get(i).sendMessage(s);
		}		
	}
	
	public void msgNotControlledBricks(String s){
		for (int i = 0; i < getBrickList().size(); i++) {
			if ( i != getBrickInControl() )
				getBrickList().get(i).sendMessage(s);
		}
	}
	
	public void msgControlledBrick(String s){
		getBrick().sendMessage(s);
	}
	
	public void send(String s){
		
		
		if ( s.equalsIgnoreCase("stop"))
		{
			msgBricks("stop");
			System.exit(0);
		}
		
		if ( getBrickInControl() < 0 )
		{
			System.out.println("No brick found");
			return;
		}
		
		if ( getBrickInControl() == getBrickList().size() )
		{
			msgBricks(s);
		}
		else
		{
			msgControlledBrick(s);
		}
	}
	
	public void switchBrick(){
		setBrickInControl((getBrickInControl()+1)%(getBrickList().size()+1));
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
		
		System.out.println("You can play now...");
		
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


	public ArrayList<String> getBrickNames() {
		return brickNames;
	}


	public void setBrickNames(ArrayList<String> brickNames) {
		this.brickNames = brickNames;
	}


	public ArrayList<String> getBrickAddresses() {
		return brickAddresses;
	}


	public void setBrickAddresses(ArrayList<String> brickAddresses) {
		this.brickAddresses = brickAddresses;
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


	public void setBrickInControl(int brickInControl) {
		this.brickInControl = brickInControl;
	}


	public int getBrickInControl() {
		return brickInControl;
	}
}
