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

	public ServerCom(int port) {

		ServerSocket serverSocket = null;
		
		try {
			serverSocket = new ServerSocket(port);
		} catch (IOException e) {
			System.err.println("Could not listen on port: " + port);
			System.exit(1);
		}

		Socket clientSocket = null;
		
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

		System.out.println("Connection closed");
	}

	public void run() {

		String inputLine;

		try {

			while ((inputLine = in.readLine()) != null) {
				treatMessageReceived(inputLine);
			}

		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}
	
	public void treatMessageReceived(String s){
		System.out.println("SERVER: String received: " + s);
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
}
