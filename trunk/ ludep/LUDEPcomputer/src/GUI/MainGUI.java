package GUI;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;

import command.Brick;
import command.Computer;

public class MainGUI {
	
	private Computer cmp;
    private JFrame window;
	
	public MainGUI(Computer cmp){
		setCmp(cmp);
		setWindow(new JFrame());
		
		getWindow().addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				getCmp().msgBricks("stop");
			}
		});
	}
	
	
	public void addBrick(Brick b){
		
		JButton button = new JButton();
		button.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent e) {
				System.out.println("yep");
			}
		});
		
		JLabel lab1 = new JLabel(b.getName());
		JLabel lab2 = new JLabel(b.getAddress());
				
		getWindow().add(button);
		getWindow().add(lab1);
		getWindow().add(lab2);
	}
	
	public void generateGUI() {
		
		ArrayList<Brick> bricks = getCmp().getBrickList();
		
		getWindow().setLayout(new GridLayout(bricks.size(), 3));
		
		for (int i = 0; i < bricks.size(); i++) {
			addBrick(bricks.get(i));		
		}
		
		getWindow().pack();
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

	public void setCmp(Computer cmp) {
		this.cmp = cmp;
	}

	public Computer getCmp() {
		return cmp;
	}
}
