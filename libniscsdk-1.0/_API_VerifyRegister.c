#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <curl/curl.h>
#include <assert.h>

#include "_API_VerifyRegister.h"
#include "md5.h"
#include "cJSON.h"
#include "Debug.h"

char RecBuffer[128] = "";

//for extern test
//int i = 1;

static inline SDK_ErrCode_t _Comm_TransStr(char *str, char *res)
{
    int i = 0;
    int j = 0;
    char dst[128];
    
    while(str[i])
    {
        if((str[i] == '\\') || (i == 0) || (i == strlen(str)-1))
        {
            i++;
            continue;
        }       
        dst[j] = str[i];
        i++;
        j++;
    }
 
    strcpy(res, dst);
    return SUCCESS;
}

static inline char * _Comm_GetRightN(char *dst,char *src, int n)
{
	char *p = src;
	char *q = dst;
	int len = strlen(src);
	
	if(n > len)
	{
		n = len;
	}
	
	p += (len - n);
	while(*q++ = *p++);
	
	return dst;
}

SDK_ErrCode_t _MD5_Encrypt(char *srcStr, char *EncryptStr)
{
	unsigned char encrypt[44] = "";
	unsigned char decrypt[32] = "";
	char destStr[16] = "";
	int i , len = 0;

	strcpy((char *)encrypt, (const char *)srcStr);
	
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5, encrypt, strlen((char *)encrypt));
	MD5Final(&md5, decrypt);
	
	for(i=0;i<16;i++)
	{
		sprintf(destStr+len, "%02x", decrypt[i]);
		len += 2;
	}
	
	strcpy(EncryptStr, destStr);

	return SUCCESS;
}


SDK_ErrCode_t _JSON_ParseData(char *buff, sc_statuscode_t *sc)
{
	cJSON *json, *jsonMessage, *jsonTime;
	char tmp[128];

	//locate json string
	char *Pjson = strstr(buff, "\"{");
	
	if(Pjson == NULL)
	{
		sc->SC_RespCode = HTTP_RESPONSE_CODE_406;
		strcpy(sc->SC_RespInfo, "invalid json string");
		DbgWarning("[SDK]:_JSON_ParseData HTTP CODE:%d\n", HTTP_RESPONSE_CODE_406);
		return ERROR_JSON_PARESEDATA;
	}
	
	_Comm_TransStr(buff, tmp);
	json = cJSON_Parse(tmp);
    	if (!json)
    	{
       	DbgError("[SDK]:cJSON_Parse %s\n", cJSON_GetErrorPtr());
    	}
    	else
	{
	   	assert(cJSON_GetArraySize(json) >= 1);
	    	jsonMessage = cJSON_GetObjectItem(json ,"message");
		if( jsonMessage->type == cJSON_String)
		{
	 		strcpy(sc->SC_RespInfo, jsonMessage->valuestring);
		}
		
		if(cJSON_GetArraySize(json) == 2)
		{
			jsonTime = cJSON_GetObjectItem(json ,"valid_date");
			if( jsonTime->type == cJSON_String)
			{
		 		strcpy(sc->SC_ValidTime, jsonTime->valuestring);
				DbgInfo("EndTime:%s\n", sc->SC_ValidTime);
			}
		}
	}
	
	cJSON_Delete(json);
	return SUCCESS;
}

size_t _CURL_WriteCallback(void *buffer, size_t size, size_t nmemb, void *user_p)
{
	DbgInfo("buffer:%s\n", (char *)buffer);
	memset(RecBuffer, 0, sizeof(RecBuffer));
	strcpy(RecBuffer, (const char *)buffer);
	return strlen((const char *)buffer);
}

int _CURL_CommGet(char *EncryptStr)
{
	CURL *curl;
	CURLcode res;
	long int RespCode = 0;	
	char ServerUrl[128] = "";
	
	sprintf(ServerUrl,"%s%s/", HTTP_SERVER, EncryptStr);
	DbgInfo("ServerUrl:%s\n", ServerUrl);

	//Begin deal wich libcurl
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, ServerUrl);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &_CURL_WriteCallback);

		//连接远端站点，发送命令，传输数据
		res = curl_easy_perform(curl);
		if(res == CURLE_OK || res == CURLE_WRITE_ERROR)
		{
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &RespCode);
			if((CURLE_OK == res) && RespCode)
			{
				DbgInfo("[debug]We received Response-Code:%ld\n", RespCode);
			}
		}
		else
		{
			DbgError("[SDK]:curl_easy_perform() failed: %s,res:%d\n", curl_easy_strerror(res),res);
			return HTTP_RESPONSE_CODE_405;
		}
		curl_easy_cleanup(curl);
		
	}
	curl_global_cleanup();
	
	return RespCode;
}

