package com.appcoder.lib;

import android.util.Log;

public class Logger {
	private static boolean DEBUG = true;
	private static String TAG = "Logger";
	
	public static void info(String msg){
		if(DEBUG){
			Log.d(TAG,msg);
		}
	}
	
	public static void error(String msg){
		if(DEBUG){
			Log.e(TAG,msg);
		}
	}
}
