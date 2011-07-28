package command;

import lejos.nxt.Battery;

public class Pulsor extends Thread {

	private NXT nxt;
	
	public Pulsor(NXT nxt)
	{
		setNxt(nxt);
		
		start();
	}
	
	public void run()
	{
		String message = "";
		
		while(getNxt().isRunning())
		{
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {}
			
			message = "boom " + Battery.getVoltageMilliVolt();
			getNxt().getCl().sendMessage(message);
		}
	}

	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}

	public NXT getNxt() {
		return nxt;
	}
}
