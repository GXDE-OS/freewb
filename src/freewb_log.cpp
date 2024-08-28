#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include "freewb_log.h"

void FreewbLog(char* pszFmt,...)
{
	FILE *fp=fopen("/tmp/freewb_log.txt","a+b");

	char buf[2048];
	time_t timep;   
	struct tm *p;

	time(&timep); /*获得time_t结构的时间，UTC时间*/
	p = gmtime(&timep); /*转换为struct tm结构的UTC时间*/

	sprintf(buf,"%d%d%d %d:%d:%d",1900 + p->tm_year, 1+ p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min, p->tm_sec);
	fwrite(buf,sizeof(char),strlen(buf),fp);
	fwrite(" ",sizeof(char),1,fp);

    va_list pArgs;
    va_start(pArgs, pszFmt);
    int dwRetVal = vsnprintf(buf, sizeof(buf), pszFmt, pArgs);
    va_end(pArgs);
	char *ptr=strrchr(buf,'/');
	++ptr;
	fwrite(ptr,sizeof(char),strlen(ptr),fp);
	fwrite("\n",sizeof(char),1,fp);

	fclose(fp);
}

