package Control;


public class ControllerRegulator extends Thread{
	
	// ATTRIBUTES
	//-----------
	private Controller cont;
	private double lastLD, lastLM, lastRD, lastRM;
	
	// CONSTRUCTORS
	//-------------
	public ControllerRegulator(Controller c){
		setCont(c);
		setLastLM(0);
		setLastLD(0);
		setLastRM(0);
		setLastRD(0);
		
		setDaemon(true);
		start();
	}

	// METHODS
	//--------
	public void run(){
	
		while (true) {
			if ( valueHaveChanged() )
			{
				setLastLM(getCont().getLeftMagnitude());
				setLastLD(getCont().getLeftDirection());
				setLastRM(getCont().getRightMagnitude());
				setLastRD(getCont().getRightDirection());
				refreshDrivingParameters();
				
			}
			
			try {
				Thread.sleep(200);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	public void refreshDrivingParameters(){
		
		int flockDirection = (int) ((-getLastLD()+90+360)%360);
		if ( getLastLM() > 25 )
			getCont().getComp().getFh().setFlockDirection(flockDirection);
		
		getCont().getComp().send("x " +
				(int) getLastLD() + " " +
				(int) getLastLM() + " " +
				(int) (getLastRM()*Math.cos(2*Math.PI/360*(450-getLastRD())%360)));
	}
	
	public boolean valueHaveChanged(){
		return getLastLD()!=getCont().getLeftDirection()
			|| getLastLM()!=getCont().getLeftMagnitude()
			|| getLastRD()!=getCont().getRightDirection()
			|| getLastRM()!=getCont().getRightMagnitude();
	}
	// GETTERS - SETTERS
	//------------------
	public Controller getCont() {
		return cont;
	}

	public void setCont(Controller cont) {
		this.cont = cont;
	}

	public void setLastLM(double lastLM) {
		this.lastLM = lastLM;
	}

	public double getLastLD() {
		return lastLD;
	}

	public void setLastLD(double lastLD) {
		this.lastLD = lastLD;
	}

	public double getLastRD() {
		return lastRD;
	}

	public void setLastRD(double lastRD) {
		this.lastRD = lastRD;
	}

	public double getLastRM() {
		return lastRM;
	}

	public void setLastRM(double lastRM) {
		this.lastRM = lastRM;
	}

	public double getLastLM() {
		return lastLM;
	}
}
