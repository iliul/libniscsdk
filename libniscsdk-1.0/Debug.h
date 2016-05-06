#ifndef DEBUG_H
#define DEBUG_H 

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <syslog.h>

#define DL_ERROR        	3
#define DL_WARNING  	4
#define DL_NOTICE      	5
#define DL_INFO		     	6
#define DL_TRACE        	7
#define DL_ALL			8

#define DBG_LEVEL DL_WARNING

#define PRINT_FUNC() fprintf(stderr , "%s %s(%d) " , __FILE__ , __func__ , __LINE__);

#ifndef DbgError
#if (DBG_LEVEL >= DL_ERROR)
#define DbgError(fmt, args...) PRINT_FUNC();fprintf(stderr,fmt, ##args);syslog(LOG_ERR,fmt, ##args);
#else
#define DbgError(fmt, args...)
#endif
#endif

#ifndef DbgWarning
#if (DBG_LEVEL >= DL_WARNING)
#define DbgWarning(fmt, args...) PRINT_FUNC();fprintf(stderr,fmt, ##args);syslog(LOG_WARNING,fmt, ##args);
#else
#define DbgWarning(fmt, args...)
#endif
#endif

#ifndef DbgNotice
#if (DBG_LEVEL >= DL_NOTICE)
#define DbgNotice(fmt, args...) fprintf(stderr,fmt, ##args)
#else
#define DbgNotice(fmt, args...)
#endif
#endif

#ifndef DbgTrace
#if (DBG_LEVEL >= DL_TRACE)
#define DbgTrace(fmt, args...) fprintf(stderr,fmt, ##args)
#else
#define DbgTrace(fmt, args...) 
#endif
#endif

#ifndef DbgInfo
#if (DBG_LEVEL >= DL_INFO)
#define DbgInfo(fmt, args...) PRINT_FUNC();fprintf(stdout,fmt, ##args);syslog(LOG_WARNING,fmt, ##args);
#else
#define DbgInfo(fmt, args...) 
#endif
#endif

#define MAX_LOG_FILE_SIZE (4 << 20)  //4M
static inline int DbgLog(int level , const char *format, ...)
{
	FILE *fd;
	va_list marker;
	
	if(level <= DBG_LEVEL) 
	{
		time_t ltime;
		char tempch[50];
		unsigned long flen = 0;
		int lognum = 0;
		struct tm *newtime;
		
		time(&ltime);
		newtime = localtime(&ltime);
		va_start(marker, format);
		
	 	sprintf(tempch,"./SDKLog_%d.txt",lognum);
		
		fd = fopen(tempch, "a+");
		fseek(fd , 0L , SEEK_END); ///* 定位到文件末尾 */ 　　
		flen = ftell(fd); ///* 得到文件大小 */ 
		if(flen >= MAX_LOG_FILE_SIZE )
		{
			fclose(fd);
			lognum ++;
			if(lognum > 9)
			{
				lognum = 0;
			}
			sprintf(tempch,"./SDKLog_%d.txt",lognum);
			fd = fopen(tempch, "w+" );
		}
		fprintf(fd, "[%04d-%02d-%02d %02d:%02d:%02d level %d]: ", 
			newtime->tm_year + 1900,
			newtime->tm_mon + 1, 
			newtime->tm_mday, 
			newtime->tm_hour, 
	        		newtime->tm_min,
			newtime->tm_sec,DBG_LEVEL);
		
		vfprintf(fd, format, marker);
		fclose(fd);
		va_end(marker);
	}

	return 0;
}

#endif
