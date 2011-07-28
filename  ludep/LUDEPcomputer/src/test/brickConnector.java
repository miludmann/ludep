package test;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import lejos.pc.comm.NXTCommFactory;
import lejos.pc.comm.NXTConnector;
import lejos.pc.comm.NXTInfo;

public class brickConnector extends Thread {

	private NXTInfo info;
	private NXTConnector connector;
	public boolean isConnected = false;
	
	protected DataInputStream inputData = null;
	protected DataOutputStream outputData = null;


	public brickConnector(String name, String address){
		this.info = new NXTInfo(NXTCommFactory.BLUETOOTH, name, address);
		this.connector = new NXTConnector();
	}

	public void connect(){
		isConnected = connector.connectTo(info.name, 
				info.deviceAddress, 
				NXTCommFactory.BLUETOOTH);
		
		if ( isConnected ){
			inputData = connector.getDataIn();
			outputData = connector.getDataOut();
			
			this.start();
		}
	}
	
	public void run(){
		
		String message = "";
		
		while ( isConnected )
		{
            try {
            	message = inputData.readLine();
    			System.out.println(info.name + " sent: " + message);
            }
            catch (IOException e) {
            	e.printStackTrace();
            }			
		}
	}

	public void sendMessage(String s){
		
		if (! isConnected ) return;
		
		try
        {
			outputData.writeChars(s);
			outputData.flush();
            
        } catch (IOException e) {
        	e.printStackTrace();
        	System.err.println("SendMessage failed: " + e.getMessage());
        }
	}
}
