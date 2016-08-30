package com.appcoder.jni.demo;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.appcoder.lib.JniHelper;
import com.appcoder.lib.JniTest;
import com.appcoder.lib.Logger;


public class JniDemoActivity extends Activity {
	private Button btnBasic = null;
	private Button btnThread = null;
	
    @Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		try {
			Logger.info("load JNIDemo");
			System.loadLibrary("JniDemo");
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		//JniHelper初始化
		JniHelper.init(this);

		initView();
	}
    
    private void initView(){
    	btnBasic = (Button)findViewById(R.id.btn_basic);
    	btnThread =  (Button)findViewById(R.id.btn_thread);
    			
    	class MyClickListener implements OnClickListener{
    		public void onClick(View v) {
    			switch(v.getId()){
    			case R.id.btn_basic:
    				JniTest.testCallNative();
    				break;
    			case R.id.btn_thread:
    				JniTest.testNatvieSubThread();
    				break;
    			default:
    				break;
    			}
    		}
    	}
    	
    	btnBasic.setOnClickListener(new MyClickListener());
    	btnThread.setOnClickListener(new MyClickListener());
    }

    @Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
