package command;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

//! Class to run the program on the computer
public class Computer {

	//! A brick
	/*!
	 * The leader brick we control
	 */
	public Brick brick;
	

	//! Constructor
	public Computer(String name, String addr){
		setBrick(new Brick(name, addr));
	}
	
	//! Setter brick
	public void setBrick(Brick brick) {
		this.brick = brick;
	}

	//! Getter brick
	public Brick getBrick() {
		return brick;
	}

	//! Main
	public static void main(String[] args) throws IOException {
		
		Computer cmp = new Computer("Branson", "0016530DB4A2");
		Controller xc = new Controller(cmp);
		
		InputStreamReader istream = new InputStreamReader(System.in) ;
		BufferedReader bufRead = new BufferedReader(istream) ;
		
		String input = "";
		
		System.out.println("You can test now");
		
		while(true){

				try {
				     input = bufRead.readLine();
				}
				catch (IOException err) {
				     System.out.println("Error reading line");
				}
				
				cmp.getBrick().sendMessage(input);
			}
    }
}
