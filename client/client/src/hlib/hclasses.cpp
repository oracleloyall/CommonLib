/*
 * hclasses.cpp
 *
 *  Created on: 2013-9-26
 *      Author: root
 */

#ifndef HCLASSES_CPP_
#define HCLASSES_CPP_

#include "hclasses.hpp"

//{ TPersistent }

TPersistent::~TPersistent()
{

}

void TPersistent::Assign(TPersistent* Source)
{
  if (Source)
	  Source->AssignTo(this);
  else
	  AssignError(NULL);
}

void TPersistent::AssignError(TPersistent* Source)
{
}

void TPersistent::AssignTo(TPersistent* Dest)
{
}

string TPersistent::GetNamePath()
{
	string s = typeid(this).name();
	if (GetOwner())
	{
		string p = GetOwner()->GetNamePath();
        if (p.length() > 0)
        	return p + '.' + s;
	}
	return s;
}

TPersistent* TPersistent::GetOwner()
{
   return NULL;
}

// TStringList

int TStringList::Add(const string s)
{
	buf.push_back(s);
	return buf.size();
}

int TStringList::Insert(const string s)
{
	buf.push_front(s);
	return buf.size();
}

int TStringList::IndexOf(const string s)
{
	int i = 0;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		if (!it->compare(s)) return i; else i++;
	}
	return -1;
}

void TStringList::Delete(int index)
{
	int i = 0;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		if (i == index)
		{
			buf.erase(it);
			return;
		}
	}
}

void TStringList::Clear()
{
	buf.clear();
}

int TStringList::Count()
{
	return buf.size();
}

string TStringList::Values(const string name)
{
	int i;
	string s;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		s = *it;
		i = s.find(seperator, 0);
		if (i > -1)
		{
			s = s.substr(0, i);
			s = Trim(s);
			if (s == name)
			{
				s = it->substr(i + seperator.length(), it->length() - i - seperator.length());
				return s;
			}
		}
	}
	return string("");
}

string TStringList::ValueFromIndex(int index)
{
	int i = 0;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		if (i++ == index)
		{
			i = it->find(seperator, 0);
			if (i > -1)
			{
				string s = it->substr(i + seperator.length(), it->length() - i - seperator.length());
				return s;
			}
			else
				return string("");
		}
	}
	return string("");
}

string TStringList::Names(int index)
{
	int i = 0;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		if (i++ == index)
		{
			i = it->find(seperator, 0);
			if (i > -1)
			{
				string s = Trim(it->substr(0, i));
				return s;
			}
			else
				return "";
		}
	}
	return string("");
}

bool TStringList::SaveToFile(const string filename)
{
	if (FileExists(filename)) DeleteFile(filename);
	FILE * f = fopen(filename.c_str(), "wb+");
	if (!f) return false;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		string x = *it + "\n";
		fwrite(x.c_str(), x.size(), 1, f);
	}
	fclose(f);
	return true;
}

bool TStringList::LoadFromFile(const string filename)
{
	if (!FileExists(filename)) return false;
	FILE * f = fopen(filename.c_str(), "rb");
	stringstream ss;
	while (!feof(f))
	{
		char ch = fgetc(f);
		if (ch == 13)
		{

		} else if (ch == 10)
		{
			Add(ss.str());
			ss.str("");
		} else
		{
			ss.write(&ch, 1);
		}
	}
	string cache = ss.str();
	if (cache.length() > 0) Add(ss.str());
	fclose(f);
	ss.str("");
	return true;
}

string TStringList::Text()
{
	stringstream ss;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		ss << *it << "\n";
	}
	return ss.str();
}

void TStringList::AssignTo(TStringList* Dest)
{
	Dest->Clear();
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++) Dest->Add(*it);
}

TStringList::TStringList()
{
	buf.clear();
	seperator = "=";
}

TStringList::~TStringList()
{
	buf.clear();
}

string TStringList::Lines(int index)
{
	int i = 0;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		if (i++ == index) return (*it);
	}
	return "";
}

void TStringList::SetLines(int index, const string s)
{
	int i = 0;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		if (i++ == index)
		{
			it->assign(s);
			return;
		}
	}
}

void TStringList::SetValue(const string name, const string value)
{
	int i;
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		i = it->find(seperator, 0);
		if (i > -1)
		{
			string s = Trim(it->substr(0, i));
			if (!s.compare(name))
			{
				s = it->substr(i + seperator.length(), it->length() - i - seperator.length());
				it->assign(name + seperator + value);
				return;
			}
		}
	}
	string temp = name + seperator + value;
	buf.push_back(temp);
}

string TStringList::ReadString(const string section, const string name, const string value)
{
	string s = Values(name);
	if (!s.compare("")) return value; else return s;
}

