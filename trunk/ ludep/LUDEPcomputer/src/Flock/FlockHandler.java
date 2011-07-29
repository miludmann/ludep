package Flock;

import java.awt.Point;
import java.util.ArrayList;

import command.Brick;
import command.Computer;

public class FlockHandler extends Thread{

	private Computer computer;
	private ArrayList<Brick> brickList;
	private int sizeFlock;
	private int nbRobotsFollowing;
	private float desiredSpace;

	private Brick brickInControl;
	private ArrayList<Brick> consideredBricks;
	private ArrayList<Point> desiredPositions;

	public FlockHandler(Computer c, ArrayList<Brick> brickList)
	{
		this.computer = c;
		this.brickList = brickList;
		this.desiredSpace = 300;

		this.setDaemon(true);
		this.start();
	}

	public void run(){
		while ( true )
		{
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			if ( null != getComputer().getBrickInControl() )
			{
				checkFlock();

				System.out.println("Size Flock: " + sizeFlock);

				if ( isLeaderLocated() && sizeFlock > 1 )
				{
					computeDesiredLocations();
					giveCommands();
				}
			}
		}
	}

	public void giveCommands()
	{
		Point brickPosition, desiredPosition;
		
		for (int i = 0; i < getConsideredBricks().size(); i++) {
			
			brickPosition = getConsideredBricks().get(i).getPosition();
			desiredPosition = getDesiredPositions().get(i);
			
			
			getConsideredBricks().get(i).sendMessage("moveC " 
					+ (desiredPosition.x-brickPosition.x) + " "
					+ (desiredPosition.y-brickPosition.y));
			
			/*
			System.out.println(getConsideredBricks().get(i).getName() + "moveC " 
					+ (desiredPosition.x-brickPosition.x) + " "
					+ (desiredPosition.y-brickPosition.y));
			*/
		}
	}

	public void computeDesiredLocations()
	{
		setDesiredPositions(new ArrayList<Point>());

		float referenceAngle = brickInControl.getDirection();
		referenceAngle = (float) (referenceAngle*2*Math.PI/360);

		Point referencePoint = new Point((int) (desiredSpace*Math.cos(referenceAngle)),
				(int) (desiredSpace*Math.sin(referenceAngle)));

		Point bufferPoint;
		float bufferAngle;

		for (int i = 1; i<=nbRobotsFollowing; i++) {

			bufferAngle = i*360/sizeFlock;
			bufferAngle = (float) (referenceAngle + bufferAngle*2*Math.PI/360);

			bufferPoint = new Point((int) (desiredSpace*Math.cos(bufferAngle)),
					(int) (desiredSpace*Math.sin(bufferAngle)));

			bufferPoint.translate(-referencePoint.x, -referencePoint.y);
			bufferPoint.translate(brickInControl.getPosition().x, brickInControl.getPosition().y);

			getDesiredPositions().add(bufferPoint);
		}

	}

	public boolean isLeaderLocated()
	{
		if ( brickInControl == null ) return false;

		return brickInControl.getPosition() != null;
	}

	public void checkFlock()
	{
		int nbFlock = 0;
		setConsideredBricks(new ArrayList<Brick>());
		brickInControl = null;

		for (Brick b : brickList) {
			if( b.isConnectionEstablished() && b.getPosition() != null )
			{
				nbFlock++;

				if ( b.equals(getComputer().getBrickInControl()) )
					brickInControl = b;					
				else
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

	public void setDesiredPositions(ArrayList<Point> desiredPosition) {
		this.desiredPositions = desiredPosition;
	}

	public ArrayList<Point> getDesiredPositions() {
		return desiredPositions;
	}

	public void setConsideredBricks(ArrayList<Brick> consideredBricks) {
		this.consideredBricks = consideredBricks;
	}

	public ArrayList<Brick> getConsideredBricks() {
		return consideredBricks;
	}


}
