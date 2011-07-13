package Control;

import java.io.IOException;

public class XControllerTester extends XController {

	public XControllerTester() {

	}

	public void yRotation(Float value) {
		System.out.println("yRotation " + value);
	}

	public void xRotation(Float value) {
		System.out.println("xRotation " + value);
	}

	public void zPosAxis(Float value) {
		System.out.println("Zpos " + value);
	}
	
	public void zNegAxis(Float value) {
		System.out.println("Zneg " + value);
	}
	
	public void yAxis(Float value) {
		System.out.println("yAxis " + value);
	}

	public void xAxis(Float value) {
		System.out.println("xAxis " + value);
	}

	public void button9(Float value) {
	}

	public void button8(Float value) {
	}

	public void button7(Float value) {
	}

	public void button6(Float value) {
	}

	public void button5(Float value) {
	}

	public void button4(Float value) {
	}

	public void button3(Float value) {
	}

	public void button2(Float value) {
	}

	public void button1(Float value) {
	}

	public void button0(Float value) {
		System.out.println("B0 " + value);
	}

	public static void main(String[] args) throws IOException {
		@SuppressWarnings("unused")
		XControllerTester cont = new XControllerTester();
	}

}
