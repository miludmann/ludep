package test;

import java.io.DataInputStream;
import java.io.DataOutputStream;

import lejos.nxt.LCD;
import lejos.nxt.Sound;
import lejos.nxt.comm.Bluetooth;
import lejos.nxt.comm.NXTConnection;



public class BTConnector {

	public static void main(String[] args) throws Exception
	{
	    String connected = "Connected";
	    String waiting  =  "Waiting...";
	    
	    LCD.drawString(waiting,0,0);
		NXTConnection connection = Bluetooth.waitForConnection();
		LCD.drawString(connected,0,0);

		DataInputStream input = connection.openDataInputStream();
		DataOutputStream output = connection.openDataOutputStream();

		String in = "";
		int i = 0;

		while(true)
		{
			try
			{
				in = input.readLine();
				LCD.drawString(in, 1, 1);
				Sound.beep();
			}
			catch (Exception e) {
			}
			
			output.writeChars(in);
			output.flush();
			
			LCD.drawInt(i, 1, 1);
			i++;
		}
	}
}
