
#include "JniHelper.h"
#include "android/asset_manager.h"
#include "android/asset_manager_jni.h"
#include "Log.h"

#include <string.h>
#include <pthread.h>

#define JNIHELPER_JAVA_CLASS "com/appcoder/lib/JniHelper"

AAssetManager* AppCoder::JniHelper::assetmanager = nullptr;

static pthread_key_t g_key;

jclass _getClassID(const char *className) {
    if (nullptr == className) {
        return nullptr;
    }

    JNIEnv* env = AppCoder::JniHelper::getEnv();

    jstring _jstrClassName = env->NewStringUTF(className);

    jclass _clazz = (jclass) env->CallObjectMethod(AppCoder::JniHelper::classloader,
                                                   AppCoder::JniHelper::loadclassMethod_methodID,
                                                   _jstrClassName);

    if (nullptr == _clazz) {
        LOGE("Classloader failed to find class of %s", className);
        env->ExceptionClear();
    }

    env->DeleteLocalRef(_jstrClassName);
        
    return _clazz;
}

void _detachCurrentThread(void* a) {
    AppCoder::JniHelper::getJavaVM()->DetachCurrentThread();
}

namespace AppCoder {

    JavaVM* JniHelper::_psJavaVM = nullptr;
    jmethodID JniHelper::loadclassMethod_methodID = nullptr;
    jobject JniHelper::classloader = nullptr;

    JavaVM* JniHelper::getJavaVM() {
        pthread_t thisthread = pthread_self();
        LOGD("JniHelper::getJavaVM(), pthread_self() = %ld", thisthread);
        return _psJavaVM;
    }

    void JniHelper::setJavaVM(JavaVM *javaVM) {
        pthread_t thisthread = pthread_self();
        LOGD("JniHelper::setJavaVM(%p), pthread_self() = %ld", javaVM, thisthread);
        _psJavaVM = javaVM;

        pthread_key_create(&g_key, _detachCurrentThread);
    }

    JNIEnv* JniHelper::cacheEnv(JavaVM* jvm) {
        JNIEnv* _env = nullptr;
        // get jni environment
        jint ret = jvm->GetEnv((void**)&_env, JNI_VERSION_1_4);
        
        switch (ret) {
        case JNI_OK :
            // Success!
            pthread_setspecific(g_key, _env);
            return _env;
                
        case JNI_EDETACHED :
            // Thread not attached
            if (jvm->AttachCurrentThread(&_env, nullptr) < 0)
                {
                    LOGE("Failed to get the environment using AttachCurrentThread()");

                    return nullptr;
                } else {
                // Success : Attached and obtained JNIEnv!
                pthread_setspecific(g_key, _env);
                return _env;
            }
                
        case JNI_EVERSION :
            // Cannot recover from this error
            LOGE("JNI interface version 1.4 not supported");
        default :
            LOGE("Failed to get the environment using GetEnv()");
            return nullptr;
        }
    }

    JNIEnv* JniHelper::getEnv() {
        JNIEnv *_env = (JNIEnv *)pthread_getspecific(g_key);
        if (_env == nullptr)
            _env = JniHelper::cacheEnv(_psJavaVM);
        return _env;
    }

    bool JniHelper::setClassLoaderFrom(jobject activityinstance) {
        JniMethodInfo _getclassloaderMethod;
        if (!JniHelper::getMethodInfo_DefaultClassLoader(_getclassloaderMethod,
                                                         "android/content/Context",
                                                         "getClassLoader",
                                                         "()Ljava/lang/ClassLoader;")) {
            return false;
        }

        jobject _c = AppCoder::JniHelper::getEnv()->CallObjectMethod(activityinstance,
                                                                    _getclassloaderMethod.methodID);

        if (nullptr == _c) {
            return false;
        }

        JniMethodInfo _m;
        if (!JniHelper::getMethodInfo_DefaultClassLoader(_m,
                                                         "java/lang/ClassLoader",
                                                         "loadClass",
                                                         "(Ljava/lang/String;)Ljava/lang/Class;")) {
            return false;
        }

        JniHelper::classloader = AppCoder::JniHelper::getEnv()->NewGlobalRef(_c);
        JniHelper::loadclassMethod_methodID = _m.methodID;

        return true;
    }

    bool JniHelper::getStaticMethodInfo(JniMethodInfo &methodinfo,
                                        const char *className, 
                                        const char *methodName,
                                        const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            LOGE("Failed to get JNIEnv");
            return false;
        }
            
