package tools;

import java.util.ArrayList;

public class MessageComputer {
	
	// ATTRIBUTES
	//-----------
	
	private String msg;
	private ArrayList<String> splitmsg;
	
	// CONSTRUCTORS
	//-------------
	
	public MessageComputer(String s){
		setMsg(s);
		this.splitMessage();
	}

	// METHODS
	//--------
	
	public void splitMessage(){
		this.split(' ');
	}
	
	public void split(char sep){
		
		String s = this.getMsg();
		char prec = ' ';
		int index = -1;
		
		ArrayList<String> res = new ArrayList<String>();
		
		for (int i = 0; i < s.length(); i++){
			if( prec == sep ){
				if( s.charAt(i) != sep){
					index = i;
				}
			}
			else
			{
				if( s.charAt(i) == sep){
					res.add(s.substring(index, i));
					index = -1;
				}
			}
			prec = s.charAt(i);
		}
		
		if( index != -1 ){
			res.add(s.substring(index, s.length()));
		}
		
		this.setSplitmsg(res);
	}
	
	public int nbFragments(){
		return this.getSplitmsg().size();
	}
	
	public String getFragment(int i){
		return this.getSplitmsg().get(i);
	}
	
	// GETTERS - SETTERS
	//------------------
	
	public void setMsg(String msg) {
		this.msg = msg;
	}

	public String getMsg() {
		return msg;
	}
	
	public void setSplitmsg(ArrayList<String> splitmsg) {
		this.splitmsg = splitmsg;
	}

	public ArrayList<String> getSplitmsg() {
		return splitmsg;
	}

}
