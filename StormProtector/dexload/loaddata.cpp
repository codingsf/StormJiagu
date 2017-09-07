#include "pch.h"
#include "loaddata.h"
#include "util.h"
#include "Messageprint.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cstdio>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "Hook.h"
#include <string>
#include <cstdlib>
#include "dexload.h"
#include "Davlikvm.h"
#include "Artvm.h"
#include "dexload.h"
#include "Security.h"

char* tag = "jack";
char* PackageFilePath;
char* PackageNames;
char* NativeLibDir;
bool haveHook=false;
const char* app_signature_sha1 = "61ED377E85D386A8DFEE6B864BD85B0BFAA5AF81";
const char HexCode[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
//for dvm
static Davlik* dvm_davlik;


loaddata::loaddata()
{
}


loaddata::~loaddata()
{
}


bool checkSignature(JNIEnv* env,jobject ctx)
{
	jclass context_class = env->GetObjectClass(ctx);

	//context.getPackageManager()
	jmethodID methodId = env->GetMethodID(context_class, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jobject package_manager_object = env->CallObjectMethod(ctx, methodId);
	if (package_manager_object == NULL) {
		Messageprint::printinfo("sign","getPackageManager() Failed!");
		return false;
	}

	//context.getPackageName()
	methodId = env->GetMethodID(context_class, "getPackageName", "()Ljava/lang/String;");
	jstring package_name_string = (jstring)env->CallObjectMethod(ctx, methodId);
	if (package_name_string == NULL) {
		Messageprint::printinfo("sign","getPackageName() Failed!");
		return false;
	}
	env->DeleteLocalRef(context_class);

	//PackageManager.getPackageInfo(Sting, int)
	//public static final int GET_SIGNATURES= 0x00000040;
	jclass pack_manager_class = env->GetObjectClass(package_manager_object);
	methodId = env->GetMethodID(pack_manager_class, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	env->DeleteLocalRef(pack_manager_class);
	jobject package_info_object = env->CallObjectMethod(package_manager_object, methodId, package_name_string, 0x40);
	if (package_info_object == NULL) {
		Messageprint::printinfo("sign", "getPackageInfo() Failed!");
		return false;
	}
	env->DeleteLocalRef(package_manager_object);

	//PackageInfo.signatures[0]
	jclass package_info_class = env->GetObjectClass(package_info_object);
	jfieldID fieldId = env->GetFieldID(package_info_class, "signatures", "[Landroid/content/pm/Signature;");
	env->DeleteLocalRef(package_info_class);
	
	//获得签名数组
	jobjectArray signature_object_array = (jobjectArray)env->GetObjectField(package_info_object, fieldId);
	if (signature_object_array == NULL) {
		Messageprint::printinfo("sign", "PackageInfo.signatures[] is null");
		return false;
	}

	//获得签名数组的第一位
	jobject signature_object = env->GetObjectArrayElement(signature_object_array, 0);
	env->DeleteLocalRef(package_info_object);

	//Signature.toByteArray()
	jclass signature_class = env->GetObjectClass(signature_object);
	methodId = env->GetMethodID(signature_class, "toByteArray", "()[B");
	env->DeleteLocalRef(signature_class);
	jbyteArray signature_byte = (jbyteArray)env->CallObjectMethod(signature_object, methodId);

	//new ByteArrayInputStream
	jclass byte_array_input_class = env->FindClass("java/io/ByteArrayInputStream");
	methodId = env->GetMethodID(byte_array_input_class, "<init>", "([B)V");
	jobject byte_array_input = env->NewObject(byte_array_input_class, methodId, signature_byte);

	//CertificateFactory.getInstance("X.509")
	jclass certificate_factory_class = env->FindClass("java/security/cert/CertificateFactory");
	methodId = env->GetStaticMethodID(certificate_factory_class, "getInstance", "(Ljava/lang/String;)Ljava/security/cert/CertificateFactory;");
	jstring x_509_jstring = env->NewStringUTF("X.509");
	jobject cert_factory = env->CallStaticObjectMethod(certificate_factory_class, methodId, x_509_jstring);

	//certFactory.generateCertificate(byteIn);
	methodId = env->GetMethodID(certificate_factory_class, "generateCertificate", ("(Ljava/io/InputStream;)Ljava/security/cert/Certificate;"));
	jobject x509_cert = env->CallObjectMethod(cert_factory, methodId, byte_array_input);
	env->DeleteLocalRef(certificate_factory_class);

	//cert.getEncoded()
	jclass x509_cert_class = env->GetObjectClass(x509_cert);
	methodId = env->GetMethodID(x509_cert_class, "getEncoded", "()[B");
	jbyteArray cert_byte = (jbyteArray)env->CallObjectMethod(x509_cert, methodId);
	env->DeleteLocalRef(x509_cert_class);

	//MessageDigest.getInstance("SHA1")
	jclass message_digest_class = env->FindClass("java/security/MessageDigest");
	methodId = env->GetStaticMethodID(message_digest_class, "getInstance", "(Ljava/lang/String;)Ljava/security/MessageDigest;");
	jstring sha1_jstring = env->NewStringUTF("SHA1");
	jobject sha1_digest = env->CallStaticObjectMethod(message_digest_class, methodId, sha1_jstring);

	//sha1.digest (certByte)
	methodId = env->GetMethodID(message_digest_class, "digest", "([B)[B");
	jbyteArray sha1_byte = (jbyteArray)env->CallObjectMethod(sha1_digest, methodId, cert_byte);
	env->DeleteLocalRef(message_digest_class);

	//toHexString
	jsize array_size = env->GetArrayLength(sha1_byte);
	jbyte* sha1 = env->GetByteArrayElements(sha1_byte, NULL);
	char *hex_sha = new char[array_size * 2 + 1];
	for (int i = 0; i <array_size; ++i) {
		hex_sha[2 * i] = HexCode[((unsigned char)sha1[i]) / 16];
		hex_sha[2 * i + 1] = HexCode[((unsigned char)sha1[i]) % 16];
	}
	hex_sha[array_size * 2] = '\0';
	Messageprint::printinfo("sign"," app current sign:%s ", hex_sha);

	


	//比较签名
	if (strcmp(hex_sha, app_signature_sha1) == 0)
	{
		Messageprint::printinfo("sign", "verify sign success");
		return true;
	}
	else {
		//Toast
		jstring str = env->NewStringUTF("verify sign failed");
		jclass tclss = env->FindClass("android/widget/Toast");
		jmethodID mid = env->GetStaticMethodID(tclss, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
		jobject job = env->CallStaticObjectMethod(tclss, mid, ctx, str, 1);
		jmethodID showId = env->GetMethodID(tclss, "show", "()V");
		env->CallVoidMethod(job, showId);
		
		Messageprint::printinfo("sign", "verify sign failed");
		return false;
	}
	
}

bool anti_debug()
{
	const int bufsize = 1024;
	char filename[bufsize];
	char line[bufsize];
	int pid = getpid();

	sprintf(filename, "/proc/%d/status", pid);
	FILE* fd = fopen(filename, "r");

	if (fd != NULL)
	{
		while (fgets(line, bufsize, fd))
		{
			if (strncmp(line, "TracerPid", 9) == 0)
			{
				int statu = atoi(&line[10]);
				if (statu != 0)
				{
					Messageprint::printerror("anti", "tracer anti");
					fclose(fd);
					raise(9);
					return false;
				}
				break;
			}
		}
	}
	return true;
}
/**
 * \brief called int attachContextBaseContext like MultiDex
 * \param env JNIEnv
 * \param obj Java Object
 * \param ctx Android Application
 */
hidden void loaddata::attachContextBaseContext(JNIEnv* env, jobject obj, jobject ctx)
{
	//检查签名
	if (!checkSignature(env, ctx))
	{	
		raise(9);
		return;
	}
	anti_debug();
	


	jclass ApplicationClass = env->GetObjectClass(ctx);
	jmethodID getFilesDir = env->GetMethodID(ApplicationClass, "getFilesDir", "()Ljava/io/File;");
	jobject File_obj = env->CallObjectMethod(ctx, getFilesDir);
	jclass FileClass = env->GetObjectClass(File_obj);
	//获取
	jmethodID getAbsolutePath = env->GetMethodID(FileClass, "getAbsolutePath", "()Ljava/lang/String;");
	jstring data_file_dir = static_cast<jstring>(env->CallObjectMethod(File_obj, getAbsolutePath));
	const char* cdata_file_dir = Util::jstringTostring(env, data_file_dir);
	//release
	env->DeleteLocalRef(data_file_dir);
	env->DeleteLocalRef(File_obj);
	env->DeleteLocalRef(FileClass);

	//NativeLibraryDir 获取lib所在文件夹
	jmethodID getApplicationInfo = env->GetMethodID(ApplicationClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
	jobject ApplicationInfo_obj = env->CallObjectMethod(ctx, getApplicationInfo);
	jclass ApplicationInfoClass = env->GetObjectClass(ApplicationInfo_obj);
	jfieldID nativeLibraryDir_field = env->GetFieldID(ApplicationInfoClass, "nativeLibraryDir", "Ljava/lang/String;");
	jstring nativeLibraryDir = static_cast<jstring>(env->GetObjectField(ApplicationInfo_obj, nativeLibraryDir_field));
	NativeLibDir = Util::jstringTostring(env, nativeLibraryDir);
	//释放
	env->DeleteLocalRef(nativeLibraryDir);
	env->DeleteLocalRef(ApplicationInfoClass);
	env->DeleteLocalRef(ApplicationInfo_obj);

	//获取apk 所在路径 方法getPackageResourcePath
	jmethodID getPackageResourcePath = env->GetMethodID(ApplicationClass, "getPackageResourcePath", "()Ljava/lang/String;");
	jstring mPackageFilePath = static_cast<jstring>(env->CallObjectMethod(ctx, getPackageResourcePath));
	const char* cmPackageFilePath = Util::jstringTostring(env, mPackageFilePath);

	PackageFilePath = const_cast<char*>(cmPackageFilePath);
	env->DeleteLocalRef(mPackageFilePath);

	//获取包名
	jmethodID getPackageName = env->GetMethodID(ApplicationClass, "getPackageName", "()Ljava/lang/String;");
	jstring PackageName = static_cast<jstring>(env->CallObjectMethod(ctx, getPackageName));
	const char* packagename = Util::jstringTostring(env, PackageName);
	PackageNames = (char*)packagename;
	env->DeleteLocalRef(PackageName);

	//重点 拿到context的ClassLoader
	jmethodID getClassLoader = env->GetMethodID(ApplicationClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
	jobject classLoader = env->CallObjectMethod(ctx, getClassLoader);

	char codePath[256] = {0};

	//codePath目录:包名/files/code/
	sprintf(codePath, "%s/%s", cdata_file_dir, "code");
	//导出dex文件并获取个数或者获取已经导出dex文件个数；
	//将Assets目录下的encrypted.dex提取到codePath目录 并获取解压到这个目录下的的个数
	int dexnums = ExtractFile(env, ctx, codePath);
	__android_log_print(ANDROID_LOG_ERROR,"storm", "dexnums:%d",dexnums);

	if (dexnums <= 0)
	{
		//Messageprint::printinfo("loaddex", "dexnums:%d", dexnums);
		return;
	}

	//加载dex 
	jclass DexFile = env->FindClass("dalvik/system/DexFile");//ClassName[1] dalvik/system/DexFile
	jfieldID mCookie;
	std::string cookietype = Util::getmCookieType(env);
	//针对mCookie 值的类型不同不对版本进行区分 
	mCookie = env->GetFieldID(DexFile, "mCookie", cookietype.c_str());

	MethodSign method_sign = Util::getMehodSign(env, "dalvik.system.DexFile", "loadDex");
	//get loadDex file Methodid 
	jmethodID openDexFileNative = nullptr;
	//for art
	if (isArt)
	{
		Messageprint::printerror("storm","enter Art mode");
		openDexFileNative = env->GetStaticMethodID(DexFile, "loadDex", method_sign.sign.c_str());
		//get and hook some function
		//Artvm::hookstart();
		//Artvm::hookEnable(false);
	}
	else
	{
		openDexFileNative = env->GetStaticMethodID(DexFile, "loadDex", method_sign.sign.c_str());
		// get and hook some function
		dvm_davlik = Davlik::initdvm();
	}
	loaddex(env, openDexFileNative, cdata_file_dir, method_sign.argSize, dexnums, cookietype.c_str(), classLoader);
}


void loaddata::myLoadDex_art(JNIEnv* env, char* szPath)
{

	

}


/**
 * \brief extract dex file from Assets
 * \param env JNIEnv
 * \param ctx Application Context
 * \param path extract file path
 * \return  number of dex files
 */
hidden int loaddata::ExtractFile(JNIEnv* env, jobject ctx, const char* path)
{
	if (access(path, F_OK) == -1)
	{
		mkdir(path, 505);
		chmod(path, 505);
		//拿到AAssetManager
		AAssetManager* mgr;
		jclass ApplicationClass = env->GetObjectClass(ctx);
		jmethodID getAssets = env->GetMethodID(ApplicationClass, "getAssets", "()Landroid/content/res/AssetManager;");
		jobject Assets_obj = env->CallObjectMethod(ctx, getAssets);
		mgr = AAssetManager_fromJava(env, Assets_obj);
		if (mgr == nullptr)
		{
			Messageprint::printerror("ExtractFile", "AAssetManager_fromJava fail");
			return 0;
		}
		//历遍根assets目录
		AAssetDir* dirs = AAssetManager_openDir(mgr, "");
		//AAsset* asset = AAssetManager_open(mgr, "dump.dex", AASSET_MODE_STREAMING);
		const char* FileName;
		int i = 0;
		while ((FileName = AAssetDir_getNextFileName(dirs)) != nullptr)
		{
			Messageprint::printinfo("dvm", "FileName:%s", FileName);
			if (strstr(FileName, "encrypt") != nullptr && strstr(FileName, "data") != nullptr)
			{
				AAsset* asset = AAssetManager_open(mgr, FileName, AASSET_MODE_STREAMING);
				FILE* file;
				void* buffer;
				int numBytesRead;
				if (asset != nullptr)
				{
					char szDexPath[256] = { 0 };
					//path为/files/code/目录
					sprintf(szDexPath, "%s/encrypt_%d.dex", path,i);
					Messageprint::printerror("dvm", "first dexpath-->%s", szDexPath);
					file = fopen(szDexPath, "wb");
					int bufferSize = AAsset_getLength(asset); 
					//       LOGI("buffersize is %d",bufferSize);
					Messageprint::printinfo("dvm", "Asset FileName:%s,Extract path:%s,size:%d\n",FileName,szDexPath, bufferSize);

					buffer = malloc(4096);
					if (!isArt)
					{
						//dvm下偏移了292个字节
						//AAsset_seek(asset, 292, SEEK_SET);
					}

					while (true)
					{
						numBytesRead = AAsset_read(asset, buffer, 4096);
						if (numBytesRead <= 0)
							break;
						fwrite(buffer, numBytesRead, 1, file);
					}
					free(buffer);
					fclose(file);
					AAsset_close(asset);
					i = i + 1;
					chmod(szDexPath, 493);
				}
				else
				{
					Messageprint::printerror("ExtractFile", "AAsset is null :%s", FileName);
				}
			}
		}
		return i;
	}
	else//获取dex数目
	{
		DIR* dir = opendir(path);
		struct dirent* direntp;
		int i = 0;
		if (dir != nullptr)
		{
			for (;;)
			{
				direntp = readdir(dir);
				if (direntp == nullptr) break;
				//printf("%s\n", direntp->d_name);
				if (strstr(direntp->d_name, "encrypt") != nullptr && strstr(direntp->d_name, "dex") != nullptr)
				{
					i = i + 1;
				}
			}
			closedir(dir);
			return i;
		}
		//Messageprint::printinfo("ExtractFile", "dir existed");
	}

	return 0;
}


/**
 * \brief Load DexFiles 
 * \param env JNIEnv* env
 * \param loadDex   Java method id
 * \param data_filePath Application Data folder path
 * \param argSize  Size of DexFile.loadDex method arg size
 * \param dexnums   number of dex files
 * \param cooketype int or long or Object
 * \param classLoader  BaseDexClassLoader Object
 */
hidden void loaddata::loaddex(JNIEnv* env, jmethodID loadDex, const char* data_filePath, int argSize, int dexnums, const char* cooketype, jobject/*for android 7.0*/ classLoader)
{
	
	char* coptdir = new char[256];
	char* codePath = new char[256];
	memset(coptdir, 0, 256);
	memset(codePath, 0, 256);

	Messageprint::printerror("storm", "cookietype:%s", cooketype);

	//coptdir目录:/包名/optdir/
	sprintf(coptdir, "%s/%s", data_filePath, "optdir");
	//codePath目录:/包名/code/
	sprintf(codePath, "%s/%s", data_filePath, "code");

	if (access(coptdir, F_OK) == -1)
	{
		mkdir(coptdir, 505);
		chmod(coptdir, 505);
	}

	
	jclass DexFile = env->FindClass("dalvik/system/DexFile");
	MethodSign method_sign = Util::getMehodSign(env, "dalvik.system.DexFile", "openDexFile");
	Messageprint::printerror("storm", "openDexFile sign:%s", method_sign.sign.c_str());
	jmethodID openDexFile = env->GetStaticMethodID(DexFile, "openDexFile", method_sign.sign.c_str());
	if (openDexFile == NULL)
	{
		Messageprint::printerror("storm", "get openDexFile methodId failed");
		return;
	}
	
	char android_buildversion[0x100] = { 0 };

	Util::get_system_property("ro.build.version.release",android_buildversion);
	Messageprint::printerror("storm", "build version:%s",android_buildversion);

	Util::get_system_property("ro.build.version.sdk", android_buildversion);
	Messageprint::printerror("storm", "sdk version:%d", atoi(android_buildversion));

	//在Android4.4下 mCookie的类型为"I"
	if (strcmp(cooketype, "I") == 0)
	{
		// art or dvm android 4.4 have art 
		for (int i = 0; i < dexnums; ++i)
		{
			//for dvm
			char szDexPath[256] = { 0 };
			//szDexPath：加密的dex路径
			sprintf(szDexPath, "%s/code/encrypt_%d.dex", data_filePath, i);
			if (dvm_davlik->initOk)
			{
				jint mcookie;
				if (dvm_davlik->loaddex(szDexPath, mcookie))
				{
					Messageprint::printerror("dvm", "start dex success");
					jobject dexfileobj = makeDexFileObject(env, mcookie, data_filePath);
					makeDexElements(env, classLoader, dexfileobj);
					Messageprint::printerror("dvm", "Multi Success");
				}
				//load fail
				else
				{
					Messageprint::printinfo("loaddex", "load fail");
				}
			}
			//init fail
			else
			{
				Messageprint::printerror("loaddex", "init dvm fail");
			}
			


		}
	}

	else if (strcmp(cooketype, "J") == 0 )
	{ //only art
		for (int i = 0; i < dexnums; ++i)
		{			
			char szDexPath[256] = { 0 };
			//szDexPath：加密的dex路径
			sprintf(szDexPath, "%s/code/encrypt_%d.dex", data_filePath, i);
			jstring inPath = env->NewStringUTF(szDexPath);

			jlong mCookie = env->CallStaticLongMethod(DexFile, openDexFile, inPath, 0, 0);			
			jobject dexfileobj = makeDexFileObject_lollipop(env, mCookie, data_filePath);
			Messageprint::printerror("storm", "call makeDexFileObject_lollipop finished");
			makeDexElements(env, classLoader, dexfileobj);
	
		}
	}

	else if (strcmp(cooketype, "Ljava/lang/Object;") == 0)
	{
		for (int i = 0; i < dexnums; ++i)
		{
			char szDexPath[256] = { 0 };
			//szDexPath：加密的dex路径
			sprintf(szDexPath, "%s/code/encrypt_%d.dex", data_filePath, i);
			jstring inPath = env->NewStringUTF(szDexPath);
			Messageprint::printerror("storm", "real dex path:%s",szDexPath);
			
			jlong* target_dexinfo_array= openFile(szDexPath);

			
			int ptr = *(int*)target_dexinfo_array;
			int dexbase = *(int*)(ptr + 4);
			int dexsize= *(int*)(ptr + 8);

			Messageprint::printerror("storm", "target dex dexbase:%x,dexsize:%x", *(int*)dexbase,dexsize);

			jobject minidexfileobj = makeDexFileObject_marshmallow(env, data_filePath);

			//dex_plan_one(env, minidexfileobj, target_dexinfo_array);

		    dex_plan_second(env, minidexfileobj, target_dexinfo_array);

		
			makeDexElements(env, classLoader, minidexfileobj);

		}
	}


	
}
void loaddata::dex_plan_one(JNIEnv* env, jobject minidexfileobj, jlong* target_dexinfo)
{
	jclass DexFileClass = env->FindClass("dalvik/system/DexFile");//"dalvik/system/DexPathList$Element"
	jfieldID field_mCookie = env->GetFieldID(DexFileClass, "mCookie", "Ljava/lang/Object;");
	jobject mCookie = env->GetObjectField(minidexfileobj, field_mCookie);

	jboolean is_data_copy = 1;
	//返回指向数组元素的指针

	jsize arraylen = env->GetArrayLength((jarray)mCookie);
	jlong* mix_element = env->GetLongArrayElements((jlongArray)mCookie, &is_data_copy);

	//数组的第一个成员
	int ptr = *(int*)mix_element;
	int dexbase = *(int*)(ptr + 4);
	int dexsize = *(int*)(ptr + 8);
	Messageprint::printerror("storm", "array len :%d,first minidex dex dexbase:%x,dexsize:%x", arraylen, *(int*)dexbase, dexsize);


	// very important
	for (int i = 0; i < arraylen; i++)
	{
		*mix_element = *target_dexinfo;
		mix_element++;
		target_dexinfo++;
	}
	
	env->ReleaseLongArrayElements((jlongArray)mCookie, mix_element, 0);

	/*
	jlong* second_mix_element = env->GetLongArrayElements((jlongArray)mCookie, &is_data_copy);
	ptr = *(int*)second_mix_element;
	dexbase = *(int*)(ptr + 4);
	dexsize = *(int*)(ptr + 8);
	Messageprint::printerror("storm", "second minidex dex dexbase:%x,dexsize:%x", *(int*)dexbase, dexsize);
	*/
	Messageprint::printerror("storm", "plan one successful");
}

void loaddata::dex_plan_second(JNIEnv* env, jobject minidexfileobj, jlong* target_dexinfo)
{
	//阅读DexFile_openFileNative函数发现 mCookie其实是一个jlongarray数组
	//构造jlongarray 也就是构造目标dex的cookie
	//构造过程可参考Android 6.0下DexFile_openFileNative函数中的ConvertNativeToJavaArray
	jlongArray long_array = env->NewLongArray(1);
	jboolean is_long_data_copied;
	jlong* long_data = env->GetLongArrayElements(long_array, &is_long_data_copied);
	jlong* tmp = long_data;
	*tmp = *target_dexinfo;
	env->ReleaseLongArrayElements(long_array, long_data, 0);


	//REPLACE COOKIE
	jclass DexFileClass = env->FindClass("dalvik/system/DexFile");//"dalvik/system/DexPathList$Element"
	jfieldID field_mCookie = env->GetFieldID(DexFileClass, "mCookie", "Ljava/lang/Object;");
	env->SetObjectField(minidexfileobj, field_mCookie, long_array);
	Messageprint::printerror("storm", "plan second successful");

}

jlong* loaddata::openFile(char* realDexPath)
{
	void *handle_art = dlopen("libart.so", RTLD_NOW);
	void *OpenFile = dlsym(handle_art, "_ZN3art7DexFile8OpenFileEiPKcbPNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE");
	if (!OpenFile)
	{
		Messageprint::printerror("storm","dlsym art_OpenFile failed");
		return 0;
	}
	typedef jlong* (*OpenFile_type)(void*, int, const char*, bool, std::string*);
	OpenFile_type OpenFile_func = (OpenFile_type)OpenFile;

	int fd = open(realDexPath, 0);
	if (fd == -1)
	{
		Messageprint::printerror("storm", "open oat file failed");
		return 0;
	}
	void* buffer = malloc(0x100);
	std::string error_msg;
	//(int fd, const char* location, bool verify,void* error_msg)
	jlong* dex_cookie = OpenFile_func(buffer, fd, realDexPath, false, &error_msg);
	if (!dex_cookie)
	{
		Messageprint::printerror("storm", "OpenFile failed");
		return 0;
	}
	Messageprint::printerror("storm", "OpenFile successful");
	dlclose(handle_art);

	return dex_cookie;
}
/**
 * \brief just for dvm ,load a mini dex file 
 * \param env 
 * \param cookie 
 * \param filedir 
 * \return DexFile object
 */
hidden jobject loaddata::makeDexFileObject(JNIEnv* env, jint cookie, const char* filedir)
{
	char* in = new char[256];
	char* out = new char[256];
	memset(in, 0, 256);
	memset(out, 0, 256);
	//in目录:/data/data/包名/files/code/mini.dex
	sprintf(in, "%s/%s/%s", filedir, "code", "mini.dex");
	sprintf(out, "%s/%s/%s", filedir, "optdir", "mini.odex");

	//写入minidex
	dvm_davlik->writeminidex(in);

	jclass DexFileClass = env->FindClass("dalvik/system/DexFile");//"dalvik/system/DexPathList$Element"
	jmethodID init = env->GetMethodID(DexFileClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;I)V");
	jstring apk = env->NewStringUTF(in);
	jstring odex = env->NewStringUTF(out);
	//创造一个空的dex对象
	jobject dexobj = env->NewObject(DexFileClass, init, apk, odex, 0);//暂时不用释放了

	//将要加载的dex的mCookie写入到这个空的dex的mCookie成员中
	jfieldID mCookie = env->GetFieldID(DexFileClass, "mCookie", "I");
	env->SetIntField(dexobj, mCookie, cookie);

	env->DeleteLocalRef(DexFileClass);
	env->DeleteLocalRef(apk);
	env->DeleteLocalRef(odex);
	delete[]in;
	delete[]out;
	return dexobj;
}

hidden jobject loaddata::makeDexFileObject_lollipop(JNIEnv* env, jlong cookie, const char* filedir)
{
	char* in = new char[256];
	char* out = new char[256];
	memset(in, 0, 256);
	memset(out, 0, 256);
	//in目录:/data/data/包名/files/code/mini.dex
	sprintf(in, "%s/%s/%s", filedir, "code", "mini.dex");
	sprintf(out, "%s/%s/%s", filedir, "optdir", "mini.odex");

	//写入minidex
	dvm_davlik->writeminidex(in);

	jclass DexFileClass = env->FindClass("dalvik/system/DexFile");//"dalvik/system/DexPathList$Element"
	jmethodID init = env->GetMethodID(DexFileClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;I)V");
	jstring apk = env->NewStringUTF(in);
	jstring odex = env->NewStringUTF(out);
	//创造一个空的dex对象
	jobject dexobj = env->NewObject(DexFileClass, init, apk, odex, 0);//暂时不用释放了

																	  //将要加载的dex的mCookie写入到这个空的dex的mCookie成员中
	jfieldID mCookie = env->GetFieldID(DexFileClass, "mCookie", "J");
	env->SetLongField(dexobj, mCookie, cookie);

	env->DeleteLocalRef(DexFileClass);
	env->DeleteLocalRef(apk);
	env->DeleteLocalRef(odex);
	delete[]in;
	delete[]out;
	return dexobj;
}

hidden jobject loaddata::makeDexFileObject_marshmallow(JNIEnv* env, const char* filedir)
{
	char* in = new char[256];
	char* out = new char[256];
	memset(in, 0, 256);
	memset(out, 0, 256);
	//in目录:/data/data/包名/files/code/mini.dex
	sprintf(in, "%s/%s/%s", filedir, "code", "mini.dex");
	sprintf(out, "%s/%s/%s", filedir, "optdir", "mini.odex");

	//写入minidex
	dvm_davlik->writeminidex(in);


	//static public DexFile loadDex(String sourcePathName, String outputPathName,
	//142        int flags) throws IOException {
	//143

	        /*
	145         * TODO: we may want to cache previously-opened DexFile objects.
	146         * The cache would be synchronized with close().  This would help
	147         * us avoid mapping the same DEX more than once when an app
	148         * decided to open it multiple times.  In practice this may not
	149         * be a real issue.
	150         */
	//151        return new DexFile(sourcePathName, outputPathName, flags);


	jclass DexFileClass = env->FindClass("dalvik/system/DexFile");//"dalvik/system/DexPathList$Element"
	jmethodID init = env->GetMethodID(DexFileClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;I)V");
	jstring apk = env->NewStringUTF(in);
	jstring odex = env->NewStringUTF(out);
	//返回一个mini.dex DexFile对象
	jobject dexobj = env->NewObject(DexFileClass, init, apk, odex, 0);//暂时不用释放了

	//将要加载的dex的mCookie写入到这个空的dex的mCookie成员中
	//jfieldID mCookie = env->GetFieldID(DexFileClass, "mCookie", "Ljava/lang/Object;");
	//env->SetObjectField(dexobj, mCookie, cookie);

	//jfieldID field_mCookie=env->GetFieldID(DexFileClass ,"mCookie", "Ljava/lang/Object;");
	//jobject mix_mCookie=env->GetObjectField(dexobj,field_mCookie);

	//jboolean tmp=1;
	//jlong* mix_element=env->GetLongArrayElements((jlongArray)mix_mCookie,&tmp);
	//*mix_element=*(jlong*)cookie;

	//Messageprint::printerror("storm", "replace mCookie successful");

	env->DeleteLocalRef(DexFileClass);
	env->DeleteLocalRef(apk);
	env->DeleteLocalRef(odex);
	delete[]in;
	delete[]out;
	return dexobj;
}


/**
  * \brief makeDexElements and add this obj 
  * \param env 
  * \param classLoader Class loader
  * \param dexFileobj 
  */
hidden void loaddata::makeDexElements(JNIEnv* env, jobject classLoader, jobject dexFileobj)
{
	jclass PathClassLoader = env->GetObjectClass(classLoader);

	jclass BaseDexClassLoader = env->GetSuperclass(PathClassLoader);


	//get pathList fieldid
	jfieldID pathListid = env->GetFieldID(BaseDexClassLoader, "pathList", "Ldalvik/system/DexPathList;");
	jobject pathList = env->GetObjectField(classLoader, pathListid);

	//get DexPathList Class 
	jclass DexPathListClass = env->GetObjectClass(pathList);
	//get dexElements fieldid
	jfieldID dexElementsid = env->GetFieldID(DexPathListClass, "dexElements", "[Ldalvik/system/DexPathList$Element;");

	//获取elements数组 get dexElement array value
	jobjectArray dexElement = static_cast<jobjectArray>(env->GetObjectField(pathList, dexElementsid));


	//获取数组的个数 get DexPathList$Element Class construction method and get a new DexPathList$Element object 
	jint len = env->GetArrayLength(dexElement);
	Messageprint::printinfo("dvm", "未加载之前的Elements个数:%d", len);

	jclass ElementClass = env->FindClass("dalvik/system/DexPathList$Element");// dalvik/system/DexPathList$Element
	jmethodID Elementinit = env->GetMethodID(ElementClass, "<init>", "(Ljava/io/File;ZLjava/io/File;Ldalvik/system/DexFile;)V");
	jboolean isDirectory = JNI_FALSE;

	//创建一个新的dalvik/system/DexPathList$Element类 dexFileobj为新的dexFileobj
	jobject element_obj = env->NewObject(ElementClass, Elementinit, nullptr, isDirectory, nullptr, dexFileobj);

	//Get dexElement all values and add  add each value to the new array
	jobjectArray new_dexElement = env->NewObjectArray(len + 1, ElementClass, nullptr);
	for (int i = 0; i < len; ++i)
	{
		//将以前的Elements添加到这个新的new_dexElement数组
		env->SetObjectArrayElement(new_dexElement, i, env->GetObjectArrayElement(dexElement, i));
	}

	//将要加载的element_obj放在新数组的最后一个成员里
	env->SetObjectArrayElement(new_dexElement, len, element_obj);

	env->SetObjectField(pathList, dexElementsid, new_dexElement);

	env->DeleteLocalRef(element_obj);
	env->DeleteLocalRef(ElementClass);
	env->DeleteLocalRef(dexElement);
	env->DeleteLocalRef(DexPathListClass);
	env->DeleteLocalRef(pathList);
	env->DeleteLocalRef(BaseDexClassLoader);
	env->DeleteLocalRef(PathClassLoader);
}
