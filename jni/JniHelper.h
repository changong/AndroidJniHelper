
#ifndef __ANDROID_JNI_HELPER_H__
#define __ANDROID_JNI_HELPER_H__

#include "android/asset_manager.h"
#include <jni.h>
#include <string>
#include <map>
#include <vector>

using namespace std;

namespace AppCoder {

typedef struct JniMethodInfo_
{
    JNIEnv *    env;
    jclass      classID;
    jmethodID   methodID;
} JniMethodInfo;

class JniHelper
{
public:
    static void setJavaVM(JavaVM *javaVM);
    static JavaVM* getJavaVM();
    static JNIEnv* getEnv();

    static bool setClassLoaderFrom(jobject activityInstance);
    static bool getStaticMethodInfo(JniMethodInfo &methodinfo,
                                    const char *className,
                                    const char *methodName,
                                    const char *paramCode);
    static bool getMethodInfo(JniMethodInfo &methodinfo,
                              const char *className,
                              const char *methodName,
                              const char *paramCode);

    static jmethodID loadclassMethod_methodID;
    static jobject classloader;

    static void setassetmanager(AAssetManager* a);
    static AAssetManager* getAssetManager() { return assetmanager; }

public:
    // utils function
    static string jString2StdString(jstring str);
    static map<string, string> jHashMap2StdMap(jobject jhashmap);
    static vector<string> jVector2StdVector(jobject jVector);
    static vector<string> jArray2StdVector(jobject jArray);

    // create java object by call java function
    static void createHashMap();
    static jobject getHashMap();
    static void pushHashMapElement(string key, string value);

    static void createVector();
    static jobject getVector();
    static void pushVectorElement(string value);

    static void createArrayList();
    static jobject getArrayList();
    static void pushArrayListElement(string value);

    // convert std object to java object
    static jobject checkHashMap(map<string, string> stdMap);
    static jobject checkArrayList(vector<string> strings);
    static jobject checkVector(vector<string> strings);

    // show msg dialog
    static void showAlertDialog(string msg);
private:
    static JNIEnv* cacheEnv(JavaVM* jvm);

    static bool getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
                                                 const char *className,
                                                 const char *methodName,
                                                 const char *paramCode);
    static JavaVM* _psJavaVM;
    static AAssetManager* assetmanager;
};

}

#endif // __ANDROID_JNI_HELPER_H__
