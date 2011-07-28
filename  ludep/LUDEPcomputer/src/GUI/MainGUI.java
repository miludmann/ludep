package GUI;

import java.awt.Font;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JTextField;

import Ccommand.Brick;
import Ccommand.Computer;



public class MainGUI {
	
	private Computer cmp;
    private JFrame window;
    private JButton allBrickConnect;
    private JButton allBrickControl;
    private JTextField textArea;
	
	public MainGUI(Computer cmp){
		setCmp(cmp);
	}
	
	public void addBrick(Brick b){
		getWindow().add(b.getBrickGUI().getButtonConnect());
		getWindow().add(b.getBrickGUI().getButtonLeader());
		getWindow().add(b.getBrickGUI().getLabel());
	}
	
	public void generateGUI() {
		
		setTextArea(new JTextField(20));
		setWindow(new JFrame());
		
		getWindow().addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				getCmp().stopProgram();
			}
		});
		
		getWindow().setLayout(new GridLayout(getCmp().getBrickList().size()+1, 3));
		
		for (Brick b : getCmp().getBrickList()) {
			addBrick(b);
		}
		
		setAllBrickConnect(new JButton());
		getAllBrickConnect().setText("Connect all bricks");
		ActionListener connect = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				for (Brick b : getCmp().getBrickList()) {
					b.connect();
				}
			}
		};
		getAllBrickConnect().addActionListener(connect);

		setAllBrickControl(new JButton());
		getAllBrickControl().setText("Control all bricks");
		ActionListener control = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				getCmp().setBrickInControl(null);
			}
		};
		getAllBrickControl().addActionListener(control);
		
		getWindow().add(getAllBrickConnect());
		getWindow().add(getAllBrickControl());
		getWindow().add(getTextArea());
		
		getTextArea().addKeyListener(new KeyListener() {
			
			@Override
			public void keyReleased(KeyEvent e) {
				if ( e.getKeyCode() == KeyEvent.VK_ENTER )
				{
					getCmp().send(getTextArea().getText());
					getTextArea().setText("");
				}
				
				if ( e.getKeyCode() == KeyEvent.VK_F1)
				{
					getCmp().switchBrick();
				}	
			}

			@Override
			public void keyPressed(KeyEvent e) {
				// TODO Auto-generated method stub
			}

			@Override
			public void keyTyped(KeyEvent e) {
				// TODO Auto-generated method stub
			}
		});
		
		getWindow().pack();
		getWindow().setLocationRelativeTo(null);
		getWindow().setVisible(true);
	}
	
	public void displayMessageBrick(String s){
		
		String[] splitmessage = s.split(" ");
		String reconstructedMessage = "";
		int idBrickMessage = Integer.parseInt(splitmessage[1]);
		
		if ( splitmessage.length < 3 ) return;
		
		System.out.println(s);
		
		for (int i = 2; i < splitmessage.length; i++) {
			reconstructedMessage = reconstructedMessage + splitmessage[i] + " ";
		}
		
		for (Brick b : getCmp().getBrickList()) {
			if ( b.getId() == idBrickMessage )
				b.getBrickGUI().getLabel().setText(reconstructedMessage);
		}
	}
	
	public void refresh(){
		
		if ( getCmp().getBrickInControl() == null )
		{
			getAllBrickControl().setFont(new Font("Dialog", 1, 12));
		}
		else
		{
			getAllBrickControl().setFont(new Font("Dialog", 0, 12));
		}
		
		for (Brick b : getCmp().getBrickList()) {
			if ( b.equals(getCmp().getBrickInControl()))
			{
				b.getBrickGUI().getButtonLeader().setFont(new Font("Dialog", 1, 12));
			}
			else
			{
				b.getBrickGUI().getButtonLeader().setFont(new Font("Dialog", 0, 12));
			}
		}
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

	public void setTextArea(JTextField textArea) {
		this.textArea = textArea;
	}

	public JTextField getTextArea() {
		return textArea;
	}

	public JButton getAllBrickConnect() {
		return allBrickConnect;
	}

	public void setAllBrickConnect(JButton allBrickConnect) {
		this.allBrickConnect = allBrickConnect;
	}

	public JButton getAllBrickControl() {
		return allBrickControl;
	}

	public void setAllBrickControl(JButton allBrickControl) {
		this.allBrickControl = allBrickControl;
	}
}
