package com.appcoder.lib;

import java.util.HashMap;
public class JniTest {
	public static void setName(String name,int age){
		System.err.println("setName the name="+name+",age="+age);
	}
	
	public static void setIsCoder(boolean isCoder){
		System.err.println("setIsCoder isCoder="+isCoder);
	}
	
	public static void setSalary(float salary){
		System.err.println("setSalary salary="+salary);
	}
	
	public static void setUserInfo(HashMap<String,String>hasMap){
		System.err.println("setUserInfo =" + hasMap);
	}
	
	public static void testCallNativeInJavaSubThread(){
		Runnable r = new Runnable() {
			@Override
			public void run() {
				testCallNative();
			}
		};
		
		Thread subThread = new Thread(r);
		subThread.start();
	}
	
	
	public static native void testCallNative();
	public static native void testNatvieSubThread();
}
