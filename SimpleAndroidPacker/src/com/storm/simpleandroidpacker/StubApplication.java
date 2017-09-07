package com.storm.simpleandroidpacker;

import com.storm.tools.Native;

import android.app.Application;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;

public class StubApplication extends Application {
	
	  public StubApplication() {
	        super();
	    }

	  protected void attachBaseContext(Context ctx) {
        super.attachBaseContext(ctx);
        Toast.makeText(ctx, "test", Toast.LENGTH_LONG).show();
        Log.d("storm","StubApplication enter");
        System.out.println("start:" + System.currentTimeMillis());
        Native.attachBaseContext(ctx);
	  }

	    public void onCreate() {
	        super.onCreate();
	        
	        Log.d("storm","StubApplication enter");
	        System.out.println("end:" + System.currentTimeMillis());
	    }
	    Toast aa(Context ctx,CharSequence a,int b){
	    	Toast test=Toast.makeText(ctx, "123", Toast.LENGTH_LONG);
	    	test.show();
	    	return test;
	    }
	    
}
