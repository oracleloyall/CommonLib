/**********************************************************
* crb3_param.cpp
* Description: param module of common request broker v3 library source code
*	   
* Copyright 2013 miaozz
*
* Create : 13 Sep. 2013 , miaozz
*
* Modify : 12 Feb. 2014 , miaozz
*          add interface 'CRB3Param::GetStringRef'
*          add interface 'CRB3Param::GetOctetRef'
*          add interface 'CRB3Param::GetOctet'
*          10 Mar. 2016 , miaozz
*          rewrite interface 'GetIntX' to support type conversion
***********************************************************/
#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "crb3_param.h"

#ifdef NAMESPACE
namespace common{
#endif

void CRB3Address2String(IN CRB3_ADDRESS * addr, OUT char * ipstr, IN int buflen, OUT int * port)
{
	if(addr->sa.sa_family == AF_INET)
	{
		inet_ntop(AF_INET, &addr->v4.sin_addr, ipstr, buflen);
		if(port)
		{
			*port = (int)ntohs(addr->v4.sin_port);
		}
	}
	else
	{
		inet_ntop(AF_INET6, &addr->v6.sin6_addr, ipstr, buflen);
		if(port)
		{
			*port = (int)ntohs(addr->v6.sin6_port);
		}
	}
}

socklen_t CRB3String2Address(IN char * ipstr, IN int port, OUT CRB3_ADDRESS * addr)
{
	memset(addr, 0, sizeof(CRB3_ADDRESS));
	if(strchr(ipstr, ':'))
	{
		addr->v6.sin6_family = AF_INET6;
		addr->v6.sin6_port = htons(port);
		inet_pton(AF_INET6, ipstr, &addr->v6.sin6_addr);
		return sizeof(struct sockaddr_in6);
	}
	else
	{
		addr->v4.sin_family = AF_INET;
		addr->v4.sin_port = htons(port);
		inet_pton(AF_INET, ipstr, &addr->v4.sin_addr);
		return sizeof(struct sockaddr_in);
	}
}

int CRB3AddressCompare(IN CRB3_ADDRESS * addr1, IN CRB3_ADDRESS * addr2)
{
	if(addr1->sa.sa_family == addr2->sa.sa_family)
	{
		if(addr1->sa.sa_family == AF_INET &&
			addr1->v4.sin_addr.s_addr == addr2->v4.sin_addr.s_addr &&
			addr1->v4.sin_port == addr2->v4.sin_port)
		{
			return(0);
		}
		else if(addr1->sa.sa_family == AF_INET6 &&
			0 == memcmp(&addr1->v6.sin6_addr, &addr2->v6.sin6_addr, 16) &&
			addr1->v6.sin6_port == addr2->v6.sin6_port)
		{
			return(0);
		}
	}
	return(1);
}

CRB3Param::CRB3Param(IN CRB3Param & param)
{
	int idx;
	
	ParamTbl = (CRB3_TLV *)malloc(param.ParamCount * sizeof(CRB3_TLV));
	assert(ParamTbl);
	memset(ParamTbl, 0, param.ParamCount * sizeof(CRB3_TLV));
	ParamCount = param.ParamCount;
	for(idx=0;idx<ParamCount;idx++)
	{
		ParamTbl[idx].Type = param.ParamTbl[idx].Type;
		ParamTbl[idx].Len = param.ParamTbl[idx].Len;
		if(ParamTbl[idx].Type == CRB3P_TYPE_STRING || ParamTbl[idx].Type == CRB3P_TYPE_OCTOR)
		{
			ParamTbl[idx].Value.oct = (uint8_t *)malloc(ParamTbl[idx].Len);
			assert(ParamTbl[idx].Value.oct);
			memcpy(ParamTbl[idx].Value.oct, param.ParamTbl[idx].Value.oct, ParamTbl[idx].Len);
		}
		else
		{
			ParamTbl[idx].Value = param.ParamTbl[idx].Value;
		}
	}
}

CRB3Param::CRB3Param(IN int ParamCnt)
{
	ParamTbl = (CRB3_TLV *)malloc(ParamCnt * sizeof(CRB3_TLV));
	assert(ParamTbl);
	memset(ParamTbl, 0, ParamCnt * sizeof(CRB3_TLV));
	ParamCount = ParamCnt;
}

CRB3Param::CRB3Param(IN int ParamCnt, IN char * TLVBuf, IN int BufSize)
{
	char * ptr = TLVBuf;
	char * end = TLVBuf + BufSize;
	int idx = 0;

	// alloc parameter buffer
	ParamCount = ParamCnt;
	ParamTbl = (CRB3_TLV *)malloc(ParamCnt * sizeof(CRB3_TLV));
	memset(ParamTbl, 0, ParamCnt * sizeof(CRB3_TLV));

	// initialize parameter buffer
	while(idx < ParamCnt && (ptr + 4 <= end))
	{
		CRB3_FETCH_INT16(ptr, ParamTbl[idx].Type);
		CRB3_FETCH_INT16(ptr, ParamTbl[idx].Len);
		switch(ParamTbl[idx].Type)
		{
		case CRB3P_TYPE_INT8:
			CRB3_FETCH_INT8(ptr, ParamTbl[idx].Value.i8);
			break;
		case CRB3P_TYPE_INT16:
			CRB3_FETCH_INT16(ptr, ParamTbl[idx].Value.i16);
			break;
		case CRB3P_TYPE_INT32:
			CRB3_FETCH_INT32(ptr, ParamTbl[idx].Value.i32);
			break;
		case CRB3P_TYPE_INT64:
			CRB3_FETCH_INT64(ptr, ParamTbl[idx].Value.i64);
			break;
		case CRB3P_TYPE_STRING:
			ParamTbl[idx].Value.str = (char *)malloc(ParamTbl[idx].Len + 1);
			if(ParamTbl[idx].Value.str)
			{
				CRB3_FETCH_BYTE(ptr, ParamTbl[idx].Value.str, ParamTbl[idx].Len);
				ParamTbl[idx].Value.str[ParamTbl[idx].Len] = '\0';
			}
			break;
		case CRB3P_TYPE_OCTOR:
			ParamTbl[idx].Value.oct = (uint8_t *)malloc(ParamTbl[idx].Len);
			if(ParamTbl[idx].Value.oct)
			{
				CRB3_FETCH_BYTE(ptr, ParamTbl[idx].Value.oct, ParamTbl[idx].Len);
			}
			break;
		default:
			ParamTbl[idx].Type = CRB3P_TYPE_NULL;
			break;
		}
		idx++;
	}
}

CRB3Param::~CRB3Param()
{
	if(ParamTbl)
	{
		_FreeParamTable(ParamTbl);
		ParamTbl = NULL;
	}
}

void CRB3Param::SetInt8(IN int Index, IN int8_t cVal)
{
	if(Index < ParamCount)
	{
		ParamTbl[Index].Type = CRB3P_TYPE_INT8;
		ParamTbl[Index].Len = 1;
		ParamTbl[Index].Value.i8 = cVal;
	}
}

void CRB3Param::SetInt16(IN int Index, IN int16_t sVal)
{
	if(Index < ParamCount)
	{
		ParamTbl[Index].Type = CRB3P_TYPE_INT16;
		ParamTbl[Index].Len = 2;
		ParamTbl[Index].Value.i16 = sVal;
	}
}

void CRB3Param::SetInt32(IN int Index, IN int32_t iVal)
{
	if(Index < ParamCount)
	{
		ParamTbl[Index].Type = CRB3P_TYPE_INT32;
		ParamTbl[Index].Len = 4;
		ParamTbl[Index].Value.i32 = iVal;
	}
}

void CRB3Param::SetInt64(IN int Index, IN int64_t lVal)
{
	if(Index < ParamCount)
	{
		ParamTbl[Index].Type = CRB3P_TYPE_INT64;
		ParamTbl[Index].Len = 8;
		ParamTbl[Index].Value.i64 = lVal;
	}
}

void CRB3Param::SetString(IN int Index, IN char * pStr)
{
	if(Index < ParamCount)
	{
		int len = strlen(pStr);
		
		ParamTbl[Index].Type = CRB3P_TYPE_STRING;
		ParamTbl[Index].Len = (uint16_t)len + 1;
		ParamTbl[Index].Value.str = (char *)malloc(ParamTbl[Index].Len);
		memcpy(ParamTbl[Index].Value.str, pStr, ParamTbl[Index].Len);
	}
}

void CRB3Param::SetOctet(IN int Index, IN uint8_t * pOctet, IN int OctetLen)
{
	if(Index < ParamCount && OctetLen > 0 && OctetLen <= 0xffff)
	{
		ParamTbl[Index].Type = CRB3P_TYPE_OCTOR;
		ParamTbl[Index].Len = (uint16_t)OctetLen;
		ParamTbl[Index].Value.oct = (uint8_t *)malloc(OctetLen);
		memcpy(ParamTbl[Index].Value.oct, pOctet, OctetLen);
	}
}

int8_t CRB3Param::GetInt8(IN int Index)
{
	switch(ParamTbl[Index].Type)
	{
	case CRB3P_TYPE_INT8:	return (int8_t)ParamTbl[Index].Value.i8;
	case CRB3P_TYPE_INT16:	return (int8_t)ParamTbl[Index].Value.i16;
	case CRB3P_TYPE_INT32:	return (int8_t)ParamTbl[Index].Value.i32;
	case CRB3P_TYPE_INT64:	return (int8_t)ParamTbl[Index].Value.i64;
	case CRB3P_TYPE_STRING:	return (int8_t)atoi(ParamTbl[Index].Value.str);
	case CRB3P_TYPE_OCTOR:	return (int8_t)ParamTbl[Index].Value.oct[0];
	default:				return (0);
	}
}

int16_t	CRB3Param::GetInt16(IN int Index)
{
	switch(ParamTbl[Index].Type)
	{
	case CRB3P_TYPE_INT8:	return (int16_t)ParamTbl[Index].Value.i8;
	case CRB3P_TYPE_INT16:	return (int16_t)ParamTbl[Index].Value.i16;
	case CRB3P_TYPE_INT32:	return (int16_t)ParamTbl[Index].Value.i32;
	case CRB3P_TYPE_INT64:	return (int16_t)ParamTbl[Index].Value.i64;
	case CRB3P_TYPE_STRING:	return (int16_t)atoi(ParamTbl[Index].Value.str);
	case CRB3P_TYPE_OCTOR:
		{
			uint16_t len;
			union
			{
				uint8_t u8[2];		// network order (big endian)
				uint16_t u16;
			}data;

			len = min_m(ParamTbl[Index].Len, 2);
			data.u16 = 0;
			
#if __BYTE_ORDER == __BIG_ENDIAN
			memcpy(&data.u8[2-len], ParamTbl[Index].Value.oct, len);
			return (int16_t)(data.u16);
#else
			memcpy(data.u8, ParamTbl[Index].Value.oct, len);
			return (int16_t)__bswap_16(data.u16);
#endif
		}
	default:
		return(0);
	}
}

int32_t	CRB3Param::GetInt32(IN int Index)
{
	switch(ParamTbl[Index].Type)
	{
	case CRB3P_TYPE_INT8:	return (int32_t)ParamTbl[Index].Value.i8;
	case CRB3P_TYPE_INT16:	return (int32_t)ParamTbl[Index].Value.i16;
	case CRB3P_TYPE_INT32:	return (int32_t)ParamTbl[Index].Value.i32;
	case CRB3P_TYPE_INT64:	return (int32_t)ParamTbl[Index].Value.i64;
	case CRB3P_TYPE_STRING:	return (int32_t)atoi(ParamTbl[Index].Value.str);
	case CRB3P_TYPE_OCTOR:
		{
			uint16_t len;
			union
			{
				uint8_t u8[4];		// network order (big endian)
				uint32_t u32;
			}data;

			len = min_m(ParamTbl[Index].Len, 4);
			data.u32 = 0;
			
#if __BYTE_ORDER == __BIG_ENDIAN
			memcpy(&data.u8[4-len], ParamTbl[Index].Value.oct, len);
			return (int32_t)(data.u32);
#else
			memcpy(data.u8, ParamTbl[Index].Value.oct, len);
			return (int32_t)__bswap_32(data.u32);
#endif
		}
	default:
		return(0);
	}
}

int64_t	CRB3Param::GetInt64(IN int Index)
{
	switch(ParamTbl[Index].Type)
	{
	case CRB3P_TYPE_INT8:	return (int64_t)ParamTbl[Index].Value.i8;
	case CRB3P_TYPE_INT16:	return (int64_t)ParamTbl[Index].Value.i16;
	case CRB3P_TYPE_INT32:	return (int64_t)ParamTbl[Index].Value.i32;
	case CRB3P_TYPE_INT64:	return (int64_t)ParamTbl[Index].Value.i64;
	case CRB3P_TYPE_STRING:	return (int64_t)atoll(ParamTbl[Index].Value.str);
	case CRB3P_TYPE_OCTOR:
		{
			uint16_t len;
			union
			{
				uint8_t u8[8];		// network order (big endian)
				uint64_t u64;
			}data;

			len = min_m(ParamTbl[Index].Len, 8);
			data.u64 = 0;
			
#if __BYTE_ORDER == __BIG_ENDIAN
			memcpy(&data.u8[8-len], ParamTbl[Index].Value.oct, len);
			return (int64_t)(data.u64);
#else
			memcpy(data.u8, ParamTbl[Index].Value.oct, len);
			return (int64_t)__bswap_64(data.u64);
#endif
		}
	default:
		return(0);
	}
}

char * CRB3Param::GetStringRef(IN int Index, OUT int & Len)
{
	if(Index < ParamCount && ParamTbl[Index].Len > 0)
	{
		Len = (int)(ParamTbl[Index].Len - 1);
		return ParamTbl[Index].Value.str;
	}
	return NULL;
}

int CRB3Param::GetString(IN int Index, OUT char * StrBuf, IN int BufLen)
{
	if(Index < ParamCount)
	{
		int irc;
		
		switch(ParamTbl[Index].Type)
		{
		case CRB3P_TYPE_STRING:
			irc = snprintf(StrBuf, BufLen, "%s", ParamTbl[Index].Value.str);
			break;
		case CRB3P_TYPE_INT64:
			irc = snprintf(StrBuf, BufLen, "%lld", ParamTbl[Index].Value.i64);
			break;
		case CRB3P_TYPE_INT32:
			irc = snprintf(StrBuf, BufLen, "%d", ParamTbl[Index].Value.i32);
			break;
		case CRB3P_TYPE_INT16:
			irc = snprintf(StrBuf, BufLen, "%d", ParamTbl[Index].Value.i16);
			break;
		case CRB3P_TYPE_INT8:
			irc = snprintf(StrBuf, BufLen, "%d", ParamTbl[Index].Value.i8);
			break;
		case CRB3P_TYPE_OCTOR:
			irc = 0;
			for(int i=0;i<ParamTbl[Index].Len;i++)
			{
				seprintf(StrBuf, BufLen, irc, "%02x ", ParamTbl[Index].Value.oct[i]);
			}
			break;
		default:
			irc = snprintf(StrBuf, BufLen, "N/A");
			break;
		}
		return irc;
	}
	return(-1);
}

uint8_t * CRB3Param::GetOctetRef(IN int Index, OUT int & Len)
{
	if(Index < ParamCount && ParamTbl[Index].Len > 0)
	{
		Len = (int)ParamTbl[Index].Len;
		return ParamTbl[Index].Value.oct;
	}
	return NULL;
}

int CRB3Param::GetOctet(IN int Index, OUT uint8_t * OctetBuf, IN int BufLen)
{
	if(Index < ParamCount)
	{
		if(ParamTbl[Index].Len <= BufLen)
		{
			memcpy(OctetBuf, ParamTbl[Index].Value.oct, ParamTbl[Index].Len);
		}
		else
		{
			memcpy(OctetBuf, ParamTbl[Index].Value.oct, BufLen);
		}
		return (int)ParamTbl[Index].Len;
	}
	return(-1);
}

int CRB3Param::GetType(IN int Index)
{
	if(Index < ParamCount)
	{
		return (int)ParamTbl[Index].Type;
	}
	return CRB3P_TYPE_NULL;
}

int CRB3Param::GetRaw(IN int Index, OUT uint8_t * RawBuf, IN int BufLen)
{
	if(Index < ParamCount)
	{
		int cp_bytes;
		
		switch(ParamTbl[Index].Type)
		{
		case CRB3P_TYPE_OCTOR:
		case CRB3P_TYPE_STRING:
			if(BufLen >= ParamTbl[Index].Len)
			{
				cp_bytes = ParamTbl[Index].Len;
			}
			else
			{
				cp_bytes = BufLen;
			}
			memcpy(RawBuf, ParamTbl[Index].Value.oct, cp_bytes);
			break;
		case CRB3P_TYPE_INT64:
			cp_bytes = sizeof(int64_t);
			memcpy(RawBuf, &ParamTbl[Index].Value.i64, cp_bytes);
			break;
		case CRB3P_TYPE_INT32:
			cp_bytes = sizeof(int32_t);
			memcpy(RawBuf, &ParamTbl[Index].Value.i32, cp_bytes);
			break;
		case CRB3P_TYPE_INT16:
			cp_bytes = sizeof(int16_t);
			memcpy(RawBuf, &ParamTbl[Index].Value.i16, cp_bytes);
			break;
		case CRB3P_TYPE_INT8:
			cp_bytes = sizeof(int8_t);
			memcpy(RawBuf, &ParamTbl[Index].Value.i8, cp_bytes);
			break;
		case CRB3P_TYPE_NULL:
			cp_bytes = 0;
			break;
		default:
			cp_bytes = -1;
			break;
		}
		return cp_bytes;
	}
	return(-1);
}

int CRB3Param::AddParamCount(IN int ParamCntInc)
{
	int NewParamCount = ParamCount + ParamCntInc;
	CRB3_TLV * NewParamTbl = (CRB3_TLV *)malloc(NewParamCount * sizeof(CRB3_TLV));

	if(NewParamTbl == NULL)return CRB3_FAILED;
	memset(NewParamTbl, 0, NewParamCount * sizeof(CRB3_TLV));
	if(ParamTbl)
	{
		memcpy(NewParamTbl, ParamTbl, ParamCount * sizeof(CRB3_TLV));
		free(ParamTbl);
	}
	ParamTbl = NewParamTbl;
	ParamCount = NewParamCount;
	return CRB3_OK;
}

int CRB3Param::GetTLVLength(void)
{
	int idx;
	int len = 0;

	for(idx=0;idx<ParamCount;idx++)
	{
		if(ParamTbl[idx].Type != CRB3P_TYPE_NULL)
		{
			len += (4 + ParamTbl[idx].Len);
		}
		else
		{
			len += 4;
		}
	}
	return len;
}

void CRB3Param::OutputTLV(OUT char * TLVBuf)
{
	int idx;
	char * ptr = TLVBuf;
	
	for(idx=0;idx<ParamCount;idx++)
	{
		CRB3_TLV * tlv = &ParamTbl[idx];

		CRB3_SUBMIT_INT16(ptr, tlv->Type);
		CRB3_SUBMIT_INT16(ptr, tlv->Len);
		switch(tlv->Type)
		{
		case CRB3P_TYPE_INT8:
			CRB3_SUBMIT_INT8(ptr, tlv->Value.i8);
			break;
		case CRB3P_TYPE_INT16:
			CRB3_SUBMIT_INT16(ptr, tlv->Value.i16);
			break;
		case CRB3P_TYPE_INT32:
			CRB3_SUBMIT_INT32(ptr, tlv->Value.i32);
			break;
		case CRB3P_TYPE_INT64:
			CRB3_SUBMIT_INT64(ptr, tlv->Value.i64);
			break;
		case CRB3P_TYPE_STRING:
			CRB3_SUBMIT_BYTE(ptr, tlv->Value.str, tlv->Len);
			break;
		case CRB3P_TYPE_OCTOR:
			CRB3_SUBMIT_BYTE(ptr, tlv->Value.oct, tlv->Len);
			break;
		default:
			ptr += tlv->Len;
			break;
		}
	}
}

void CRB3Param::_FreeParamTable(IN CRB3_TLV * Table)
{
	int idx;

	for(idx=0;idx<ParamCount;idx++)
	{
		if(Table[idx].Type == CRB3P_TYPE_STRING &&
			Table[idx].Value.str != NULL)
		{
			free(Table[idx].Value.str);
		}
		else if(Table[idx].Type == CRB3P_TYPE_OCTOR &&
				Table[idx].Value.oct != NULL)
		{
			free(Table[idx].Value.oct);
		}
	}
	free(Table);
}

#ifdef NAMESPACE
}
#endif
