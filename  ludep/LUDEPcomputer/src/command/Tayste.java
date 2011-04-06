package command;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.ArrayList;

public class Tayste {
	
	// ATTRIBUTES
	//-----------
	
	private String msg;
	private String[] splitmsg;
	
	// CONSTRUCTORS
	//-------------
	
	public Tayste(String s){
		setMsg(s);
		this.splitMessage();
	}
	
	

	// METHODS
	//--------
	
	public void splitMessage(){
		
		String s = this.getMsg();
		char prec = ' ';
		int index = -1;
		
		ArrayList<String> res = new ArrayList<String>();
		
		for (int i = 0; i < s.length(); i++){
			if( prec == ' ' ){
				if( s.charAt(i) != ' '){
					index = i;
				}
			}
			else
			{
				if( s.charAt(i) == ' '){
					res.add(s.substring(index, i));
					index = -1;
				}
			}
			prec = s.charAt(i);
		}
		
		if( index != -1 ){
			res.add(s.substring(index, s.length()));
		}
		
		for (int i=0; i<res.size(); i++){
			System.out.println(res.get(i));
		}
	}
	
	
	// GETTERS - SETTERS
	//------------------
	
	public void setMsg(String msg) {
		this.msg = msg;
	}

	public String getMsg() {
		return msg;
	}

	public void setSplitmsg(String[] splitmsg) {
		this.splitmsg = splitmsg;
	}

	public String[] getSplitmsg() {
		return splitmsg;
	}
	
	//! Main
	public static void main(String[] args) throws IOException {

		Tayste tt = new Tayste("   mou sdsdha hadddha   ");

    }

}