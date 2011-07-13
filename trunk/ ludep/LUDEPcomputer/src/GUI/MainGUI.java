package GUI;

import java.awt.Font;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextField;

import command.Brick;
import command.Computer;


public class MainGUI {
	
	private Computer cmp;
    private JFrame window;
    private ArrayList<JButton> buttonList;
    private ArrayList<JLabel> labelList;
    private JTextField textArea;
	
	public MainGUI(Computer cmp){
		setCmp(cmp);
		setButtonList(new ArrayList<JButton>());
		setLabelList(new ArrayList<JLabel>());
		setTextArea(new JTextField(20));
		setWindow(new JFrame());
		
		getWindow().addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				getCmp().stopProgram();
			}
		});
	}
	
	public void addBrick(Brick b, final int id){
		
		JButton button = new JButton();
		button.setText(b.getName());
		
		button.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent e) {
				getCmp().setBrickInControl(id);
			}
		});
		if ( id == 0 )
			button.setFont(new Font("Serif",Font.BOLD, 14));
		else
			button.setFont(new Font("Serif",Font.PLAIN, 14));
		
		getButtonList().add(id, button);		
		getWindow().add(button);
		
		JLabel label = new JLabel();
		getLabelList().add(label);
		getWindow().add(label);
	}
	
	public void generateGUI() {
		
		ArrayList<Brick> bricks = getCmp().getBrickList();
		final int sizeBrickList = bricks.size();
		
		getWindow().setLayout(new GridLayout(bricks.size()+1, 2));
		
		for (int i = 0; i < bricks.size(); i++) {
			addBrick(bricks.get(i), i);		
		}
		
		
		JButton button = new JButton();
		button.setText("All bricks");
		
		button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				getCmp().setBrickInControl(sizeBrickList);
			}
		});
		
		button.setFont(new Font("Serif",Font.PLAIN, 14));
		
		getButtonList().add(sizeBrickList, button);
		getWindow().add(button);
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
	
	public void changeBrickInControl(int i){
				
		for (JButton button : getButtonList()) {
			
			if ( getButtonList().indexOf(button) == i )
				button.setFont(new Font("Serif",Font.BOLD, 14));
			else
				button.setFont(new Font("Serif",Font.PLAIN, 14));
		}
	}

	
	public void displayMessageBrick(String s){
		
		if ( null == getLabelList() )
			return;
		
		String[] splitmessage = s.split(" ");
		String reconstructedMessage = "";
		int idBrickMessage = Integer.parseInt(splitmessage[1]);
		
		for (int i = 2; i < splitmessage.length; i++) {
			reconstructedMessage = reconstructedMessage + splitmessage[i] + " ";
		}
		
		getLabelList().get(idBrickMessage).setText(reconstructedMessage);
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

	public void setButtonList(ArrayList<JButton> buttonList) {
		this.buttonList = buttonList;
	}

	public ArrayList<JButton> getButtonList() {
		return buttonList;
	}

	public ArrayList<JLabel> getLabelList() {
		return labelList;
	}

	public void setLabelList(ArrayList<JLabel> labelList) {
		this.labelList = labelList;
	}

	public void setTextArea(JTextField textArea) {
		this.textArea = textArea;
	}

	public JTextField getTextArea() {
		return textArea;
	}


}
