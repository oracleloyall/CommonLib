#ifndef _IN_crb3_paramh_
#define _IN_crb3_paramh_

#include "crb3_define.h"

#ifdef NAMESPACE
namespace common{
#endif

#define CRB3_PARAM_RESERVE_LEN		20

#define CRB3P_TYPE_NULL				0x0000
#define CRB3P_TYPE_INT8				0x0001
#define CRB3P_TYPE_INT16			0x0002
#define CRB3P_TYPE_INT32			0x0003
#define CRB3P_TYPE_INT64			0x0004
#define CRB3P_TYPE_STRING			0x0005
#define CRB3P_TYPE_OCTOR			0x0006

typedef union
{
	int8_t		i8;
	int16_t		i16;
	int32_t		i32;
	int64_t		i64;
	char *		str;
	uint8_t	*	oct;
}CRB3_VALUE;

typedef struct
{
	uint16_t Type;
	uint16_t Len;		// Byte size of Value
	CRB3_VALUE Value;
}CRB3_TLV;

void CRB3Address2String(IN CRB3_ADDRESS * addr, OUT char * ipstr, IN int buflen, OUT int * port);
socklen_t CRB3String2Address(IN char * ipstr, IN int port, OUT CRB3_ADDRESS * addr);
int CRB3AddressCompare(IN CRB3_ADDRESS * addr1, IN CRB3_ADDRESS * addr2);

class CRB3Param
{
	CRB3_TLV * ParamTbl;

public:
	int ParamCount;
	
public:
	CRB3Param(IN CRB3Param & param);
	CRB3Param(IN int ParamCnt);
	CRB3Param(IN int ParamCnt, IN char * TLVBuf, IN int BufSize);
	~CRB3Param();

	void SetInt8(IN int Index, IN int8_t cVal);
	void SetInt16(IN int Index, IN int16_t sVal);
	void SetInt32(IN int Index, IN int32_t iVal);
	void SetInt64(IN int Index, IN int64_t lVal);
	void SetString(IN int Index, IN char * pStr);
	void SetOctet(IN int Index, IN uint8_t * pOctet, IN int OctetLen);
	
	int8_t	GetInt8(IN int Index);
	int16_t	GetInt16(IN int Index);
	int32_t	GetInt32(IN int Index);
	int64_t	GetInt64(IN int Index);
	char *	GetStringRef(IN int Index){		return ParamTbl[Index].Value.str;	}
	char *	GetStringRef(IN int Index, OUT int & Len);
	int		GetString(IN int Index, OUT char * StrBuf, IN int BufLen);
	uint8_t * GetOctetRef(IN int Index){	return ParamTbl[Index].Value.oct;	}
	uint8_t * GetOctetRef(IN int Index, OUT int & Len);
	uint8_t * GetOctet(IN int Index, OUT int & OctetLen)
	{
		OctetLen = (int)ParamTbl[Index].Len;
		return ParamTbl[Index].Value.oct;
	}
	int GetOctet(IN int Index, OUT uint8_t * OctetBuf, IN int BufLen);
	int GetType(IN int Index);
	int GetRaw(IN int Index, OUT uint8_t * RawBuf, IN int BufLen);

	int GetParamCount(void){	return ParamCount;	}
	int AddParamCount(IN int ParamCntInc);
	int GetTLVLength(void);
	void OutputTLV(OUT char * TLVBuf);

private:
	void _FreeParamTable(IN CRB3_TLV * Table);
};

#ifdef NAMESPACE
}
#endif
#endif