        jclass classID = _getClassID(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetStaticMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find static method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }
            
        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;
        return true;
    }

    bool JniHelper::getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
                                                     const char *className,
                                                     const char *methodName,
                                                     const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return false;
        }

        jclass classID = env->FindClass(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

    bool JniHelper::getMethodInfo(JniMethodInfo &methodinfo,
                                  const char *className,
                                  const char *methodName,
                                  const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return false;
        }

        jclass classID = _getClassID(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

    void JniHelper::setassetmanager(AAssetManager* a) {
        if (nullptr == a) {
            LOGD("setassetmanager : received unexpected nullptr parameter");
            return;
        }

        AppCoder::JniHelper::assetmanager = a;
    }

    //======================================================================
    // utils functions
    //======================================================================
    string JniHelper::jString2StdString(jstring jstr) {
        if (jstr == nullptr) {
            return "";
        }
        
        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return nullptr;
        }

        const char* chars = env->GetStringUTFChars(jstr, nullptr);
        string ret(chars);
        env->ReleaseStringUTFChars(jstr, chars);

        return ret;
    }

    map<string, string> JniHelper::jHashMap2StdMap(jobject jhashmap){
        JNIEnv *env = JniHelper::getEnv();
        map<string, string> retmap;

        do
        {
            if (jhashmap == NULL)
            {
                break;
            }
            
            if (env == NULL)
            {
                break;
            }
            static jclass mapClass = env->FindClass("java/util/HashMap");
            if (mapClass == NULL)
            {
                break;
            }
            static jmethodID mapClass_keySet = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
            if (mapClass_keySet == NULL)
            {
                break;
            }
            static jmethodID mapClass_get = env->GetMethodID(mapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
            if (mapClass_get == NULL)
            {
                break;
            }
            jobject keyset =  env->CallObjectMethod(jhashmap, mapClass_keySet);
            if (keyset == NULL)
            {
                break;
            }
            static jclass setClass =  env->FindClass("java/util/Set");
            if (setClass == NULL)
            {
                break;
            }
            static jmethodID setClass_toArray = env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;");
            if (setClass_toArray == NULL)
            {
                break;
            }
            jobjectArray keyarray = (jobjectArray)env->CallObjectMethod(keyset, setClass_toArray);
            if (keyarray == NULL)
            {
                break;
            }
            jint arraysize = env->GetArrayLength(keyarray);
            for(jint i = 0; i < arraysize; i++)
            {
                jstring key = (jstring)env->GetObjectArrayElement(keyarray, i);
                jstring value = (jstring)env->CallObjectMethod(jhashmap, mapClass_get, key);
                retmap.insert(pair<string, string>(JniHelper::jString2StdString(key), JniHelper::jString2StdString(value)));
                env->DeleteLocalRef(key);
                env->DeleteLocalRef(value);
            }
            env->DeleteLocalRef(keyset);
            env->DeleteLocalRef(keyarray);
        } while (0);
        return retmap;
    }

    vector<string> jVector2StdVector(jobject jVector){
        JNIEnv *env = JniHelper::getEnv();
        vector<string> v;

        do {
            if (jVector == NULL)
            {
                break;
            }
            
            if (env == NULL)
            {
                break;
            }
            static jclass vectorClass = env->FindClass("java/util/Vector");
            if (vectorClass == NULL)
            {
                break;
            }
            static jmethodID vectorClass_toArray = env->GetMethodID(vectorClass, "toArray", "()[Ljava/lang/Object;");
            if (vectorClass_toArray == NULL)
            {
                break;
            }
            jobjectArray valuearray = (jobjectArray)env->CallObjectMethod(jVector, vectorClass_toArray);
            if (valuearray == NULL)
            {
                break;
            }
            jint arraysize = env->GetArrayLength(valuearray);
            for(jint i = 0; i < arraysize; i++)
            {
                jstring value = (jstring)env->GetObjectArrayElement(valuearray, i);
                v.push_back(JniHelper::jString2StdString(value));
                env->DeleteLocalRef(value);
            }
            
        } while (0);
        return v;
    }

    vector<string> JniHelper::jArray2StdVector(jobject jArray){
        JNIEnv *env = JniHelper::getEnv();
        vector<string> v;

        do {
            if (jArray == NULL)
            {
                break;
            }
            
            if (env == NULL)
            {
                break;
            }
            static jclass arrayClass = env->FindClass("java/util/ArrayList");
            if (arrayClass == NULL)
            {
                break;
            }
            static jmethodID arrayClass_toArray = env->GetMethodID(arrayClass, "toArray", "()[Ljava/lang/Object;");
            if (arrayClass_toArray == NULL)
            {
                break;
            }
            jobjectArray valuearray = (jobjectArray)env->CallObjectMethod(jArray, arrayClass_toArray);
            if (valuearray == NULL)
            {
                break;
            }
            jint arraysize = env->GetArrayLength(valuearray);
            for(jint i = 0; i < arraysize; i++)
            {
                jstring value = (jstring)env->GetObjectArrayElement(valuearray, i);
                v.push_back(JniHelper::jString2StdString(value));
                env->DeleteLocalRef(value);
            }
            
        } while (0);
        return v;
    }

    //======================================================================
    // Cpp call Java functions
    //======================================================================
    void JniHelper::createHashMap(){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "createHashMap"
                                           , "()V"))
        {
            t.env->CallStaticVoidMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    jobject JniHelper::getHashMap(){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "getHashMap"
                                           , "()Ljava/util/HashMap;"))
        {
            jobject jobj = (jobject)t.env->CallStaticObjectMethod(t.classID, t.methodID);        
            t.env->DeleteLocalRef(t.classID);
            return jobj;
        }
        return NULL;
    }

    void JniHelper::pushHashMapElement(string key, string value){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "pushHashMapElement"
                                           , "(Ljava/lang/String;Ljava/lang/String;)V"))
        {
            jstring jkey = t.env->NewStringUTF(key.c_str());
            jstring jvalue = t.env->NewStringUTF(value.c_str());
            
            t.env->CallStaticVoidMethod(t.classID, t.methodID, jkey, jvalue);
            
            t.env->DeleteLocalRef(jkey);
            t.env->DeleteLocalRef(jvalue);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    void JniHelper::createVector(){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "createVector"
                                           , "()V"))
        {
            t.env->CallStaticVoidMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    jobject JniHelper::getVector(){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "getVector"
                                           , "()Ljava/util/Vector;"))
        {
            jobject jobj = (jobject)t.env->CallStaticObjectMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
            return jobj;
        }
        return NULL;
    }

    void JniHelper::pushVectorElement(string value){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "pushVectorElement"
                                           , "(Ljava/lang/String;)V"))
        {
            jstring jvalue = t.env->NewStringUTF(value.c_str());
            
            t.env->CallStaticVoidMethod(t.classID, t.methodID, jvalue);
            
            t.env->DeleteLocalRef(jvalue);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    void JniHelper::createArrayList(){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "createArrayList"
                                           , "()V"))
        {
            t.env->CallStaticVoidMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    jobject JniHelper::getArrayList(){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "getArrayList"
                                           , "()Ljava/util/ArrayList;"))
        {
            jobject jobj = (jobject)t.env->CallStaticObjectMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
            return jobj;
        }
        return NULL;
    }

    void JniHelper::pushArrayListElement(string value){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "pushArrayListElement"
                                           , "(Ljava/lang/String;)V"))
        {
            jstring jvalue = t.env->NewStringUTF(value.c_str());
            
            t.env->CallStaticVoidMethod(t.classID, t.methodID, jvalue);
            
            t.env->DeleteLocalRef(jvalue);
            t.env->DeleteLocalRef(t.classID);
        }
    }

    jobject JniHelper::checkHashMap(map<string, string> stdMap)
    {   
        JNIEnv *env = 0;
        JavaVM* jvm = JniHelper::getJavaVM();
        jint ret = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);
        switch (ret) {
            case JNI_OK:
                break;
                
            case JNI_EDETACHED :
                if (jvm->AttachCurrentThread(&env, NULL) < 0)
                {
                    LOGD("Failed to get the environment using AttachCurrentThread()");
                    return NULL;
                }
                break;
                
            case JNI_EVERSION :
            default :
                LOGD("Failed to get the environment using GetEnv()");
                return NULL;
        }

        JniHelper::createHashMap();
        for(map<string, string>::iterator it = stdMap.begin(); it != stdMap.end(); ++it)
        {
            JniHelper::pushHashMapElement(it->first, it->second);
        }

        return JniHelper::getHashMap();
    }

    jobject JniHelper::checkArrayList(vector<string> strings)
    {   
        JNIEnv *env = 0;
        JavaVM* jvm = JniHelper::getJavaVM();
        jint ret = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);
        switch (ret) {
            case JNI_OK:
                break;
                
            case JNI_EDETACHED :
                if (jvm->AttachCurrentThread(&env, NULL) < 0)
                {
                    LOGD("Failed to get the environment using AttachCurrentThread()");
                    return NULL;
                }
                break;
                
            case JNI_EVERSION :
            default :
                LOGD("Failed to get the environment using GetEnv()");
                return NULL;
        }

        JniHelper::createArrayList();
        for(vector<string>::iterator it = strings.begin(); it != strings.end(); ++it)
        {
            JniHelper::pushArrayListElement(*it);
        }
        
        return JniHelper::getArrayList();
    }

    jobject JniHelper::checkVector(vector<string> strings)
    {   
        JNIEnv *env = 0;
        JavaVM* jvm = JniHelper::getJavaVM();
        jint ret = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);
        switch (ret) {
            case JNI_OK:
                break;
                
            case JNI_EDETACHED :
                if (jvm->AttachCurrentThread(&env, NULL) < 0)
                {
                    LOGD("Failed to get the environment using AttachCurrentThread()");
                    return NULL;
                }
                break;
                
            case JNI_EVERSION :
            default :
                LOGD("Failed to get the environment using GetEnv()");
                return NULL;
        }

        JniHelper::createVector();
        for(vector<string>::iterator it = strings.begin(); it != strings.end(); ++it)
        {
            JniHelper::pushVectorElement(*it);
        }
        
        return JniHelper::getVector();
    }

    void JniHelper::showAlertDialog(string msg){
        JniMethodInfo t;
        if( JniHelper::getStaticMethodInfo(t
                                           , JNIHELPER_JAVA_CLASS
                                           , "showAlertDialog"
                                           , "(Ljava/lang/String;)V"))
        {
            jstring jvalue = t.env->NewStringUTF(msg.c_str());
            
            t.env->CallStaticVoidMethod(t.classID, t.methodID, jvalue);
            
            t.env->DeleteLocalRef(jvalue);
            t.env->DeleteLocalRef(t.classID);
        }
    }
} //namespace AppCoder
