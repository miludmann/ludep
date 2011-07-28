package MessageComponent;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;

import lejos.pc.comm.NXTCommFactory;
import lejos.pc.comm.NXTConnector;
import lejos.pc.comm.NXTInfo;


public class MessageFramework {

	//private static MessageFramework m_instance = new MessageFramework();
	private Object m_RXguard;

	protected MessageListenerInterface m_messageListener;
	protected NXTConnector m_connecter;
	protected InputStream m_is;
	protected OutputStream m_os;
	protected boolean m_connected;

	private Reader m_reader;
	private ArrayList<Byte> m_receivedBytes;

	public MessageFramework() {

		m_messageListener = null;
		m_receivedBytes = new ArrayList<Byte>();
		m_RXguard = new Object();

		m_connecter = new NXTConnector();
		m_connected = false;
		
		m_reader = new Reader(); 
		m_reader.setDaemon(true);
	}

	public boolean ConnectToNXT(NXTInfo info)
	{
		if ( m_connected ) return true;
		
		setM_connected(m_connecter.connectTo(info.name, info.deviceAddress, NXTCommFactory.BLUETOOTH));

		if (!m_connected) return false;

		m_is = m_connecter.getInputStream();
		m_os = m_connecter.getOutputStream();

		if (m_is == null || m_os == null)
		{
			setM_connected(false);
		}

		if (!m_connected) return false; 

		try{
			m_reader.start(); //Start to listen for incoming messages
		} catch ( IllegalThreadStateException e ) {
			e.printStackTrace();
		}

		return m_connected;
	}

	public void SendMessage(LIMessage msg)
	{
		try
		{
			byte[] outgoing = msg.getEncodedMsg();

			m_os.write(outgoing);
			m_os.flush();

		} catch (IOException e) {
			e.printStackTrace();
			close();
		}		
	}

	public void setMessageListener(MessageListenerInterface msgListener)
	{
		m_messageListener = msgListener;
	}

	private class Reader extends Thread
	{
		public void run()
		{
			while (m_connected)
			{
				try {

					byte input;
					//TODO: We should address the blocking issue.
					while ((input = (byte)m_is.read()) >= 0)
					{
						//Adding package to received array
						m_receivedBytes.add(input);

						//Check if this is a packet frame end <ETX>
						if(input == (byte)3) //ETX
							ReceivedNewPacket();
					}
				}
				catch (IOException e) {
					e.printStackTrace();
					close();
				}
			}
		}
	}

	private void ReceivedNewPacket()
	{
		byte[] msgBytes = new byte[m_receivedBytes.size()];
		for(int i=0; i<m_receivedBytes.size(); i++)
		{
			msgBytes[i] = m_receivedBytes.get(i);
		}

		m_receivedBytes.clear();

		//only create and transmit the message if it is valid
		if(isPacketValid(msgBytes))
		{
			synchronized (m_RXguard) {
				LIMessage msg = LIMessage.setEncodedMsg(msgBytes);
				m_messageListener.receivedNewMessage(msg);
			}
		}
		else
		{
			System.out.println("Invalid Packet:" + new String(msgBytes));
		}
	}

	private boolean isPacketValid(byte[] packet)
	{
		try
		{
			if(packet[0] != (byte)2) //Does it contain a start frame byte?	
				return false;
			else if(packet[2] != (byte)':') //Does it contain the command payload seperator?
				return false;
			else if(packet[packet.length-1] != (byte)3) //Does it contain an end frame byte?
				return false;
			else
				return true;
		} catch (Exception e) {
			System.out.println("Received corrupt package: " + getBytesString(packet));
			return false;
		}
	}

	private String getBytesString(byte[] bytes)
	{
		StringBuilder sb = new StringBuilder();
		for(int i=0; i<bytes.length; i++)
		{
			sb.append(String.valueOf(bytes[i] + ", "));
		}

		return sb.toString();
	}

	public void close() {
		
		try {
			setM_connected(false);
			m_is.close();
			m_os.close();

			if (m_connecter != null)
				m_connecter.close();
		}
		catch (IOException e){
			e.printStackTrace();
			System.err.println("Error while closing MessageFramework: " + e.getMessage());
		}
	}

	public boolean isM_connected() {
		return m_connected;
	}

	public void setM_connected(boolean mConnected) {
		m_connected = mConnected;
		m_messageListener.connectionStatus(m_connected);
	}
}
