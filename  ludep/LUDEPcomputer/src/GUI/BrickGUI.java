package GUI;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JLabel;

import command.Brick;


public class BrickGUI{

	private Brick brick;
	private JButton buttonConnect;
	private JButton buttonLeader;
	private JLabel label;
	private JLabel baterryLabel;
	private ActionListener connect, leader;

	public BrickGUI(Brick b){
		setBrick(b);
		setLabel(new JLabel("Please connect"));
		setBaterryLabel(new JLabel(""));

		setButtonConnect(new JButton());
		connect = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				getBrick().connect();
			}
		};
		getButtonConnect().setText("Connect to " + brick.getName());
		getButtonConnect().addActionListener(connect);

		setButtonLeader(new JButton());
		leader = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				getBrick().setLeader();
			}
		};
		getButtonLeader().setText("Set " + brick.getName() + " leader");
		getButtonLeader().addActionListener(leader);

		getButtonLeader().setEnabled(false);
	}

	public void brickConnected(){
		getButtonConnect().setEnabled(false);
		getButtonLeader().setEnabled(true);
		getLabel().setText("Brick connected!");
	}

	public void brickDisconnected(){
		getButtonLeader().setEnabled(false);
		getButtonConnect().setEnabled(true);
		getLabel().setText("Connection closed");
	}

	public void setText(String s){
		String[] splitText = s.split(" ");
		int battery;
		
		if( splitText[0].equalsIgnoreCase("boom") )
		{
			battery = Integer.parseInt(splitText[1]);
			getBaterryLabel().setText("Battery: " + battery + "mV");
			reactToBatteryLvl(battery);
		}
		else
		{
			getLabel().setText(s);
		}
	}
	
	public void reactToBatteryLvl(int batteryLvl)
	{
		if ( batteryLvl < 6500 )
		{
			brick.sendMessage("stop");
			getBrick().closeConnection();
			getBaterryLabel().setText("Battery too low _ connection closed");
		}
	}

	public JButton getButtonConnect() {
		return buttonConnect;
	}

	public void setButtonConnect(JButton buttonConnect) {
		this.buttonConnect = buttonConnect;
	}

	public JButton getButtonLeader() {
		return buttonLeader;
	}

	public void setButtonLeader(JButton buttonLeader) {
		this.buttonLeader = buttonLeader;
	}

	public ActionListener getConnect() {
		return connect;
	}

	public void setConnect(ActionListener connect) {
		this.connect = connect;
	}

	public ActionListener getLeader() {
		return leader;
	}

	public void setLeader(ActionListener leader) {
		this.leader = leader;
	}

	public void setBrick(Brick brick) {
		this.brick = brick;
	}

	public Brick getBrick() {
		return brick;
	}

	public void setLabel(JLabel label) {
		this.label = label;
	}

	public JLabel getLabel() {
		return label;
	}

	public void setBaterryLabel(JLabel baterryLabel) {
		this.baterryLabel = baterryLabel;
	}

	public JLabel getBaterryLabel() {
		return baterryLabel;
	}
}
