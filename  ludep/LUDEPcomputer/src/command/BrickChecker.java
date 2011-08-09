package command;

import java.awt.Point;

public class BrickChecker extends Thread {

	private Brick brick;
	private int heartBeatChecker;
	private Point lastPosition;
	private int samePositionChecker;
	
	public int limitHeartBeat = 10;
	public int limitPositionUnchanged = 10;

	public BrickChecker(Brick brick)
	{
		this.brick = brick;
		
		setDaemon(true);
		start();
	}

	public void run()
	{
		resetCount();
		samePositionChecker = 0;

		while (getHeartBeatChecker() < limitHeartBeat) {
			try {
				Thread.sleep(250);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			setHeartBeatChecker(getHeartBeatChecker()+1);
			
			// In that part, we check if the brick is still located
			// We could have done another thread for this purpose
			// But let's save the world... one thread at a time

			if (lastPosition == null)
			{
				lastPosition = brick.getPosition(); 
			}
			else
			{
				if(lastPosition != null)
				{
					if(lastPosition.equals(brick.getPosition()))
					{
						samePositionChecker++;
						
						if(samePositionChecker == limitPositionUnchanged)
						{
							// System.out.println("Brick " + brick.getId() + " has been lost");
							brick.setPosition(null);
							lastPosition = null;
							samePositionChecker = 0;
						}
					}
					else
					{
						lastPosition = brick.getPosition();
						samePositionChecker = 0;
					}
				}
			}
			
		}

		brick.closeConnection();
		interrupt();
	}
	
	public void resetCount()
	{
		setHeartBeatChecker(0);
	}

	public void setHeartBeatChecker(int heartBeatChecker) {
		this.heartBeatChecker = heartBeatChecker;
	}

	public int getHeartBeatChecker() {
		return heartBeatChecker;
	}

}
