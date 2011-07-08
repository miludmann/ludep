package test;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;
import net.java.games.input.Event;
import net.java.games.input.EventQueue;

public abstract class Xbox360Controller extends Thread {

	private Controller cont;
	private float xAxis, yAxis, zNegAxis, zPosAxis, xRotation, yRotation;
	private double sensitivity;

	public Xbox360Controller() {

		setCont(null);
		initButtons();
		setSensitivity(0.2);

		checkController();

		if (null == getCont()) {
			System.out.println("Xbox 360 controller not found");
			return;
		}

		this.start();
	}

	public void run() {

		while (true) {

			getCont().poll();
			EventQueue queue = getCont().getEventQueue();
			Event event = new Event();

			Component comp = null;

			while (queue.getNextEvent(event)) {

				comp = event.getComponent();

				analyzeEvent(event);

				/*
				 * System.out.println(comp.getName());
				 * System.out.println(event.getValue());
				 */
			}
		}
	}

	private void initButtons() {
		setxAxis(0);
		setyAxis(0);
		setzNegAxis(0);
		setzPosAxis(0);
		setxRotation(0);
		setyRotation(0);
	}

	private void analyzeEvent(Event event) {
		String name = event.getComponent().getName();
		Float value = event.getValue();

		if (name.equals("X Axis")) {
			if (isFarEnough(value, getxAxis())) {
				setxAxis(value);
				xAxis(value);
			}
		}

		if (name.equals("Y Axis")) {
			if (isFarEnough(value, getyAxis())) {
				setyAxis(value);
				yAxis(value);
			}
		}

		if (name.equals("Z Axis")) {
			if (value > 0) {
				if (isFarEnough(value, getzPosAxis())) {
					setzPosAxis(value);
					zPosAxis(value);
				}
			} else {
				if (isFarEnough(value, getzNegAxis())) {
					setzNegAxis(value);
					zNegAxis(value);
				}
			}
		}

		if (name.equals("X Rotation")) {
			if (isFarEnough(value, getxRotation())) {
				setxRotation(value);
				xRotation(value);
			}
		}
		
		if (name.equals("Y Rotation")) {
			if (isFarEnough(value, getyRotation())) {
				setyRotation(value);
				yRotation(value);
			}
		}

		if (name.equals("Button 0")) {
			button0(value);
		}

		if (name.equals("Button 1")) {
			button1(value);
		}

		if (name.equals("Button 2")) {
			button2(value);
		}

		if (name.equals("Button 3")) {
			button3(value);
		}

		if (name.equals("Button 4")) {
			button4(value);
		}

		if (name.equals("Button 5")) {
			button5(value);
		}

		if (name.equals("Button 6")) {
			button6(value);
		}

		if (name.equals("Button 7")) {
			button7(value);
		}

		if (name.equals("Button 8")) {
			button8(value);
		}

		if (name.equals("Button 9")) {
			button9(value);
		}

	}

	abstract void yRotation(Float value);

	abstract void xRotation(Float value);

	abstract void zPosAxis(Float value);

	abstract void zNegAxis(Float value);

	abstract void yAxis(Float value);

	abstract void xAxis(Float value);

	abstract void button9(Float value);

	abstract void button8(Float value);

	abstract void button7(Float value);

	abstract void button6(Float value);

	abstract void button5(Float value);

	abstract void button4(Float value);

	abstract void button3(Float value);

	abstract void button2(Float value);

	abstract void button1(Float value);

	abstract void button0(Float value);

	private boolean isFarEnough(float a, float b) {
		return Math.abs(a - b) > getSensitivity();
	}

	private void checkController() {

		Controller[] controllers = ControllerEnvironment
				.getDefaultEnvironment().getControllers();

		for (int i = 0; i < controllers.length; i++) {

			System.out.println(controllers[i].getType());
			System.out.println(controllers[i].getName());

			if (controllers[i].getType().equals(Controller.Type.GAMEPAD)
					&& (controllers[i].getName().contains("Xbox 360") || controllers[i]
							.getName().contains("XBOX 360"))) {
				setCont(controllers[i]);
			}
		}
	}

	public void setCont(Controller cont) {
		this.cont = cont;
	}

	public Controller getCont() {
		return cont;
	}

	public float getxAxis() {
		return xAxis;
	}

	public void setxAxis(float xAxis) {
		this.xAxis = xAxis;
	}

	public float getyAxis() {
		return yAxis;
	}

	public void setyAxis(float yAxis) {
		this.yAxis = yAxis;
	}

	public float getxRotation() {
		return xRotation;
	}

	public void setxRotation(float xRotation) {
		this.xRotation = xRotation;
	}

	public float getyRotation() {
		return yRotation;
	}

	public void setyRotation(float yRotation) {
		this.yRotation = yRotation;
	}

	public float getzNegAxis() {
		return zNegAxis;
	}

	public void setzNegAxis(float zNegAxis) {
		this.zNegAxis = zNegAxis;
	}

	public float getzPosAxis() {
		return zPosAxis;
	}

	public void setzPosAxis(float zPosAxis) {
		this.zPosAxis = zPosAxis;
	}

	public void setSensitivity(double d) {
		this.sensitivity = d;
	}

	public double getSensitivity() {
		return sensitivity;
	}
}
