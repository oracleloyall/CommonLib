/*
 * randommac.h
 *
 *  Created on: 2015年10月9日
 *      Author: sw
 */

#ifndef RANDOMMAC_H_
#define RANDOMMAC_H_

typedef signed   char   INT8S;
typedef unsigned char 	INT8U;
typedef signed   short  INT16S;
typedef unsigned short  INT16U;
typedef signed 	 int    INT32S;
typedef unsigned int    INT32U;

void MacStrToInt(const char* Mac,INT8U* UMac);
void MacIntToStr(const INT8U* Mac,char* MacStr);
int IFileExit(const char* filename);
void MyExec(const char* cmd);
int SetMacByUci(const char* interface,const INT8U* Mac);
void RandomMac(void);

#endif /* RANDOMMAC_H_ */
