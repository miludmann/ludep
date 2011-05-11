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
	
	public ValueBoard vb;
	

	//! Constructor
	public Computer(String name, String addr){
		setBrick(new Brick(this, name, addr));
	}
	
	//! Setter brick
	public void setBrick(Brick brick) {
		this.brick = brick;
	}

	//! Getter brick
	public Brick getBrick() {
		return brick;
	}
	
	public ValueBoard getVb() {
		return vb;
	}

	public void setVb(ValueBoard vb) {
		this.vb = vb;
	}

	//! Main
	public static void main(String[] args) throws IOException {
		
		Computer cmp = new Computer("Branson", "0016530DB4A2");
		Controller xc = new Controller(cmp);
		
		cmp.setVb(new ValueBoard()); 
		
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
				
				cmp.getBrick().sendMessage(input);
			}
    }
}
