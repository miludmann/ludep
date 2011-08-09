package test;

import java.util.ArrayList;

public class flockTest {
	
	ArrayList<Integer> listInt;
	
	public flockTest(ArrayList<Integer> tab)
	{
		listInt = tab;
		listAllPermutations();
	}
	
	public void listAllPermutations()
	{
		recursiveListing(listInt, new ArrayList<Integer>());
	}
	
	public void recursiveListing(ArrayList<Integer> initTab,
								 ArrayList<Integer> result)
	{
		if ( initTab.size()-result.size() == 0 )
			System.out.println(result.toString());
		
		for (int i = 0; i < initTab.size() ; i++) {
			
			if(! result.contains((Object) i) )
			{
				result.add(i);
				recursiveListing(initTab, result);
				result.remove((Object) i);
			}
		}
	}
	
	
	public static void main(String [] args )
	{
		ArrayList<Integer> tab = new ArrayList<Integer>();
		
		for (int i = 0; i < 4; i++) {
			tab.add(i);
		}
		
		new flockTest(tab);
	}
}
