package com.storm.tools;

import android.content.Context;

public class Native {

	static {
        System.loadLibrary("dexload");
    }

    public Native() {
        super();
    }

    public static native void attachBaseContext( Context ctx) ;

    public static native boolean loadPluginDex(String arg1) ;

    public static native boolean loadXposedDex(String arg1, Object arg2);
}
