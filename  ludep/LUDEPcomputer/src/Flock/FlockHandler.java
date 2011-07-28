package Flock;

import java.awt.Point;
import java.util.ArrayList;

import command.Brick;
import command.Computer;

public class FlockHandler extends Thread{
	
	private Computer computer;
	private ArrayList<Brick> brickList;
	private Brick brickInControl;
	private int sizeFlock;
	private int nbRobotsFollowing;
	private int desiredSpace;
	
	private ArrayList<Brick> consideredBricks;
	private ArrayList<Point> desiredPosition;
	
	public FlockHandler(Computer c, ArrayList<Brick> brickList, Brick b)
	{
		this.computer = c;
		this.brickInControl = b;
		this.brickList = brickList;
		this.desiredSpace = 50;
		
		this.setDaemon(true);
		this.start();
	}
	
	public void run(){
		while ( true )
		{
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			checkFlock();
			
			if ( isLeaderLocated() )
			{
				computeDesiredLocations();
				
			}
		}
	}
	
	public void computeDesiredLocations()
	{
		setDesiredPosition(new ArrayList<Point>());
		
		for (int i = 0; i < nbRobotsFollowing; i++) {
			
		}
	}
	
	public boolean isLeaderLocated()
	{
		if ( getBrickInControl() == null ) return false;
		
		return brickInControl.getPosition() != null;
	}
	
	public void checkFlock()
	{
		int nbFlock = 0;
		setConsideredBricks(new ArrayList<Brick>());
		
		for (Brick b : brickList) {
			if( b.isConnectionEstablished() )
			{
				nbFlock++;
				getConsideredBricks().add(b);
			}
		}
		
		this.sizeFlock = nbFlock;
		this.nbRobotsFollowing = nbFlock - 1;
	}

	public Computer getComputer() {
		return computer;
	}

	public void setComputer(Computer computer) {
		this.computer = computer;
	}

	public ArrayList<Brick> getBrickList() {
		return brickList;
	}

	public void setBrickList(ArrayList<Brick> brickList) {
		this.brickList = brickList;
	}

	public Brick getBrickInControl() {
		return brickInControl;
	}

	public void setBrickInControl(Brick brickInControl) {
		this.brickInControl = brickInControl;
	}

	public int getSizeFlock() {
		return sizeFlock;
	}

	public void setSizeFlock(int sizeFlock) {
		this.sizeFlock = sizeFlock;
	}

	public int getNbRobotsFollowing() {
		return nbRobotsFollowing;
	}

	public void setNbRobotsFollowing(int nbRobotsFollowing) {
		this.nbRobotsFollowing = nbRobotsFollowing;
	}

	public void setDesiredPosition(ArrayList<Point> desiredPosition) {
		this.desiredPosition = desiredPosition;
	}

	public ArrayList<Point> getDesiredPosition() {
		return desiredPosition;
	}

	public void setConsideredBricks(ArrayList<Brick> consideredBricks) {
		this.consideredBricks = consideredBricks;
	}

	public ArrayList<Brick> getConsideredBricks() {
		return consideredBricks;
	}
	
	
}