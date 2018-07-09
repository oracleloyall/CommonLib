/*
 * log4cxx.hpp
 *
 *  Created on: 2015-4-22
 *      Author: root
 */

#ifndef LOG4CXX_HPP_
#define LOG4CXX_HPP_
#include <iostream>
#include <list>
using namespace std;

//#define	LOG_TEST_WHITELIST
#define 	LOG_FILE_CONF

#ifdef	LOG_TEST_WHITELIST
void logout(int level = 5,const char* format,...);
#else
void logout(const char* format,...);
#endif

class LogMin
{
private:
	list<char*> buf;
	string logfile;
	int MaxLen;
	int remain;
	int savinglog;
	void DeleteLog();
public:
	void SaveToFile();
	int GetSizeOfLog();
	void Log(string strbuf,int level);
	void AddLog(const char* strbuf,int len);
	LogMin(int len = 16384,string file = "/tmp/client.log",int level = 5);
	~LogMin();
};


#endif /* LOG4CXX_HPP_ */
