/*
 * hclasses.hpp
 *
 *  Created on: 2013-9-26
 *      Author: root
 */

#ifndef HCLASSES_HPP_
#define HCLASSES_HPP_

#include "global.hpp"
#include <typeinfo>
#include "hpasutils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

enum TSeekOffset {soFromBeginning = 0, soFromCurrent = 1, soFromEnd = 2};

class TPersistent
{
private:
    void AssignError(TPersistent* Source);
protected:
    virtual void AssignTo(TPersistent* Dest);
    virtual TPersistent* GetOwner();
public:
    virtual ~TPersistent();
    virtual void Assign(TPersistent* Source);
    virtual string GetNamePath();
};

class TMemoryStream:public TPersistent
{
public:
	pByte Memory;
	int Position;
	int Size;
	void Clear();
	int Seek(const int step, TSeekOffset mode);
	void WriteBuffer(void* src, int len);
	void ReadBuffer(void* dst, int len);
	TMemoryStream();
	virtual ~TMemoryStream();
};

//{ TStrings class }

class TStringList:public TPersistent
{
protected:
	list<string> buf;
public:
	string seperator;
	int Add(const string s);
	void AddStrings(TStringList& AStrings);
	int Insert(const string s);
	int IndexOf(const string s);
	void Delete(int index);
	void Clear();
	int Count();
	string Values(const string name);
	void SetValue(const string name, const string value);
	string ValueFromIndex(int index);
	string Names(int index);
	string Lines(int index);
	void SetLines(int index, const string s);
	string ReadString(const string section, const string name, const string value);
	bool SaveToFile(const string filename);
	bool LoadFromFile(const string filename);
	string Text();
	void SaveToStream(TMemoryStream& m);
	void SetText(const string src, const string sep = "");
	void AssignTo(TStringList* Dest);
	TStringList();
	virtual ~TStringList();
};

extern void GetNets(TStringList& Nets);
extern void GetMACs(TStringList& MACs);
extern void GetIPs(TStringList& IPs);
extern string GetIP(const string if_name);
extern string GetMAC(const string if_name);
extern void Split(const string big, const string sep, TStringList& sl);

#endif /* HCLASSES_HPP_ */
