/*
 * proj_utils.cpp
 *
 *  Created on: 2015-1-27
 *      Author: masanari
 */

#ifndef PROJ_UTILS_CPP_
#define PROJ_UTILS_CPP_

#include "proj_utils.hpp"

int HexToInt(char c)
{
   if (c >= '0' && c <= '9') return (c - '0');
   if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
   if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
   return 0;
}

void HexToBytes(char* hexstring, char* bytes, int hexlength)
{
   for (int i = 0; i < hexlength ; i += 2)
   {
       bytes[i/2] = (char) ((HexToInt(hexstring[i]) << 4) | HexToInt(hexstring[i + 1]));
   }
}

void BytesToHex(char* bytes, int bytelength, char *hexstring, int hexstrlength)
{
   char str2[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
   for (int i=0,j=0;(i<bytelength)&&(j<hexstrlength);i++,j++)
   {
       int b;
       b = 0x0f&(bytes[i]>>4);
       char s1 = str2[b];
       hexstring[j] = s1;
       b = 0x0f & bytes[i];
       char s2 = str2[b];
       j++;
       hexstring[j] = s2;
   }
}

/*
string read_mac(pByte buf)
{
	char temp[16];
	temp[16] = 0;
	for(int i = 0; i < 6; i++)
	{
		sprintf(&temp[i * 2], "%02x", *(buf + i));
		if (i < 5) temp[i * 3 + 2] = '-';
	}
	string s = temp;
	return s;
}

void write_mac(pByte buf, string mac)
{
	memset(buf, 6, 0);
	for(int i = 0; i < 6; i++)
	{
		*(buf+i) = HexToInt(mac[i * 3]) * 16 + HexToInt(mac[i * 3 + 1]);
	}
}
*/

string read_mac(pByte buf)
{
	char temp[16];
	temp[16] = 0;
	for(int i = 0; i < 6; i++)
	{
		sprintf(&temp[i * 2], "%02X", *(buf + i));
	}
	string s = temp;
	return s;
}

void write_mac(pByte buf, string mac)
{
	memset(buf, 0, 6);
	for(int i = 0; i < 6; i++)
	{
		*(buf+i) = HexToInt(mac[i * 2]) * 16 + HexToInt(mac[i * 2 + 1]);
	}
}

string read_ip(pByte buf)
{
	string s = "";
	for (int i = 0; i < 8; i++)
	{
		s += IntToStr(*(buf+i));
		if ((i == 7) || ((i == 3) && (*(buf + 4) == 0))) break;
		s += '.';
	}
	return s;
}

void write_ip(pByte buf, string ip)
{
	memset(buf, 0, 8);
	int j = ip.size();
	int head = 0;
	int index = 0;
	for(int i = 0; i < j; i++)
	{
		if (ip[i] == '.')
		{
			 *(buf + index++) = StrToInt(ip.substr(head, i - head));
			 head = i + 1;
		}
	}
	*(buf + index) = StrToInt(ip.substr(head, j - 1));
}

string read_devid(pByte buf)
{
	if (*buf == 0) return "";
	string s = (char*)buf;
	return s;
}

void write_devid(pByte buf, string dev_id)
{
	memset(buf, 0, 16);
	Move(dev_id.c_str(), buf, dev_id.size());
}

string read_hardseq(pByte buf)
{
	/*
	char tmp[129];
	tmp[128] = 0;
	BytesToHex((char*)buf, 64, tmp, 128);
	string s = tmp;
	*/
	buf[63] = 0;
	string s = (char*)buf;
	return s;
}

void write_hardseq(pByte buf, string hard_seq)
{
	//memset(buf, 64, 0);
	//HexToBytes((char*)hard_seq.c_str(), (char*)buf, hard_seq.length());
	int i = hard_seq.length();
	i = i > 63 ? 63 : i;
	buf[i] = 0;
	if (!i) return;
	Move(hard_seq.c_str(), buf, i);
}

#endif /* PROJ_UTILS_CPP_ */


