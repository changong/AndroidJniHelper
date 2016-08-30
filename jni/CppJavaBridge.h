#ifndef APPCODER_CPP_JAVA_BRIDGE_H
#define APPCODER_CPP_JAVA_BRIDGE_H

#include <jni.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#define CPPJ_ERR_OK                 0
#define CPPJ_ERR_TYPE_NOT_SUPPORT   (-1)
#define CPPJ_ERR_INVALID_SIGNATURES (-2)
#define CPPJ_ERR_METHOD_NOT_FOUND   (-3)
#define CPPJ_ERR_EXCEPTION_OCCURRED (-4)
#define CPPJ_ERR_VM_THREAD_DETACHED (-5)
#define CPPJ_ERR_VM_FAILURE         (-6)

class CppJavaBridge
{
public:
	static int callJavaStaticMethod(const char* className,const char* methodName,const char* methodSig,vector<jvalue> jargs);

private:
    typedef enum
    {
        TypeInvalid = -1,
        TypeVoid    = 0,
        TypeInteger = 1,
        TypeFloat   = 2,
        TypeBoolean = 3,
        TypeString  = 4,
        TypeVector  = 5,
        TypeFunction= 6,
        TypeMap     = 7,
        TypeArrayList = 8,
    } ValueType;

    typedef vector<ValueType> ValueTypes;

    typedef union
    {
        int     intValue;
        float   floatValue;
        int     boolValue;
        string *stringValue;
        jobject objectValue;
    } ReturnValue;

    class CallInfo
    {
    public:
        CallInfo(const char *className, const char *methodName, const char *methodSig)
        : m_valid(false)
        , m_error(CPPJ_ERR_OK)
        , m_className(className)
        , m_methodName(methodName)
        , m_methodSig(methodSig)
        , m_returnType(TypeVoid)
        , m_argumentsCount(0)
        , m_retjs(NULL)
        , m_env(NULL)
        , m_classID(NULL)
        , m_methodID(NULL)
        {
            memset(&m_ret, 0, sizeof(m_ret));
        	m_valid = validateMethodSig() && getMethodInfo();
        }
    	~CallInfo(void);

        bool isValid(void) {
        	return m_valid;
        }

        int getErrorCode(void) {
        	return m_error;
        }

        JNIEnv *getEnv(void) {
        	return m_env;
        }

        int argumentTypeAtIndex(size_t index) {
        	return m_argumentsType.at(index);
        }

        bool execute(void);
        bool executeWithArgs(jvalue *args);

        int getArgumentCount(){
        	return m_argumentsCount;
        };
    private:
        bool 		m_valid;
        int 		m_error;

        string      m_className;
        string      m_methodName;
        string      m_methodSig;
        int         m_argumentsCount;
        ValueTypes  m_argumentsType;
        ValueType   m_returnType;

        ReturnValue m_ret;
        jstring     m_retjs;

        JNIEnv     *m_env;
        jclass      m_classID;
        jmethodID   m_methodID;

        bool validateMethodSig(void);
        bool getMethodInfo(void);
        ValueType checkType(const string& sig, size_t *pos);
    };
};

#endif //APPCODER_CPP_JAVA_BRIDGE_H