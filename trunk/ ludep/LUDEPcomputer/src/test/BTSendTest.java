package test;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;


public class BTSendTest {
	
	private static ArrayList<brickConnector> bricks;

	public static void main(String[] args){
		
		bricks = new ArrayList<brickConnector>();
		
		bricks.add(new brickConnector("Branson", "0016530DB4A2"));
		bricks.add(new brickConnector("Jambon",  "0016530DB4FC"));
		
		bricks.get(1).connect();
		
		// ============================================
		// Part where we treat all the messages we send 
		// ============================================

		InputStreamReader istream = new InputStreamReader(System.in) ;
		BufferedReader bufRead = new BufferedReader(istream) ;
		
		String input = "";
		
		System.out.println("You can play now...");
		
		while(true){

			try {
			     input = bufRead.readLine();
			}
			catch (IOException err) {
			     System.out.println("Error reading line");
			}
			
			sendMessageBricks(input);
		}
		// ============================================
		// End of the input treatment
		// ============================================
	}
	
	public static void sendMessageBricks(String s){
		for (brickConnector b : bricks) {
			b.sendMessage(s);
		}
	}
}
