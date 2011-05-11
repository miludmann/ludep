package command;

import java.awt.GridLayout;

import javax.swing.JFrame;
import javax.swing.JLabel;

public class ValueBoard {
	
	private JFrame window;
	private JLabel jl1,jl2,jl3,jl4,jl5,jl6;
	
	
	public ValueBoard(){
		setWindow(new JFrame());
		getWindow().setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		getWindow().setLayout(new GridLayout(6, 2));

		setJl1(new JLabel("-"));
		setJl2(new JLabel("-"));
		setJl3(new JLabel("-"));
		setJl4(new JLabel("-"));
		setJl5(new JLabel("-"));
		setJl6(new JLabel("-"));
		
		
		window.add(new JLabel("Angle:"));
		window.add(getJl1());
		window.add(new JLabel("Speed:"));
		window.add(getJl2());
		window.add(new JLabel("Rotation:"));
		window.add(getJl3());
		window.add(new JLabel("motA:"));
		window.add(getJl4());
		window.add(new JLabel("motB:"));
		window.add(getJl5());
		window.add(new JLabel("motC:"));
		window.add(getJl6());
		
	    
	    
	    window.pack();
	    
		getWindow().setLocationRelativeTo(null);
		
		getWindow().setVisible(true);
        
	}

	// GETTERS - SETTERS
	//------------------
	public void setWindow(JFrame window) {
		this.window = window;
	}

	public JFrame getWindow() {
		return window;
	}

	public JLabel getJl1() {
		return jl1;
	}

	public void setJl1(JLabel jl1) {
		this.jl1 = jl1;
	}

	public JLabel getJl2() {
		return jl2;
	}

	public void setJl2(JLabel jl2) {
		this.jl2 = jl2;
	}

	public JLabel getJl3() {
		return jl3;
	}

	public void setJl3(JLabel jl3) {
		this.jl3 = jl3;
	}

	public JLabel getJl4() {
		return jl4;
	}

	public void setJl4(JLabel jl4) {
		this.jl4 = jl4;
	}

	public JLabel getJl5() {
		return jl5;
	}

	public void setJl5(JLabel jl5) {
		this.jl5 = jl5;
	}

	public JLabel getJl6() {
		return jl6;
	}

	public void setJl6(JLabel jl6) {
		this.jl6 = jl6;
	}
	

}