int _CURL_CommPost(char *EncryptStr)
{
	CURL *curl;
	CURLcode res;
	long int RespCode = -1;

	#if 0	
	FILE *fptr;
	
	//CURL结果重定向到文件指针
	if ((fptr = fopen(FILENAME, "w")) == NULL) 
	{
		fprintf(stderr, "fopen file error: %s\n", FILENAME);
		exit(1);
	}
	#endif

	//Begin deal wich libcurl
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, HTTP_SERVER);

		DbgInfo("post string=%s\n", EncryptStr);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, EncryptStr);
		
		//不设置默认取字符串长度
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(EncryptStr));

		//设置CURL  回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &_CURL_WriteCallback);

		//非0表示此请求为POST
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		
		//设置是否返回HTTP头部信息,  返回到 终端
		//curl_easy_setopt(curl, CURLOPT_HEADER, 1);

		//获取详细信息,包含自定义状态码,  返回到 终端
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		//设置CURL  已打开文件指针
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, fptr);

		//连接远端站点，发送命令，传输数据
		res = curl_easy_perform(curl);
		if(res == CURLE_OK || res == CURLE_WRITE_ERROR)
		{
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &RespCode);
			if((CURLE_OK == res) && RespCode)
			{
				DbgInfo("[debug]We received Response-Code:%ld\n", RespCode);
			}
		}
		else
		{
			DbgError("[SDK]:curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return HTTP_RESPONSE_CODE_405;
		}
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	//fclose(fptr);
	return RespCode;
}

int _CURL_CommPut(char *EncryptStr)
{
	CURL *curl;
	CURLcode res;
	long int RespCode = -1;

	//Begin deal wich libcurl
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, HTTP_SERVER);

		DbgInfo("%s:%s\n", __func__, EncryptStr);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, EncryptStr);
		
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		
		//curl_easy_setopt(curl, CURLOPT_PUT, 1L);
		//curl_easy_setopt(curl, CURLOPT_INFILESIZE, long filesize);
		//curl_easy_setopt(curl, CURLOPT_READDATA, void *pointer);

		//设置CURL  回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &_CURL_WriteCallback);

		//连接远端站点，发送命令，传输数据
		res = curl_easy_perform(curl);
		if(res == CURLE_OK || res == CURLE_WRITE_ERROR)
		{
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &RespCode);
			if((CURLE_OK == res) && RespCode)
			{
				DbgInfo("[debug]We received Response-Code:%ld\n", RespCode);
			}
		}
		else
		{
			DbgError("[SDK]:curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return HTTP_RESPONSE_CODE_405;
		}
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return RespCode;
}

inline SDK_ErrCode_t _SDK_MakePostFields(char *RegCode, char *RetStr)
{
	int ret = 0;
	char os_release[32] = "";
	char buildtime[12] = "";
	char SNCode[32] = "";
	char TmpStr[196] = "";
	char systemUUID[40] = "";
	char FormatCode[8] = "";
	int len = 0;

	ret = _SDK_GetHDSN(DEVNAME, SNCode, sizeof(SNCode));
	if((ret != 0) || (SNCode == NULL))
	{
		return ERROR_GET_HDSN;
	}
	sprintf(TmpStr,"regcode=%s&harddisk_nu=%s&", RegCode, _Comm_GetRightN(FormatCode,SNCode,8));
	len = strlen(TmpStr);
	DbgInfo("TmpStr=%s,len=%d\n", TmpStr, len);

	ret = _SDK_GetSystemUUID(systemUUID);
	if((ret != 0) || (systemUUID == NULL))
	{
		return ERROR_GET_SYSINFO;
	}
	sprintf(TmpStr+len,"system_uuid=%s&", systemUUID);
	len = strlen(TmpStr);
	DbgInfo("TmpStr=%s,len=%d\n", TmpStr, len);


	ret = _SDK_GetBuildTime(buildtime);
	DbgInfo("BuildTime:%s,len=%zu\n", buildtime,strlen(buildtime));
	if((ret != 0) || (buildtime== NULL))
	{
		return ERROR_GET_BUILDTIME;
	}
	sprintf(TmpStr+len,"buildtime=%s&", buildtime);
	len = strlen(TmpStr);
	DbgInfo("TmpStr=%s,len=%d\n", TmpStr, len);

	
	ret = _SDK_GetOSRelease(os_release);
	DbgInfo("OS-Release:%s\n", os_release);
	if((ret != 0) || (os_release == NULL))
	{
		return ERROR_GET_OSRELEASE;
	}
	sprintf(TmpStr+len,"os_version=%s", os_release);
	len = strlen(TmpStr);
	DbgInfo("TmpStr=%s,len=%d\n", TmpStr, len);

	strcpy(RetStr, TmpStr);
	
	return SUCCESS;
}

