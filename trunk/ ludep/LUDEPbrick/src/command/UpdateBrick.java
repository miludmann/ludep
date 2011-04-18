package command;

public class UpdateBrick extends Thread{
	
	
	private ControlMotor cm;
	
	public UpdateBrick(ControlMotor cm){
		this.setCm(cm);
	}
	
	public void run(){
		while(true){
			cm.refreshMotors("abc");

			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
			}

		}
	}
	
	public ControlMotor getCm() {
		return cm;
	}

	public void setCm(ControlMotor cm) {
		this.cm = cm;
	}


	
}