void TStringList::SetText(const string src, const string sep)
{
	string se = (sep == "") ? seperator : sep;
	buf.clear();
    string::size_type pos = 0;
    int i = se.length();
    int j = -1;
    int k = 0;
    while( (pos = src.find(sep, pos)) != string::npos )
    {
    	buf.push_back(src.substr(k, pos - k));
    	j = pos;
    	pos += i;
    	k = pos;
    }
    if (j == -1)
    	buf.push_back(src);
    else
    {
    	j += i;
    	if (src.length() > uint(j + 1)) buf.push_back(src.substr(j, src.length() - j));
    }
}

void TStringList::AddStrings(TStringList& AStrings)
{
	for(list<string>::iterator it = AStrings.buf.begin(); it != AStrings.buf.end(); it++)
	{
		buf.push_back(*it);
	}
}

void TStringList::SaveToStream(TMemoryStream& m)
{
	for(list<string>::iterator it = buf.begin(); it != buf.end(); it++)
	{
		string s = (*it) + "\r\n";
		m.WriteBuffer((void*)s.c_str(), s.size());
	}
}

// {* TMemoryStream *}

void TMemoryStream::Clear()
{
	Position = 0;
	Size = 0;
	if (Memory)
	{
		free(Memory);
		Memory = NULL;
	}
}

int TMemoryStream::Seek(const int step, TSeekOffset mode)
{
	switch(mode)
	{
		case soFromBeginning:
		{
			if (step < 0)
				Position = 0;
			else if (step <= Size)
				Position = step;
			break;
		}
		case soFromCurrent:
		{
			int np = Position + step;
			if ((np >= 0) && (np <= Size))
				Position = np;
			break;
		}
		case soFromEnd:
		{
			Position = Size;
			break;
		}
	}
	return Position;
}

void TMemoryStream::WriteBuffer(void* src, int len)
{
	int end = Position + len;
	if (end > Size)
	{
		Memory = (pByte)realloc(Memory, end);
		Size = end;
	}
	Move(src, Memory + Position, len);
	Position += len;
}

void TMemoryStream::ReadBuffer(void* dst, int len)
{
	int end = Position + len;
	if (end > Size)
		end = Size - Position;
	else
		end = len;
	Move(Memory + Position, dst, end);
	Position += end;
}

TMemoryStream::TMemoryStream()
{
	Memory = NULL;
	Clear();
}

TMemoryStream::~TMemoryStream()
{
	Clear();
}

const int IFNAMSIZ = 16;

typedef struct ifreq
{
  char ifr_name[IFNAMSIZ];
  sockaddr ifr_addr;  // address
} *pifreq;

string GetIP(const string if_name)
{
  ifreq ifr;
  int sock = if_name.length();
  if (sock > 0) Move(if_name.c_str(), ifr.ifr_name, sock);
  ifr.ifr_name[sock] = 0;
  ifr.ifr_addr.sa_family = AF_INET;
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock >= 0)
  {
    if (ioctl(sock, SIOCGIFADDR, &ifr) >= 0)
    {
      close(sock);
      return  IntToStr(Byte(ifr.ifr_addr.sa_data[2]))
      + "." + IntToStr(Byte(ifr.ifr_addr.sa_data[3]))
      + "." + IntToStr(Byte(ifr.ifr_addr.sa_data[4]))
      + "." + IntToStr(Byte(ifr.ifr_addr.sa_data[5]));
    }
    close(sock);
  }
  return "0.0.0.0";
}

void GetNets(TStringList& Nets)
{
  Int64 c = 0;
  Nets.Clear();
  string s;
  TStringList2 nets;
  DirFiles("/sys/class/net/", "", false, nets, &c);
  for(TStringList2::iterator it = nets.begin(); it != nets.end(); it++)
  {
	  s = ExtractFilename(*it);
	  if (s != "lo") Nets.Add(s);
  }
}

void GetMACs(TStringList& MACs)
{
  string s;
  GetNets(MACs);
  for(int j = MACs.Count() - 1; j > -1; j--)
  {
    s = GetMAC(MACs.Lines(j));
    if (s == "") MACs.Delete(j); else MACs.SetLines(j, s);
  }
}

void GetIPs(TStringList& IPs)
{
  string s;
  GetNets(IPs);
  for(int j = IPs.Count() - 1; j > -1; j--)
  {
    s = GetIP(IPs.Lines(j));
    if (s == "") IPs.Delete(j); else IPs.SetLines(j, s);
  }
}

string GetMAC(const string if_name)
{
  string s = "/sys/class/net/" + if_name + "/address";
  if (!FileExists(s)) return "";
  FILE * f = fopen(s.c_str(), "rb");
  if (!f) return "";
  char buf[64];
  memset(buf, 0, 64);
  fscanf(f, "%s", buf);
  fclose(f);
  return string(buf);
}

void Split(const string big, const string sep, TStringList& sl)
{
  sl.SetText(big, sep);
}

#endif /* HCLASSES_CPP_ */