SDK_ErrCode_t _SDK_MD5Encrypt(char *RetStr)
{
	int   ret;
	char EncryptStr[32] = "";
	char TmpStr[44] = "";
	char systemUUID[40] = "";
	char SNCode[32] = "";
	char FormatCode[8] = "";

	ret = _SDK_GetSystemUUID(systemUUID);
	if(ret != 0)
	{
		return ERROR_GET_SYSINFO;
	}
	
	ret = _SDK_GetHDSN(DEVNAME, SNCode, sizeof(SNCode));
	if(ret != 0)
	{
		return ERROR_GET_HDSN;
	}
	sprintf(TmpStr, "%s%s", systemUUID, _Comm_GetRightN(FormatCode,SNCode,8));

	DbgInfo("TmpStr:%s\n", TmpStr);
	memset(EncryptStr, 0, sizeof(EncryptStr));
	ret = _MD5_Encrypt(TmpStr, EncryptStr);
	if((ret !=0) || (EncryptStr == NULL))
	{
		return ERROR_MD5_ENCRYPT;
	}

	DbgInfo("EncryptStr:%s,len:%zu\n", EncryptStr, strlen(EncryptStr));
	strcpy(RetStr, EncryptStr);
	return SUCCESS;
}

sc_statuscode_t * _SDK_DoVerify()
{
	int   ret = 0;
	char EncryptStr[32] = "";
	sc_statuscode_t *sc = (sc_statuscode_t *)malloc(sizeof(sc_statuscode_t));

	ret = _SDK_MD5Encrypt(EncryptStr);
	if(ret != 0)
	{
		DbgError("[SDK]:_SDK_MD5Encrypt [ret]:%d\n", ret);
	}

	sc->SC_RespCode = _CURL_CommGet(EncryptStr);
	if(sc->SC_RespCode == HTTP_RESPONSE_CODE_405)
	{
		strcpy(sc->SC_RespInfo, "Couldn't connect to server");
	}
	else
	{
		ret = _JSON_ParseData(RecBuffer, sc);
		if(ret != 0)
		{
			DbgError("[SDK]:_JSON_ParseData [ret]:%d\n", ret);
		}
	}
	
	return sc;
}

//int _SDK_DoRegister(char *RegCode, sc_statuscode_t *res);
sc_statuscode_t * _SDK_DoRegister(char *RegCode)
{
	int ret;
	char EncryptStr[196] = "";
	sc_statuscode_t *sc = (sc_statuscode_t *)malloc(sizeof(sc_statuscode_t));

	ret = _SDK_MakePostFields(RegCode, EncryptStr);
	if((ret != SUCCESS) || (EncryptStr == NULL))
	{
		DbgError("[SDK]:_SDK_MakePostFields [ret]:%d\n", ret);
	}

	sc->SC_RespCode = _CURL_CommPost(EncryptStr);
	if(sc->SC_RespCode == HTTP_RESPONSE_CODE_405)
	{
		strcpy(sc->SC_RespInfo, "Couldn't connect to server");
		return sc;
	}
	else if(sc->SC_RespCode == HTTP_RESPONSE_CODE_303)
	{
		sc->SC_RespCode = _CURL_CommPut(EncryptStr);
		if(sc->SC_RespCode == HTTP_RESPONSE_CODE_405)
		{
			strcpy(sc->SC_RespInfo, "Couldn't connect to server");
			return sc;
		}
	}

	ret = _JSON_ParseData(RecBuffer, sc);
	if(ret != 0)
	{
		DbgError("[SDK]:_JSON_ParseData [ret]:%d\n", ret);
	}

	return sc;
}

