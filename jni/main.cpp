
#include "JniHelper.h"
#include "android/asset_manager.h"
#include "android/asset_manager_jni.h"
#include "Log.h"

using namespace AppCoder;

extern "C"
{
	jint JNI_OnLoad(JavaVM *vm, void *reserved)
	{
		LOGD("JNI_OnLoad");
	    JniHelper::setJavaVM(vm);
	    return JNI_VERSION_1_4;
	}

	JNIEXPORT void JNICALL Java_com_appcoder_lib_JniHelper_nativeSetContext(JNIEnv*  env, jobject thiz, jobject context, jobject assetManager) {
        JniHelper::setClassLoaderFrom(context);
        JniHelper::setassetmanager(AAssetManager_fromJava(env, assetManager));
    }
}