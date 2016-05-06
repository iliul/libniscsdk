#ifndef __SDK_GETSTATICINFO_H
#define __SDK_GETSTATICINFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SYSUUIDCMD "dmidecode -s system-uuid"
#define OS_RELEASE "cat /etc/os-release | grep VERSION_ID"
#define BUILDTIME "/usr/share/NFS Desktop/buildtime"


#define HTTP_SERVER "http://nisc.nfschina.com/v1/regauth/"
#define DEBUG_SERVER "http://192.168.8.132:8000/v1/regauth/"
#define LOCAL_SERVER "http://192.168.8.125:8000/push/"
#define BASE_VERSION "SDK v1.0"
#define FILENAME "callback.log"
#define DEVNAME "/dev/sda"

typedef struct{
	char SC_RespInfo[64];
	char SC_ValidTime[20];
	long int SC_RespCode;
}sc_statuscode_t;

enum{
	HTTP_RESPONSE_CODE_200 = 200,
	HTTP_RESPONSE_CODE_201,

	HTTP_RESPONSE_CODE_302 = 302,
	HTTP_RESPONSE_CODE_303,

	HTTP_RESPONSE_CODE_400 = 400,
	HTTP_RESPONSE_CODE_404 = 404,
	HTTP_RESPONSE_CODE_405,
	HTTP_RESPONSE_CODE_406,
	HTTP_RESPONSE_CODE_410 = 410,
};

typedef enum{
    SUCCESS = 0,

    ERROR_POPEN = 100,
    ERROR_MAKEPOSTFILEDS,
    ERROR_GET_SYSINFO,
    ERROR_GET_OSRELEASE,
    ERROR_GET_BUILDTIME,
    ERROR_GET_HDSN,
    ERROR_MD5_ENCRYPT,
    ERROR_JSON_STRING,
    ERROR_JSON_PARESEDATA,
    ERROR_SDK_MD5ENCRYPT,

    ERROR_CURL_GET = 200,
    ERROR_CURL_POST,
    ERROR_CURL_PREFORM
}SDK_ErrCode_t;

/*
   函数 : _SDK_TestAdd
*/
SDK_ErrCode_t _SDK_TestAdd(int a, int b);

size_t _CURL_WriteCallback(void *buffer, size_t size, size_t nmemb, void *user_p);

static inline SDK_ErrCode_t _Comm_TransStr(char *str, char *res);
static inline char * _Comm_GetRightN(char *dst,char *src, int n);

/*
   函数名 : 	_JSON_ParseData
   参数 	:  	buff 待解析JSON	字符串
   				sc 结构体指针
   返回值 : 	成功返回0，失败返回错误原因代码
   功能	:  	解析JSON  数据，获取状态码
*/
SDK_ErrCode_t _JSON_ParseData(char *buff, sc_statuscode_t *sc);

inline SDK_ErrCode_t _SDK_MakePostFields(char *RegCode, char *RetStr);
inline int _SDK_GetHDSN(const char* szDevName, char* szSN, size_t nLimit);
inline int _SDK_GetOSRelease(char *os_rel);
inline int _SDK_GetBuildTime(char *bt);
inline int _SDK_GetSystemUUID(char *sysuuid);
//int _SDK_DoRegister(char *RegCode, sc_statuscode_t *res);
sc_statuscode_t * _SDK_DoRegister(char *RegCode);
sc_statuscode_t * _SDK_ConfirmRegister(char *RegCode);
sc_statuscode_t * _SDK_DoVerify();
SDK_ErrCode_t _SDK_GetRespCode(sc_statuscode_t * StatusCode);
SDK_ErrCode_t _SDK_MD5Encrypt(char *RetStr);

/*
   函数名 : 	_SDK_GetLibVersion
   参数 	:  	无
   返回值 : 	成功返回0，失败返回错误原因代码
   功能	:  	获取SDK 版本信息
*/
SDK_ErrCode_t _SDK_GetLibVersion();

/*
   函数名 : 	_MD5_Encrypt
   参数 	: 	srcStr  待加密字符串
   				EncryptStr 加密后返回串
   返回值 :  	成功返回0，失败返回错误原因代码
   功能	:  	字符串MD5	加密
*/
SDK_ErrCode_t _MD5_Encrypt(char *srcStr, char *EncryptStr);


/*
   函数名 : 	_CURL_CommPost
   参数 	: 	EncryptStr 已构造的POST	请求参数字符串
   返回值 :  	成功返回0，失败返回错误原因代码
   功能	:  	利用CURL	进行POST	请求
*/
int _CURL_CommPost(char *EncryptStr);


/*
   函数名 : 	_CURL_CommGet
   参数 	: 	EncryptStr 已机密的GET   请求参数字符串
   				如:	    e4f5a986b785994266ebe3cae4b24141
   返回值 :  	成功返回0，失败返回错误原因代码
   功能	:  	利用CURL	进行GET	请求
*/
int _CURL_CommGet(char *EncryptStr);

#ifdef __cplusplus
}
#endif

#endif