sc_statuscode_t * _SDK_ConfirmRegister(char *RegCode)
{
	int ret;
	char EncryptStr[196] = "";
	sc_statuscode_t *sc = (sc_statuscode_t *)malloc(sizeof(sc_statuscode_t));

	ret = _SDK_MakePostFields(RegCode, EncryptStr);
	if((ret != SUCCESS) || (EncryptStr == NULL))
	{
		DbgError("[SDK]:_SDK_MakePostFields [ret]:%d\n", ret);
	}

	sc->SC_RespCode = _CURL_CommPut(EncryptStr);
	if(sc->SC_RespCode == HTTP_RESPONSE_CODE_405)
	{
		strcpy(sc->SC_RespInfo, "Couldn't connect to server");
		return sc;
	}

	ret = _JSON_ParseData(RecBuffer, sc);
	if(ret != 0)
	{
		DbgError("[SDK]:_JSON_ParseData [ret]:%d\n", ret);
	}

	return sc;
}


SDK_ErrCode_t _SDK_GetRespCode(sc_statuscode_t * StatusCode)
{
	int ret;

	//locate json string
	char *Pjson = strchr(RecBuffer, '{');
	sc_statuscode_t *sc = (sc_statuscode_t *)malloc(sizeof(sc_statuscode_t));
	
	ret = _JSON_ParseData(Pjson, sc);
	if(ret != 0)
	{
		DbgError("[SDK]:_JSON_ParseData [ret]:%d\n", ret);
	}

	memcpy(StatusCode, sc, sizeof(sc_statuscode_t));
	return SUCCESS;
}

inline int _SDK_GetSystemUUID(char *sysuuid)
{
	static char system_uuid[36] = "";
	
	FILE *fp = popen(SYSUUIDCMD,"r");
	if( fp != NULL)
	{
		fread(system_uuid, 1, sizeof(system_uuid), fp);
		pclose(fp);
	}
	else
	{
		DbgError("[SDK]:_SDK_GetSystemUUID popen error!\n");
        	return -1;
	}
	strncpy(sysuuid, system_uuid, strlen(system_uuid));

       return 0;
}

inline int _SDK_GetOSRelease(char *os_rel)
{
	char tmp_osrel[48] = "";
	memset(tmp_osrel, 0, sizeof(tmp_osrel));

	FILE *osl = popen(OS_RELEASE, "r");
	if( osl != NULL)
	{
		fread(tmp_osrel, 1, sizeof(tmp_osrel)-1, osl);
	}
	else
	{
		DbgError("[SDK]:_SDK_GetOSRelease popen error!\n");
        	return -1;
	}
	
	fclose(osl);
	DbgInfo("src os_release length:%zu\n", strlen(tmp_osrel));
	//strcpy(os_rel, _Comm_GetRightN(os_rel, tmp_osrel, 22));
	strncpy(os_rel, tmp_osrel+12, strlen(tmp_osrel)-12-2);
	
    return 0;
}

inline int _SDK_GetBuildTime(char *bt)
{
	char *tmp_buildtime;
	tmp_buildtime = (char *)malloc(sizeof(char) * 19);
	
	FILE *fp = fopen(BUILDTIME, "r");
	if( fp != NULL)
	{
		fread(tmp_buildtime, 1, 19, fp);
	}
	else
	{
		DbgError("[SDK]:_SDK_GetBuildTime fopen error!\n");
        	return -1;
	}
	fclose(fp);

	//e.g. string : "source 201506102025"
	DbgInfo("Tmp string buildtime:[%s]\n", tmp_buildtime);
	strcpy(bt, _Comm_GetRightN(bt, tmp_buildtime, 12));

    	return 0;
}

//shell cmd : hdparm -I /dev/sda | grep "Serial Number:"
inline int _SDK_GetHDSN(const char* szDevName, char* szSN, size_t nLimit)
{
    struct hd_driveid id;

    int  fd = open(szDevName, O_RDONLY|O_NONBLOCK);
    while(1)
    {
        if(fd < 0)
        {
            perror(szDevName);
            break;
        }

        if(!ioctl(fd, HDIO_GET_IDENTITY, &id))
        {
            strncpy(szSN, (const char *)id.serial_no, nLimit);
        }
	 else
	 {
		return -1;
	 }
        break;
    }
    return 0;
}

SDK_ErrCode_t _SDK_GetLibVersion()
{
	printf("%s\n",BASE_VERSION);
	return SUCCESS;
}

SDK_ErrCode_t _SDK_TestAdd(int a, int b)
{
	return a + b;
	return SUCCESS;
}
