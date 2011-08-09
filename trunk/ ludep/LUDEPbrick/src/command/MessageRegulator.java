package command;

import java.util.Queue;

import tools.MessageComputer;

public class MessageRegulator extends Thread{

	// ATTRIBUTES
	//-----------
	private NXT nxt;
	private ControlMotor cm;
	private Queue queueMessages;
	private int nbCmdReceived;
	private int nbCmdDone;
	private MessageComputer bis;
	private boolean calibrating;
	
	
	// CONSTRUCTORS
	//-------------
	public MessageRegulator(NXT nxt)
	{
		setNxt(nxt);
		setBis(null);
		setCm(new ControlMotor(getNxt()));
		setQueueMessages(new Queue());
		setCalibrating(false);
		
		setNbCmdReceived(0);
		setNbCmdDone(0);
	}

	// METHODS
	//--------
	public void run()
	{
		MessageComputer tmp;
		
		while(getCm().getNxt().isRunning())
		{
			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {}
			
			if ( getQueueMessages().size() > 0 )
			{
				setNbCmdDone(getNbCmdDone()+1);
				tmp = (MessageComputer) getQueueMessages().pop();
				commandMotors(tmp);
			}
		}		
	}

	public void addMessage(MessageComputer mc)
	{
		getQueueMessages().addElement(mc);
		setNbCmdReceived(getNbCmdReceived()+1);
	}
	
	/**
	 * The function takes a message from the computer
	 * and analyzes it so as to command the motors
	 * @param mc the message incoming from the computer
	 */
	public void commandMotors(MessageComputer mc){
		
		int nbFrag = mc.nbFragments();
		String si = null;
		
		if( mc.equals(null) ) return;
		
		
		if ( mc.getMsg().equalsIgnoreCase("+") )
		{
			commandMotors(getBis());
			return;
		}
		
		for ( int i = 0; i < nbFrag; i++ )
		{
			si = mc.getFragment(i);
			
			if ( si.indexOf("a") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				getCm().setPowerA(i1);
			}
			
			if ( si.indexOf("b") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				getCm().setPowerB(i1);
			}
			
			if ( si.indexOf("c") >= 0  && (nbFrag > i+1) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				getCm().setPowerC(i1);
			}
			
			if ( si.equalsIgnoreCase("s") )
			{
				getCm().setPowerA(0);
				getCm().setPowerB(0);
				getCm().setPowerC(0);
				getCm().setRotationPower(0);
			}
			
			if ( si.indexOf("x") >= 0  && (nbFrag > i+3) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				int i3 = Integer.parseInt(mc.getFragment(i+3));
				getCm().setMaxSpeed(60);
				getCm().angleMotors(i1,i2);
				getCm().setRotationPower(i3);
			}
			
			if ( si.equalsIgnoreCase("motreg") )
			{
				if ( nbFrag > i+1 )
				{
					boolean b = Boolean.parseBoolean(mc.getFragment(i+1));
					getCm().changeMotorsRegulation(b);
				}
				getCm().stopMotors();
				sendMessageCmp("Motor Regulation = " + getCm().isRegulateSpeed());
			}
			
			/*
			if ( si.equalsIgnoreCase("cc") )
			{
				if ( isCalibrating() )
				{
					getInterpretator().getNxt().getCs().stopCalibration();
					getCm().stopMotors();
					setCalibrating(false);
					getInterpretator().getNxt().getCl().sendMessage("Calibration OFF");
				}
				else
				{
					getInterpretator().getNxt().getCs().startCalibration();
					getCm().setMaxSpeed(100);
					getCm().setPowerA(10);
					getCm().setPowerB(10);
					getCm().setPowerC(10);
					setCalibrating(true);
					getInterpretator().getNxt().getCl().sendMessage("Calibration ON");
				}
			}
			*/
			
			if  ( si.equalsIgnoreCase("wd") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setWheelDiameter(i1);
				}
				getCm().stopMotors();
				sendMessageCmp("Wheel Diameter = " + getCm().getWheelDiameter());
			}
			
			if  ( si.equalsIgnoreCase("reg") )
			{
				if ( nbFrag > i+1 )
				{
					if ( mc.getFragment(i+1).equalsIgnoreCase("switch") )
					{
						getCm().getMotreg().setInRegulation(!getCm().getMotreg().isInRegulation());
					}
					else
					{
						boolean b = Boolean.parseBoolean(mc.getFragment(i+1));
						getCm().getMotreg().setInRegulation(b);
					}
				}
				getCm().stopMotors();
				sendMessageCmp("Regulation = " + getCm().getMotreg().isInRegulation());
			}
			
			if  ( si.equalsIgnoreCase("fp") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setFactorP(i1);					
				}
				getCm().stopMotors();
				sendMessageCmp("Factor P = " + getCm().getFactorP());
			}
			
