package command;

public class CommandsRegulator extends Thread {
	
	// ATTRIBUTES
	//-----------
	private Controller cont;
	private long timeBuffer;
	private int oldDir, oldMagn, oldRot, dir, magn, rot;
	
	// CONSTRUCTORS
	//-------------
	public CommandsRegulator(Controller c) {
		setCont(c);
		refreshData(0,0,0);
		archiveData();
		setTimeBuffer(System.currentTimeMillis());
	}
		
	// METHODS
	//--------
	public void run(){
		
		long time;
		
		while (true){
			
			time = System.currentTimeMillis();
			
			if ( isUseful() /*&& valuesHaveChanged()*/ && time-getTimeBuffer() > 50){
				setTimeBuffer(time);
				sendDrivingDirections();
			}				
		}
		
	}
	
	public void sendDrivingDirections(){
		getCont().getComp().getBrick().sendCommand(getDir(), getMagn(), getRot());
		
		getCont().getComp().getVb().getJl1().setText(Integer.toString(getDir()));
		getCont().getComp().getVb().getJl2().setText(Integer.toString(getMagn()));
		getCont().getComp().getVb().getJl3().setText(Integer.toString(getRot()));

		archiveData();
	}

	public void refreshData(int leftDir, int leftMagn, int rightRot) {
		setDir(leftDir);
		setMagn(leftMagn);
		setRot(rightRot);		
	}
	
	public void archiveData(){
		setOldDir(getDir());
		setOldMagn(getMagn());
		setOldRot(getRot());
	}
	
	public boolean valuesHaveChanged(){
		return (getOldDir()!=getDir() ||
				getOldMagn()!=getMagn() ||
				getOldRot()!=getRot());
	}
	
	public boolean isUseful(){
		return !(getOldMagn()==0 &&
			   getMagn()==0 &&
			   getOldRot()==0 &&
			   getRot()==0);
	}
	
	// GETTERS - SETTERS
	//------------------
	
	public void setCont(Controller cont) {
		this.cont = cont;
	}

	public Controller getCont() {
		return cont;
	}


	public int getOldDir() {
		return oldDir;
	}


	public void setOldDir(int oldDir) {
		this.oldDir = oldDir;
	}


	public int getOldMagn() {
		return oldMagn;
	}


	public void setOldMagn(int oldMagn) {
		this.oldMagn = oldMagn;
	}


	public int getOldRot() {
		return oldRot;
	}


	public void setOldRot(int oldRot) {
		this.oldRot = oldRot;
	}


	public int getDir() {
		return dir;
	}


	public void setDir(int dir) {
		this.dir = dir;
	}


	public int getMagn() {
		return magn;
	}


	public void setMagn(int magn) {
		this.magn = magn;
	}


	public int getRot() {
		return rot;
	}


	public void setRot(int rot) {
		this.rot = rot;
	}

	public void setTimeBuffer(long timeBuffer) {
		this.timeBuffer = timeBuffer;
	}

	public long getTimeBuffer() {
		return timeBuffer;
	}
}
