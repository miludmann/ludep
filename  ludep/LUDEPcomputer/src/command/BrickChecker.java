package command;

public class BrickChecker extends Thread {

	private Brick brick;
	private int heartBeatChecker;

	public BrickChecker(Brick brick)
	{
		this.brick = brick;
		
		setDaemon(true);
		start();
	}

	public void run()
	{
		resetCount();

		while (getHeartBeatChecker() < 5) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			setHeartBeatChecker(getHeartBeatChecker()+1);
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
