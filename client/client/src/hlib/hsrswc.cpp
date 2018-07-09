/*
 * hsrswc.cpp
 *
 *  Created on: 2015-2-4
 *      Author: masanari
 */

#ifndef HSRSWC_CPP_
#define HSRSWC_CPP_

#include "hsrswc.hpp"

THSRSWC::THSRSWC(int nSize)
{
    m_nBufSize = nSize;
    buf = (PHDataNode)malloc(m_nBufSize * sizeof(THDataNode));
    SetEmpty();
}

THSRSWC::~THSRSWC()
{
    free(buf);
}

void THSRSWC::SetEmpty()
{
	m_readpos = 0;
	m_writepos = 0;
	memset(buf, 0, m_nBufSize * sizeof(THDataNode));
}

bool THSRSWC::Read(THPointer& x)
{
	if (buf[m_readpos].sign == 0) return false;
	x = buf[m_readpos].data;
	buf[m_readpos].sign = 0;
	m_readpos = (m_readpos + 1) % m_nBufSize;
	return true;
}

bool THSRSWC::CanRead()
{
	return buf[m_readpos].sign != 0;
}

bool THSRSWC::Write(THPointer x)
{
	if (buf[m_writepos].sign != 0) return false;
	buf[m_writepos].data = x;
	buf[m_writepos].sign = (THPointer)1;
	m_writepos = (m_writepos + 1) % m_nBufSize;
	return true;
}

bool THSRSWC::CanWrite()
{
	return buf[m_writepos].sign == 0;
}


#endif /* HSRSWC_CPP_ */


