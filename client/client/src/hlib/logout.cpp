#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include "logout.hpp"
#include "global.hpp"

extern const int H_COMMON = 5;
extern const int H_CAUTION = 4;
extern const int H_WARNING = 3;
extern const int H_EMERGENCY = 2;
extern const int H_DEADLY = 1;

int LogLevel = LOG_INFO;
bool UseLog = true;

void logout(const char* format,...)
{
#ifdef	H_DEBUG
	struct timeval tv;
	gettimeofday(&tv,NULL);
	struct tm *ptm = localtime(&(tv.tv_sec));
	if(!ptm)
	{
		return;
	}
	char buf[1024] = "";
	snprintf(buf,1024,"%02d:%02d:%02d [sw] %s",ptm->tm_hour,ptm->tm_min,ptm->tm_sec,format);
	va_list valst;
	va_start(valst, format);
	vprintf(format, valst);
	va_end(valst);
#endif
}

void LogMin::DeleteLog()
{
	char *tmp = buf.front();
	int len = strlen(tmp);
	savinglog -= len;
	remain += len;
	buf.pop_front();
	free(tmp);
}
void LogMin::AddLog(const char* strbuf,int len)
{
	if((!strbuf) || (len > MaxLen))
		return;
	while(len > remain)
		DeleteLog();
	savinglog += len;
	remain -= len;
	buf.push_back(strdup(strbuf));
}

void LogMin::SaveToFile()
{
	FILE* fp = fopen(logfile.c_str(), "w+");
	if (!fp)
		return;
	list<char*>::iterator it = buf.begin();
	for (int i = 0; i < buf.size();i++,it++)
	{
		char* tmp = *it;
		fprintf(fp,"%s\n",tmp);
	}
	fclose(fp);
	logout("Save Log into file %s\n",logfile.c_str());
}

int LogMin::GetSizeOfLog()
{
	return savinglog;
}

void LogMin::Log(string strbuf,int level)
{
	if(level >= H_COMMON)
//	if(level >= H_DEADLY)
		AddLog(strbuf.c_str(),strbuf.length());
}

LogMin::LogMin(int len,string file,int level)
{
	MaxLen = len;
	logfile = file;
	savinglog = 0;
	remain = MaxLen;
	logout("New LogMin is suceful\n");
}

LogMin::~LogMin()
{
	SaveToFile();
	list<char*>::iterator it = buf.begin();
	for(int i = 0 ;i < buf.size();it++)
	{
		char* tmp = *it;
		free(tmp);
	}
	buf.clear();
}

