package Server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class ServerComTester {

	public static void main(String[] args) throws IOException {

		ServerCom sc = new ServerCom(4242);

		String outputLine;
		BufferedReader stdIn = new BufferedReader(new InputStreamReader(
				System.in));

		try {
			while (true) {

				outputLine = stdIn.readLine();
				sc.sendMessage(outputLine);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
