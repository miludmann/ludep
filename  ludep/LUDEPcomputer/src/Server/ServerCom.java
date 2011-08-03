package Server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class ServerCom extends Thread {

	private BufferedReader in;
	private PrintWriter out;
	private IServerCom isc;
	
	private ServerSocket serverSocket;
	private Socket clientSocket;
	private int port;
	

	public ServerCom(int port, IServerCom isc){
		this.isc = isc;
		this.port = port;
		connectTo(port);
	}
	
	public void connectTo(int port) {

		//ServerSocket serverSocket = null;
		
		System.out.println("Waiting for client");
		try {
			serverSocket = new ServerSocket(port);
		} catch (IOException e) {
			System.err.println("Could not listen on port: " + port);
			System.exit(1);
		}

		//Socket clientSocket = null;
		
		try {
			clientSocket = serverSocket.accept();
		} catch (IOException e) {
			System.err.println("Accept failed.");
			System.exit(1);
		}

		setIn(null);
		setOut(null);
		
		try {
			setIn(new BufferedReader(new InputStreamReader(clientSocket
					.getInputStream())));
			setOut(new PrintWriter(clientSocket.getOutputStream(), true));
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		this.start();
		System.out.println("Connexion with client OK");
	}

	public void run() {

		String inputLine;

		try {

			while ((inputLine = in.readLine()) != null) {
				treatMessageReceived(inputLine);
			}
			System.out.println("Connection closed");

		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		try {
			closeConnection();
		} catch (IOException e) {}
		
		connectTo(this.port);
	}
	
	
	public void closeConnection() throws IOException
	{
		in.close();
		out.close();
		serverSocket.close();
		clientSocket.close();
	}
	
	public void treatMessageReceived(String s){
		if ( null != this.isc )
		{
			isc.treatmessage(s);
		}
	}

	public void sendMessage(String s) {
		getOut().println(s);
	}

	public BufferedReader getIn() {
		return in;
	}

	public void setIn(BufferedReader in) {
		this.in = in;
	}

	public PrintWriter getOut() {
		return out;
	}

	public void setOut(PrintWriter out) {
		this.out = out;
	}

	public void setIsc(IServerCom isc) {
		this.isc = isc;
	}

	public IServerCom getIsc() {
		return isc;
	}
}
