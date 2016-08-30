package com.appcoder.lib;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Vector;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.res.AssetManager;

/**
 * JNI工具类：用于Java与C++通信
 * 
 * @author changong (http://blog.csdn.net/m_changgong)
 * @created 2016-06-22
 * @微信 AppCoder
 */
public class JniHelper {
	private static Activity mActivity;
	private static AssetManager assetManager;
	
	private static HashMap<String, String> mHashMap = null;
	private static Vector<String> mVector = null;
	private static ArrayList<String> mArrayList = null;
	
	public static void init(final Activity activity) {
		mActivity = activity;
		
		JniHelper.assetManager = activity.getAssets();
		JniHelper.nativeSetContext((Context) activity,
				JniHelper.assetManager);
	}
	
	public static void createHashMap(){
		mHashMap = new HashMap<String, String>();
	}
	
	public static HashMap<String, String> getHashMap(){
		return mHashMap;
	}
	
	public static void pushHashMapElement(String key, String value){
		if(mHashMap == null)
			return;
		
		mHashMap.put(key, value);
	}
	
	public static void createVector(){
		mVector = new Vector<String>();
	}
	
	public static Vector<String> getVector(){
		return mVector;
	}
	
	public static void pushVectorElement(String value){
		if(mVector == null)
			return;
		
		mVector.add(value);
	}
	
	public static void createArrayList(){
		mArrayList = new ArrayList<String>();
	}
	
	public static ArrayList<String> getArrayList(){
		return mArrayList;
	}
	
	public static void pushArrayListElement(String value){
		if(mArrayList == null)
			return;
		
		mArrayList.add(value);
	}
	
	// demo中提示调用的对话框
	public static void showAlertDialog(final String msg) {
		mActivity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				new AlertDialog.Builder(mActivity).setMessage(msg)
						.setPositiveButton("确定", new OnClickListener() {

							@Override
							public void onClick(DialogInterface dialog,
									int which) {
								dialog.dismiss();
							}
						}).show();
			}
		});
	}
	
	private static native void nativeSetContext(final Context pContext,
			final AssetManager pAssetManager);
}
