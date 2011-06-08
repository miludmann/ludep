package command;

import java.util.Queue;

import tools.MessageComputer;

public class MessageRegulator extends Thread{

	// ATTRIBUTES
	//-----------
	private ControlMotor cm;
	private Queue queueMessages;
	
	
	// CONSTRUCTORS
	//-------------
	public MessageRegulator(ControlMotor cm)
	{
		setCm(cm);
		setQueueMessages(new Queue());
	}

	// METHODS
	//--------
	public void run()
	{
		MessageComputer tmp;
		
		while(getCm().getNxt().isRunning())
		{
			if ( getQueueMessages().size() > 0 )
			{
				tmp = (MessageComputer) getQueueMessages().pop();
				commandMotors(tmp);
			}
		}
		
		
	}

	public void addMessage(MessageComputer mc)
	{
		getQueueMessages().addElement(mc);
	}
	
	/**
	 * The function takes a message from the computer
	 * and analyzes it so as to command the motors
	 * @param mc the message incoming from the computer
	 */
	public void commandMotors(MessageComputer mc){
		
		int nbFrag = mc.nbFragments();
		String si = null;
		
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
			
			if ( si.equalsIgnoreCase("reg") )
			{
				if ( nbFrag > i+1 )
				{
					boolean b = Boolean.parseBoolean(mc.getFragment(i+1));
					getCm().changeMotorsRegulation(b);
				}
				
				sendMessageCmp("Motor Regulation = " + getCm().isRegulateSpeed());
			}
			
			if  ( si.equalsIgnoreCase("wd") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setWheelDiameter(i1);
				}
				
				sendMessageCmp("Wheel Diameter = " + getCm().getWheelDiameter());
			}
			
			if  ( si.equalsIgnoreCase("fp") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setFactorP(i1);					
				}
				
				sendMessageCmp("Factor P = " + getCm().getFactorP());
			}
			
			if  ( si.equalsIgnoreCase("fi") )
			{
				if ( nbFrag > i+1 )
				{
					double i1 = Double.parseDouble(mc.getFragment(i+1));
					getCm().setFactorI(i1);					
				}
				
				sendMessageCmp("Factor I = " + getCm().getFactorI());
			}
			
			if  ( si.equalsIgnoreCase("rec") )
			{
				getCm().drawRectangle();
			}
			
			if ( si.equalsIgnoreCase("move") && (nbFrag > i+2) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				getCm().moveAngLen(i1, i2);
			}
			
			if ( si.equalsIgnoreCase("moveC") && (nbFrag > i+2) )
			{
				int i1 = Integer.parseInt(mc.getFragment(i+1));
				int i2 = Integer.parseInt(mc.getFragment(i+2));
				getCm().moveCart(i1, i2);
			}
		}
	}
	
	public void sendMessageCmp(String s){
		getCm().getNxt().getCl().sendMessage(s);
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
}
