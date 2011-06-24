package command;

import lejos.nxt.Motor;

public class MotorRegulator extends Thread {
	
	// ATTRIBUTES
	//-----------
	private ControlMotor cm;
	private boolean inRegulation;
	
	// CONSTRUCTORS
	//-------------
	public MotorRegulator(ControlMotor cm)
	{
		setCm(cm);
		setInRegulation(false);
	}
	
	// METHODS
	//--------
	public void run(){
		
		double ref;
		double noRot, noRotInt, corRot;
		
		while(true)
		{
			if(isInRegulation())
			{
				ref = getMotorTachoCountSum();
				noRot = 0;
				noRotInt = 0;
								
				while(isInRegulation())
				{
					noRotInt = noRotInt*getCm().getReduceI() + noRot;
					noRot = ref - getMotorTachoCountSum();
					
					corRot = noRot*getCm().getFactorP()
					       + noRotInt*getCm().getFactorI();
					
					/*
					getCm().getNxt().getCl().sendMessage(
							"Rot " + (int) noRot + " " +
							"RotInt " + (int) noRotInt + " " +
							"corRot " + (int) corRot
							);
					*/
					
					getCm().setRotationPower(corRot);
					getCm().refreshMotors();
				}
			}
		}
		
	}
	
	public int getMotorTachoCountSum()
	{
		return Motor.A.getTachoCount()
			 + Motor.B.getTachoCount()
			 + Motor.C.getTachoCount();
	}
	
	
	// GETTERS - SETTERS
	//------------------
	public void setCm(ControlMotor cm) {
		this.cm = cm;
	}

	public ControlMotor getCm() {
		return cm;
	}

	public void setInRegulation(boolean inRegulation) {
		this.inRegulation = inRegulation;
	}


	public boolean isInRegulation() {
		return inRegulation;
	}
}
