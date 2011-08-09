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
	
	// private ReachableAreas ra;

	private Brick brickInControl;
	private ArrayList<Brick> consideredBricks;
	private ArrayList<Point> desiredPositions;
	
	public int proximityThreshold = 200;
	public int proximityLeader = 1000;
	public float desiredSpace = 1200;

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

			checkFlock();
			
			if ( null != getComputer().getBrickInControl() )
			{
				// checkFlock();

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
				/*
				getConsideredBricks().get(i).sendMessage("move " 
						+ ((brickPosition.x-brickInControl.getPosition().x)*100) + " "
						+ ((brickPosition.y-brickInControl.getPosition().y)*100) );
						*/
				
				double angle = Math.atan2(brickPosition.y-brickInControl.getPosition().y,
										  brickPosition.x-brickInControl.getPosition().x);
				
				desiredPosition.translate((int) (proximityLeader*Math.cos(angle)),
										  (int) (proximityLeader*Math.sin(angle)));
				
			}
			else
			{
				if ( distancePositions(desiredPosition, brickPosition) > proximityThreshold )
				{
					if (  getConsideredBricks().get(i).isOnEdge() )
					{
						/*
						getConsideredBricks().get(i).sendMessage("moveC " 
								+ ((brickInControl.getPosition().x-brickPosition.x)/2+brickPosition.x) + " "
								+ ((brickInControl.getPosition().y-brickPosition.y)/2+brickPosition.y));
								*/
						
						giveBrickDirection(getConsideredBricks().get(i),
										   new Point((brickInControl.getPosition().x-brickPosition.x)/2+brickPosition.x,
										   			 (brickInControl.getPosition().y-brickPosition.y)/2+brickPosition.y));
					}
					else
					{
						
						/*
						getConsideredBricks().get(i).sendMessage("moveC " 
								+ (desiredPosition.x-brickPosition.x) + " "
								+ (desiredPosition.y-brickPosition.y));
						*/
						
						// System.out.println("A " + brickPosition.toString() + " / " + desiredPosition.toString());
						
						giveBrickDirection(getConsideredBricks().get(i), desiredPosition);
					}
				}
				else
				{
					getConsideredBricks().get(i).sendMessage("s");
				}
			}
		}
	}

	public void giveBrickDirection(Brick b, Point p)
	{
		while( distancePositions(b.getPosition(), p) > proximityThreshold )
		{
			if(movementPossible(b, p))
			{
				b.setGoToPostion(p);
				System.out.println("= " + b.getPosition().toString() + " / " + b.getGoToPostion().toString());
				return;
			}
			else
			{
				p.translate((-p.x+b.getPosition().x)/3,
							(-p.y+b.getPosition().y)/3);
			}
		}
	}
	
	public boolean movementPossible(Brick b, Point p)
	{
		Point start = b.getPosition();
		Point end = p;
		
		for (Brick brick : getConsideredBricks()) {
			if (! b.equals(brick) )
			{
				if ( crossedMovements(start, end, brick.getPosition(), brick.getGoToPostion()) )
					return false;
			}
		}
		
		return true;
	}
	
	public boolean crossedMovements(Point PA, Point PB, Point PC, Point PD)
	{
		if ( PD == null )
		{
			return Geometry.distance(PA.x, PA.y, PB.x, PB.y, PC.x, PC.y) < proximityThreshold;
		}
		
		
		if ( Geometry.isLineIntersectingLine(PA.x, PA.y,
											 PB.x, PB.y,
											 PC.x, PC.y,
											 PD.x, PD.y) )
		{
			return true;
		}
		
		if ( Geometry.distance(PA.x, PA.y, PB.x, PB.y, PC.x, PC.y) < proximityThreshold )
		{
			return true;
		}
		if ( Geometry.distance(PA.x, PA.y, PB.x, PB.y, PD.x, PD.y) < proximityThreshold )
		{
			return true;
		}
		if ( Geometry.distance(PC.x, PC.y, PD.x, PD.y, PA.x, PA.y) < proximityThreshold )
		{
			return true;
		}
		if ( Geometry.distance(PC.x, PC.y, PD.x, PD.y, PB.x, PB.y) < proximityThreshold )
		{
			return true;
		}
		
		return false;
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
		
		// setRa(new ReachableAreas(this));
		
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
				b.setGoToPostion(null);

				if ( b.equals(getComputer().getBrickInControl()) )
					brickInControl = b;
				else
					getConsideredBricks().add(b);
			}
		}

		setSizeFlock(nbFlock);
		setNbRobotsFollowing(nbFlock - 1);
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
		
		if (sizeFlock != this.sizeFlock)
			System.out.println("Size Flock: " + sizeFlock);
		
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
