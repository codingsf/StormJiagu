#pragma once
class loaddata
{
public:

	loaddata();
	~loaddata();
	
	static void run(JNIEnv* env, jobject obj, jobject ctx);
	static void attachContextBaseContext(JNIEnv* env, jobject obj, jobject ctx);
	static void loaddex(JNIEnv* env, jmethodID openDexFileNative, const char* data_filePath, int argSize, int dexnums, const char* cooketype, jobject/*for android 7.0*/ classLoader);
	static void makeDexElements(JNIEnv* env, jobject classLoader, jobject file);
	static jobject makeDexFileObject(JNIEnv* env, jint cookie, const char* filedir);
	static jobject makeDexFileObject_lollipop(JNIEnv* env, jlong cookie, const char* filedir);
	static jobject makeDexFileObject_marshmallow(JNIEnv* env, const char* filedir);
	static void dex_plan_one(JNIEnv*, jobject, jlong*);
	static void dex_plan_second(JNIEnv*, jobject, jlong*);
	static void myLoadDex_art(JNIEnv* env, char* szPath);
	static jlong* openFile(char*);
private:
	static int ExtractFile(JNIEnv* env, jobject ctx, const char* path);

};
