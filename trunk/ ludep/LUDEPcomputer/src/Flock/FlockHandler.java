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
	private int flockDirection;
	private ArrayList<Integer> optimalRepartition;

	private Brick brickInControl;
	private ArrayList<Brick> consideredBricks;
	private ArrayList<Point> desiredPositions;
	
	public int proximityThreshold = 150;
	public int proximityLeader = 650;
	public float desiredSpace = 800;

	public FlockHandler(Computer c, ArrayList<Brick> brickList)
	{
		this.computer = c;
		this.brickList = brickList;
		this.flockDirection = 90;

		this.setDaemon(true);
		this.start();
	}

	public void run(){
		while ( true )
		{
			try {
				Thread.sleep(250);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			if ( null != getComputer().getBrickInControl() )
			{
				checkFlock();

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
		
		setOptimalRepartition(findOptimalMovements());
		
		
		for (int i = 0; i < getConsideredBricks().size(); i++) {
			
			brickPosition = getConsideredBricks().get(i).getPosition();
			desiredPosition = getDesiredPositions().get(getOptimalRepartition().get(i));
			
			if ( distancePositions(brickPosition, brickInControl.getPosition()) < proximityLeader )
			{
				getConsideredBricks().get(i).sendMessage("moveC " 
						+ ((brickPosition.x-brickInControl.getPosition().x)*100) + " "
						+ ((brickPosition.y-brickInControl.getPosition().y)*100) );
			}
			else
			{
				if ( distancePositions(desiredPosition, brickPosition) > proximityThreshold )
				{
					
					if (  getConsideredBricks().get(i).isOnEdge() )
					{
						getConsideredBricks().get(i).sendMessage("moveC " 
								+ ((brickInControl.getPosition().x-brickPosition.x)/2) + " "
								+ ((brickInControl.getPosition().y-brickPosition.y)/2));
					}
					else
					{
						getConsideredBricks().get(i).sendMessage("moveC " 
								+ (desiredPosition.x-brickPosition.x) + " "
								+ (desiredPosition.y-brickPosition.y));
					}
	
				/*
					System.out.println(getConsideredBricks().get(i).getName() + "moveC " 
						+ (desiredPosition.x-brickPosition.x) + " "
						+ (desiredPosition.y-brickPosition.y));
				*/
				}
				else
				{
					getConsideredBricks().get(i).sendMessage("s");
				}
			}
			if ( getComputer().t2 == 0 )
			{
				getComputer().t2 = System.currentTimeMillis();
				System.out.println("Time = " + (getComputer().t2 - getComputer().t1) );
			}
		}
	}
	
	public ArrayList<Integer> findOptimalMovements()
	{
		ArrayList<Integer> result = new ArrayList<Integer>();
		double bufferDistance, bufferPosition, tmpDist;
		Point bufferP1, bufferP2;
		
		/*
		for (int i = 0; i < getConsideredBricks().size(); i++) {
			result.add(i);
		}
		*/
		
		for (int i = 0; i < getConsideredBricks().size(); i++) {
			
			bufferDistance = -1;
			bufferPosition = -1;
			
			bufferP1 = getConsideredBricks().get(i).getPosition();
			
			for (int j = 0; j < getConsideredBricks().size(); j++) {
				
				if ( ! result.contains((Object) j) )
				{
					bufferP2 = getDesiredPositions().get(j);
					
					tmpDist = distancePositions(bufferP1, bufferP2);
					
					if ( bufferPosition == -1 || bufferDistance > tmpDist)
					{
						bufferPosition = j;
						bufferDistance = tmpDist;
					}
				}
			}
			
			result.add((int) bufferPosition);
		}
		
		return result;
	}

	public void computeDesiredLocations()
	{
		setDesiredPositions(new ArrayList<Point>());

		//float referenceAngle = brickInControl.getDirection();
		float referenceAngle = getFlockDirection();
		referenceAngle = (float) (referenceAngle*2*Math.PI/360);
		
		// Change the size of the circle according to the number of units in the flock
		float desiredCircleFlock = (float) (desiredSpace/(2*Math.sin(Math.PI/sizeFlock)));

		Point referencePoint = new Point((int) (desiredCircleFlock*Math.cos(referenceAngle)),
				(int) (desiredCircleFlock*Math.sin(referenceAngle)));

		Point bufferPoint;
		float bufferAngle;

		for (int i = 1; i<=nbRobotsFollowing; i++) {

			bufferAngle = i*360/sizeFlock;
			bufferAngle = (float) (referenceAngle + bufferAngle*2*Math.PI/360);

			bufferPoint = new Point((int) (desiredCircleFlock*Math.cos(bufferAngle)),
					(int) (desiredCircleFlock*Math.sin(bufferAngle)));

			bufferPoint.translate(-referencePoint.x, -referencePoint.y);
			bufferPoint.translate(brickInControl.getPosition().x, brickInControl.getPosition().y);

			getDesiredPositions().add(bufferPoint);
		}
	}

	public double distancePositions(Point a, Point b)
	{
		return Math.sqrt(((a.x)-(b.x))*((a.x)-(b.x))
				+((a.y)-(b.y))*((a.y)-(b.y)));
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

	public void setFlockDirection(int flockDirection) {
		this.flockDirection = flockDirection;
	}

	public int getFlockDirection() {
		return flockDirection;
	}

	public ArrayList<Integer> getOptimalRepartition() {
		return optimalRepartition;
	}

	public void setOptimalRepartition(ArrayList<Integer> optimalRepartition) {
		this.optimalRepartition = optimalRepartition;
	}


}
