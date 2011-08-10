package test;

import java.util.Random;

import Flock.Geometry;

public class flockTest {

	public static void main(String [] args )
	{
		System.out.println(Geometry.isLineIntersectingLine(15, 0, 10, 0, 5, 5, -5, -5));
		
		Random generator = new Random();
		
		while (true) {
			
			try {
				Thread.sleep(250);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			System.out.println(generator.nextInt(1));
		}
	}
}
