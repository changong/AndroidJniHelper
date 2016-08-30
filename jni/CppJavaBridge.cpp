#include "CppJavaBridge.h"
#include "JniHelper.h"
#include "Log.h"

using namespace AppCoder;

int CppJavaBridge::callJavaStaticMethod(const char* className,const char* methodName,const char* methodSig,vector<jvalue> jargs)
{	

	CallInfo call(className, methodName, methodSig);

	jvalue* args = NULL;
	// 传递的参数数量
	int count = jargs.size();
	if(count > call.getArgumentCount())
	{
		LOGD("too many args in CppJavaBridge::callJavaStaticMethod");
		return 0;
	}
	else if(count < call.getArgumentCount())
	{
		LOGD("too few args in CppJavaBridge::callJavaStaticMethod");
		return 0;
	}
	else if(count > 0 && count == call.getArgumentCount())
	{
		args = new jvalue[count];
		for(int i=0;i < count; i++)
		{
			args[i] = jargs.at(i);
		}
	}

	if (!call.isValid())
	{
		LOGD("LuaJavaBridge::callJavaStaticMethod(\"%s\", \"%s\", args, \"%s\") CHECK FAILURE, ERROR CODE: %d",
    			className, methodName, methodSig, call.getErrorCode());
		return 2;
	}

	bool success = args ? call.executeWithArgs(args) : call.execute();
    if (args) delete []args;

    if (!success)
    {
    	LOGD("LuaJavaBridge::callJavaStaticMethod(\"%s\", \"%s\", args, \"%s\") EXECUTE FAILURE, ERROR CODE: %d",
    			className, methodName, methodSig, call.getErrorCode());
    	return 2;
    }

    LOGD("LuaJavaBridge::callJavaStaticMethod(\"%s\", \"%s\", args, \"%s\") SUCCESS",
			className, methodName, methodSig);

    return 1 ;
}

// CallInfo functions
CppJavaBridge::CallInfo::~CallInfo(void)
{
    switch (m_returnType)
    {
        case TypeVoid:
        case TypeInteger:
        case TypeFloat:
        case TypeBoolean:
            break;            
        case TypeString:
            if (m_ret.stringValue)
                delete m_ret.stringValue;
            break;
        case TypeVector:
        case TypeArrayList:
        case TypeMap:
            if (m_ret.objectValue)
                m_env->DeleteLocalRef( m_ret.objectValue );
            break;
    }
}

bool CppJavaBridge::CallInfo::execute(void)
{
	switch (m_returnType)
    {
        case TypeVoid:
            m_env->CallStaticVoidMethod(m_classID, m_methodID);
            break;

        case TypeInteger:
            m_ret.intValue = m_env->CallStaticIntMethod(m_classID, m_methodID);
            break;

        case TypeFloat:
            m_ret.floatValue = m_env->CallStaticFloatMethod(m_classID, m_methodID);
            break;

        case TypeBoolean:
            m_ret.boolValue = m_env->CallStaticBooleanMethod(m_classID, m_methodID);
            break;

        case TypeString: {
            m_retjs = (jstring)m_env->CallStaticObjectMethod(m_classID, m_methodID);
            const char *stringBuff = m_env->GetStringUTFChars(m_retjs, 0);
            m_ret.stringValue = new string(stringBuff);
            m_env->ReleaseStringUTFChars(m_retjs, stringBuff);
            break;
        }

        case TypeVector:
        case TypeArrayList:
        case TypeMap:
            m_ret.objectValue = (jobject)m_env->CallStaticObjectMethod(m_classID, m_methodID);
            break;
    }

	if (m_env->ExceptionCheck() == JNI_TRUE)
	{
		m_env->ExceptionDescribe();
		m_env->ExceptionClear();
		m_error = CPPJ_ERR_EXCEPTION_OCCURRED;
		return false;
	}

	return true;
}

bool CppJavaBridge::CallInfo::executeWithArgs(jvalue *args)
{
    switch (m_returnType)
     {
         case TypeVoid:
             m_env->CallStaticVoidMethodA(m_classID, m_methodID, args);
             break;

         case TypeInteger:
             m_ret.intValue = m_env->CallStaticIntMethodA(m_classID, m_methodID, args);
             break;

         case TypeFloat:
             m_ret.floatValue = m_env->CallStaticFloatMethodA(m_classID, m_methodID, args);
             break;

         case TypeBoolean:
             m_ret.boolValue = m_env->CallStaticBooleanMethodA(m_classID, m_methodID, args);
             break;

         case TypeString: {
        	 m_retjs = (jstring)m_env->CallStaticObjectMethodA(m_classID, m_methodID, args);
			 const char *stringBuff = m_env->GetStringUTFChars(m_retjs, 0);
			 m_ret.stringValue = new string(stringBuff);
			 m_env->ReleaseStringUTFChars(m_retjs, stringBuff);
             break;
        }
         case TypeVector:
         case TypeArrayList:
         case TypeMap:
             m_ret.objectValue = (jobject)m_env->CallStaticObjectMethodA(m_classID, m_methodID, args);
             break;
     }

	if (m_env->ExceptionCheck() == JNI_TRUE)
	{
		m_env->ExceptionDescribe();
		m_env->ExceptionClear();
		m_error = CPPJ_ERR_EXCEPTION_OCCURRED;
		return false;
	}

	return true;
}

