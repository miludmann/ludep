package command;

import java.io.IOException;

import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;
import net.java.games.input.test.ControllerEventTest;

public class test {

	public test(){
		
	}
	
	public static void main(String[] args) throws IOException {
		
		// ControllerEventTest test = new ControllerEventTest();

		Controller[] controllers = ControllerEnvironment.getDefaultEnvironment().getControllers();
		Controller gamepad = null;
		
		for (int i = 0; i < controllers.length; i++) {
			
			System.out.println(controllers[i].getType());
			
			if (controllers[i].getType().equals(Controller.Type.GAMEPAD))
			{
				System.out.println("GAMEPAD FOUND");
				gamepad = controllers[i];
			}
		}
		
		if ( null == gamepad ) return;
		
		gamepad.poll();
		
		String a1 = "haha";
		String bb = a1;
		String test = new String(a1);
		a1 = "huhu";
		
		System.out.println(a1 + " " + bb);
		
	}	
}