			if  ( si.equalsIgnoreCase("fi") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setFactorI(i1);					
				}
				getCm().stopMotors();
				sendMessageCmp("Factor I = " + getCm().getFactorI());
			}
			
			if  ( si.equalsIgnoreCase("fri") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setReduceI(i1);					
				}
				getCm().stopMotors();
				sendMessageCmp("Factor Reduction I = " + getCm().getReduceI());
			}
			
			if  ( si.equalsIgnoreCase("draw") )
			{
				if ( nbFrag > i+2 )
				{
					int i1 = Integer.parseInt(mc.getFragment(i+1));
					int i2 = Integer.parseInt(mc.getFragment(i+2));
					getCm().drawPentagone(i1, i2);
				}
				else
				{
					if ( nbFrag > i+1 )
					{
						int i1 = Integer.parseInt(mc.getFragment(i+1));
						getCm().drawPentagone(i1, 5000);
					}
					else
					{
						getCm().drawPentagone(4, 5000);
					}
				}
			}
			
			if ( si.equalsIgnoreCase("move") && (nbFrag > i+2) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				getCm().moveAngLen(i1, i2);
			}
			
			if ( si.equalsIgnoreCase("move") && (nbFrag > i+3) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				int i3 = Integer.parseInt(mc.getFragment(i+3));
				getCm().setMaxSpeed(i3);
				getCm().moveAngLen(i1, i2);
			}
			
			if ( si.equalsIgnoreCase("moveC") && (nbFrag > i+2) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				getCm().moveCart(i1, i2);
			}
			
			if ( si.equalsIgnoreCase("moveC") && (nbFrag > i+3) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				int i3 = Integer.parseInt(mc.getFragment(i+3));
				getCm().setMaxSpeed(i3);
				getCm().moveCart(i1, i2);
			}
		}
		
		setBis(new MessageComputer(mc.getMsg()));
		getCm().refreshMotors();
	}
	
	public void sendMessageCmp(String s){
		getCm().getNxt().getCl().sendMessage(s);
	}
	
	public void treatMessage(MessageComputer mc){
		getCm().incID();
		addMessage(mc);
	}

	
	// GETTERS - SETTERS
	//------------------
	public void setCm(ControlMotor cm) {
		this.cm = cm;
	}

	public ControlMotor getCm() {
		return cm;
	}


	public void setQueueMessages(Queue queueMessages) {
		this.queueMessages = queueMessages;
	}


	public Queue getQueueMessages() {
		return queueMessages;
	}

	public int getNbCmdReceived() {
		return nbCmdReceived;
	}

	public void setNbCmdReceived(int nbCmdReceived) {
		this.nbCmdReceived = nbCmdReceived;
	}

	public int getNbCmdDone() {
		return nbCmdDone;
	}

	public void setNbCmdDone(int nbCmdDone) {
		this.nbCmdDone = nbCmdDone;
	}

	public void setBis(MessageComputer bis) {
		this.bis = bis;
	}

	public MessageComputer getBis() {
		return bis;
	}

	public boolean isCalibrating() {
		return calibrating;
	}

	public void setCalibrating(boolean calibrating) {
		this.calibrating = calibrating;
	}

	public NXT getNxt() {
		return nxt;
	}

	public void setNxt(NXT nxt) {
		this.nxt = nxt;
	}
}
