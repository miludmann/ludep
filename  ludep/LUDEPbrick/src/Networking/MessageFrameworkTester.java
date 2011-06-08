package Networking;

import lejos.nxt.Button;
import lejos.nxt.ButtonListener;
import lejos.nxt.LCD;
import lejos.nxt.Sound;

public class MessageFrameworkTester implements MessageListenerInterface {

	protected MessageFramework m_commander;   
	   
	public MessageFrameworkTester() {
		m_commander = MessageFramework.getInstance();
	}
	   
	public void launch() {
		
	   m_commander.addMessageListener(this);
	   m_commander.StartListen();
	   
	   Button.LEFT.addButtonListener(new ButtonListener() {
		   @Override
		   public void buttonReleased(Button b) {
			   m_commander.SendMessage(new LIMessage(LIMessageType.Command, "LEFT"));
		   }
			
		   @Override
		   public void buttonPressed(Button b) {};
	   });
	   
	   Button.RIGHT.addButtonListener(new ButtonListener() {
		   @Override
		   public void buttonReleased(Button b) {
			   m_commander.SendMessage(new LIMessage(LIMessageType.Command, "RIGHT"));
		   }
			
		   @Override
		   public void buttonPressed(Button b) {};
	   });
	
	   m_commander.StopListen();
		
		while (!Button.ESCAPE.isPressed())
		{
		}
	}

	@Override
	public void receivedNewMessage(LIMessage msg) {
		
		LCD.drawString(msg.m_payload, 0, 0);
		LCD.drawString("length: " + msg.m_payload.length(), 0, 1);
		
		if(msg.m_payload.equals("0"))
		{
			LCD.drawString("BEEP", 0, 2);
			Sound.beep();
		}
		else if(msg.m_payload.equals("1"))
		{
			LCD.drawString("BUZZ", 0, 2);
			Sound.buzz();
		}
		else if(msg.m_payload.equals("2"))
		{
			LCD.drawString("TwoBEEP", 0, 2);
			Sound.twoBeeps();
		}
		else
		{
			LCD.drawString("No Match ???", 0, 2);
			Sound.beepSequenceUp();
		}
	}
}
