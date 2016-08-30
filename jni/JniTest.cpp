#include <thread>
#include <stdio.h>
#include <stdlib.h>

#include "JniHelper.h"
#include "Log.h"
#include "CppJavaBridge.h"

using namespace AppCoder;
using namespace std;

/*
typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;
*/

#define JNITEST_CLASS_NAME "com/appcoder/lib/JniTest"

extern "C"
{
    
	JNIEXPORT void JNICALL Java_com_appcoder_lib_JniTest_testCallNative(JNIEnv*  env, jobject thiz, jobject context) {
        LOGD("testCallNative in Cpp");

        // 演示参数为String、int
        LOGD("示例1：演示参数为String、int");
        jstring jname = env->NewStringUTF("AppCoder");
        jint jage = 18;

        jvalue name,age;
        name.l = jname;
        age.i = jage;

        vector<jvalue> jargs;
        jargs.push_back(name);
        jargs.push_back(age);
        CppJavaBridge::callJavaStaticMethod(JNITEST_CLASS_NAME,"setName","(Ljava/lang/String;I)V",jargs);


        // 演示参数为boolean
        LOGD("示例2：演示参数为boolean");
        jvalue isCoder;
        isCoder.z = JNI_TRUE;

        vector<jvalue> jargs2;
        jargs2.push_back(isCoder);
        CppJavaBridge::callJavaStaticMethod(JNITEST_CLASS_NAME,"setIsCoder","(Z)V",jargs2);

        // 演示参数为float
        LOGD("示例3：演示参数为float");
        jvalue slary;
        slary.f = 9.9f;

        vector<jvalue> jargs3;
        jargs3.push_back(slary);
        CppJavaBridge::callJavaStaticMethod(JNITEST_CLASS_NAME,"setSalary","(F)V",jargs3);

         // 演示参数为HashMap
        LOGD("示例4：演示参数为HashMap");
        map<std::string, std::string> infoMap;
        infoMap.insert(pair<std::string, std::string>("name","AppCoder"));
        infoMap.insert(pair<std::string, std::string>("age","18"));
        infoMap.insert(pair<std::string, std::string>("slary","9.9"));

        vector<jvalue> jargs4;
        jvalue jinfo;
        jinfo.l = JniHelper::checkHashMap(infoMap);
        jargs4.push_back(jinfo);

        CppJavaBridge::callJavaStaticMethod(JNITEST_CLASS_NAME,"setUserInfo","(Ljava/util/HashMap;)V",jargs4);

        // 演示弹出框
        JniHelper::showAlertDialog(string("演示完毕,请看控制台log输出"));
    }

    void subThreadWork()
    {
        // 演示弹出框
        JniHelper::showAlertDialog(string("from C sub thread."));
    }

    void createSubThread()
    {
        auto t = new std::thread(subThreadWork);
        if (t->joinable()) {
            t->join();
        }
    }

    JNIEXPORT void JNICALL Java_com_appcoder_lib_JniTest_testNatvieSubThread(JNIEnv*  env, jobject thiz, jobject context) {
        createSubThread();
    }
}