/*
 * psystem.h
 *
 *  Created on: 2013-9-15
 *      Author: root
 */

#ifndef PASUTILS_H_
#define PASUTILS_H_

//#define H_REDHAT_STYLE
#define H_DEBIAN_STYLE

#include "global.hpp"
#include <algorithm>
#include <cctype>
#include "hbase64.hpp"
#ifdef H_ICONV
#include <iconv.h>
#endif
#include "md5.h"

typedef list<string> TStringList2;

const int HoursPerDay   = 24;
const int MinsPerHour   = 60;
const int SecsPerMin    = 60;
const int MSecsPerSec   = 1000;
const int MinsPerDay    = HoursPerDay * MinsPerHour;
const int SecsPerDay    = MinsPerDay * SecsPerMin;
const int MSecsPerDay   = SecsPerDay * MSecsPerSec;
const int UnixDateDelta = 25569;
const int DateDelta     = 693594;
const string charmap = "-_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

typedef void (*TServiceAction)();

extern uint64_t DiskFree(const string path);
extern uint64_t DiskAvail(const string path);
extern uint64_t DiskTotal(const string path);
extern double GetFileDate(const string filename);
extern int64_t GetFileSize(const string filename);
extern bool WriteInstallConfig(const string ExeName, const string Intro);
extern string IntToStr(const int n);
extern string UIntToStr(const uint n);
extern string Int64ToStr(const long long n);
extern string UInt64ToStr(const unsigned long long n);
extern string AppName();
extern string ThisPath();
extern bool Install(const string intro);
extern bool Uninstall();
extern bool RegisterService(const string s);
extern uint ParamCount();
extern string ParamStr(const uint index);
extern string GetCommandLine();
extern bool FileExists(const string filename);
extern string ExtractFilename(const string ExeName);
extern string ExtractFilepath(const string ExeName);
extern bool DirectoryExists(const string DirName);
extern bool ForceDirectories(const string DirName);
extern bool RenameFile(const string src, const string dst);
extern bool DeleteFile(const string src);
extern bool Mkdir(const string DirName);
extern void RunService(const string intro, const TServiceAction StartAction);
extern void StringReplace(string& strBig, const string strsrc, const string strdst);
extern void Dirs(const string Path, const string SubParam, const bool IncludeSubDir, TStringList2& DirResult, pInt64 TotalSize);
extern void DirFiles(const string Path, const string SubParam, const bool IncludeSubDir, TStringList2& DirResult, pInt64 TotalSize);
extern int LastDelimiter(const string s);
extern string ChangeFileExt(const string Filename, const string ext);
extern string ExtractFileExt(const string Filename);
extern bool TryStrToWord(const string s, pWord w);
extern int CompareMem(const void* src, const void* dst, const int len);
extern int StrToInt(const string s);
extern uint StrToUInt(const string s);
extern Int64 StrToInt64(const string s);
extern UInt64 StrToUInt64(const string s);
extern bool TryStrToInt(const string s, int& i);
extern double StrToFloat(const string s);
extern string MidStr(const string src, const int from, const int len);
extern string LeftStr(const string src, const int len);
extern string RightStr(const string src, const int len);
extern void Move(const void* src, void* dst, const int len);
extern void Inc(int& dst, int src);
extern void Dec(int& dst, int src);
extern void Inc(int& dst);
extern void Inc(Int64& dst);
extern void Inc(Int64& dst, Int64 src);
extern void Inc(Int64& dst, int src);
extern void Dec(int& dst);
extern string Trim(const string s);
extern string BoolToStr(bool b, bool alpha = true);
extern int PosChar(const Byte c, const pByte src, const int len);
extern int PosWord(const Word c, const pWord src, const int len);
extern int PosCardinal(const Cardinal c, const pCardinal src, const int len);
extern int Length(const string s);
extern string LowerCase(const string s);
extern string UpperCase(const string s);
extern int Pos(const string substr, const string src);
extern string URLEncode(const string sIn);
extern string URLDecode(const string sIn);
extern string RandomString(const int len = 32);
extern Byte fromHex(const Byte &x);
extern Byte toHex(const Byte &x);
extern string Seal(const string s);
extern string Unseal(const string s);
extern string StringToSeal(const string s);
extern string SealToString(const string s);
extern string FormatByte(Int64 i);
extern string FormatByte(int i);
extern string FormatByte(double f);
extern string DecodePassword(const string password);
extern string EncodePassword(const string password);
extern string DecodeStringBase64(const string src);
extern string EncodeStringBase64(const string src);
#ifdef H_ICONV
extern string UTF8ToCP936(string s);
extern string CP936ToUTF8(string s);
#endif
extern string FloatToStr(const double d, string format = "%.1f");
extern string md5(string src);
extern double UnixToDateTime(const Int64 AValue);
extern Int64 StrToInt64(const string s);

#endif /* PASUTILS_H_ */