bool CppJavaBridge::CallInfo::validateMethodSig(void)
{
    size_t len = m_methodSig.length();
    if (len < 3 || m_methodSig[0] != '(') // min sig is "()V"
    {
    	m_error = CPPJ_ERR_INVALID_SIGNATURES;
    	return false;
	}

    size_t pos = 1;
    while (pos < len && m_methodSig[pos] != ')')
    {
    	CppJavaBridge::ValueType type = checkType(m_methodSig, &pos);
    	if (type == TypeInvalid) return false;

		m_argumentsCount++;
		m_argumentsType.push_back(type);
        pos++;
    }

    if (pos >= len || m_methodSig[pos] != ')')
	{
    	m_error = CPPJ_ERR_INVALID_SIGNATURES;
    	return false;
	}

    pos++;
    m_returnType = checkType(m_methodSig, &pos);
    return true;
}

CppJavaBridge::ValueType CppJavaBridge::CallInfo::checkType(const string& sig, size_t *pos)
{
    switch (sig[*pos])
    {
        case 'I':
            return TypeInteger;
        case 'F':
            return TypeFloat;
        case 'Z':
            return TypeBoolean;
        case 'V':
        	return TypeVoid;
        case 'L':
            size_t pos2 = sig.find_first_of(';', *pos + 1);
            if (pos2 == string::npos)
            {
                m_error = CPPJ_ERR_INVALID_SIGNATURES;
                return TypeInvalid;
            }

            const string t = sig.substr(*pos, pos2 - *pos + 1);
            if (t.compare("Ljava/lang/String;") == 0)
            {
            	*pos = pos2;
                return TypeString;
            }
            else if (t.compare("Ljava/util/Vector;") == 0)
            {
            	*pos = pos2;
                return TypeVector;
            }
            else if (t.compare("Ljava/util/HashMap;") == 0)
            {
                *pos = pos2;
                return TypeMap;
            }
            else if (t.compare("Ljava/util/ArrayList;") == 0)
            {
                *pos = pos2;
                return TypeArrayList;
            }
            else
            {
            	m_error = CPPJ_ERR_TYPE_NOT_SUPPORT;
                return TypeInvalid;
            }
    }

    m_error = CPPJ_ERR_TYPE_NOT_SUPPORT;
    return TypeInvalid;
}

bool CppJavaBridge::CallInfo::getMethodInfo(void)
{
    m_methodID = 0;
    m_env = 0;

    JavaVM* jvm = AppCoder::JniHelper::getJavaVM();
    jint ret = jvm->GetEnv((void**)&m_env, JNI_VERSION_1_4);
    switch (ret) {
        case JNI_OK:
            break;

        case JNI_EDETACHED :
            if (jvm->AttachCurrentThread(&m_env, NULL) < 0)
            {
                LOGD("%s", "Failed to get the environment using AttachCurrentThread()");
                m_error = CPPJ_ERR_VM_THREAD_DETACHED;
                return false;
            }
            break;

        case JNI_EVERSION :
        default :
            LOGD("%s", "Failed to get the environment using GetEnv()");
            m_error = CPPJ_ERR_VM_FAILURE;
            return false;
    }
    jstring _jstrClassName = m_env->NewStringUTF(m_className.c_str());
    m_classID = (jclass) m_env->CallObjectMethod(AppCoder::JniHelper::classloader,
                                                   AppCoder::JniHelper::loadclassMethod_methodID,
                                                   _jstrClassName);

    if (NULL == m_classID) {
        LOGD("Classloader failed to find class of %s", m_className.c_str());
    }

    m_env->DeleteLocalRef(_jstrClassName);
    m_methodID = m_env->GetStaticMethodID(m_classID, m_methodName.c_str(), m_methodSig.c_str());
    if (!m_methodID)
    {
        m_env->ExceptionClear();
        LOGD("Failed to find method id of %s.%s %s",
                m_className.c_str(),
                m_methodName.c_str(),
                m_methodSig.c_str());
        m_error = CPPJ_ERR_METHOD_NOT_FOUND;
        return false;
    }

    return true;
}
