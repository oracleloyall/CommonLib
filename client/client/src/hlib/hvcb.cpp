/*
 * hvcb.cpp
 *
 *  Created on: 2014-12-22
 *      Author: masanari
 */

#ifndef HVCB_CPP_
#define HVCB_CPP_

#include "hvcb.hpp"

THVCB::THVCB(int nSize)
{
  m_nBufSize = nSize;
  m_pInnerBuffer = (pByte)malloc(m_nBufSize);
  SetEmpty();
}

THVCB::~THVCB()
{
  free(m_pInnerBuffer);
}

void THVCB::SetEmpty()
{
  m_nFreeCacheSize = m_nBufSize;
  m_pHead = m_pInnerBuffer;
  m_pCacheTail = m_pInnerBuffer;
  m_pCacheHead = 0;
  m_pOuterBuffer = 0;
  m_nInnerBufferValid = 0;
  m_nOuterBufferValid = 0;
  m_nSmartCache = 0;
  m_nValidCount = 0;
}

int THVCB::GetSafeValidCount()
{
  return m_nValidCount;
}

bool THVCB::IsOverFlowBuffer(int nLen)
{
  //Result := nLen >= m_nBufSize - m_nValidCount;
  return nLen >= (m_nBufSize - m_nInnerBufferValid);
}

bool THVCB::HeadSafePosInc(int nStep)
{
  if (nStep < m_nInnerBufferValid)  // only move on inner buffer
  {
	m_pHead += nStep;
	m_nInnerBufferValid -= nStep;
	m_nValidCount = m_nInnerBufferValid + m_nOuterBufferValid;
	return true;
  }
  if (nStep > m_nValidCount) return false;
  if (m_nInnerBufferValid > 0) // have inner buffer, but moved onto outer buffer
  {
	nStep -= m_nInnerBufferValid;
	m_pHead = m_pOuterBuffer;
	m_pHead += nStep;
	m_nOuterBufferValid -= nStep;
	m_nInnerBufferValid = 0;
	m_nValidCount = m_nOuterBufferValid;
	return true;
  }
  // only move on outer buffer
  m_pHead += nStep;
  m_nOuterBufferValid -= nStep;
  m_nValidCount = m_nOuterBufferValid;
  return true;
}

bool THVCB::WriteCirBuffer(pByte pBuffer, int m_nBufLen)
{
  if (m_nBufLen < 1) return false;
  m_nOuterBufferValid = m_nBufLen;
  m_pOuterBuffer = pBuffer;
  m_pCacheHead = pBuffer;
  if (m_nInnerBufferValid == 0)
  {
	m_pHead = pBuffer;
	m_nValidCount = m_nOuterBufferValid;
  }
  else
	m_nValidCount = m_nOuterBufferValid + m_nInnerBufferValid;
  return true;
}

void THVCB::GetBufferData(pByte* pBuffer, int m_nBufLen, int m_nPos)
{
  int tail, cz, i;
  //pByte p;
  tail = m_nPos + m_nBufLen;
  if ((m_nBufLen < 1) || (m_nPos > m_nValidCount) || (tail > m_nValidCount)) return;
  if ((m_nInnerBufferValid == 0) || (m_nOuterBufferValid == 0) || (tail <= m_nInnerBufferValid))  // inner, outer, inner+outer but only inner
  {
    *pBuffer = m_pHead;
    *pBuffer += m_nPos;
    return;
  }
  if (m_nPos >= m_nInnerBufferValid)  // inner+outer but only outer
  {
    *pBuffer = m_pOuterBuffer;
    *pBuffer += (m_nPos - m_nInnerBufferValid);
    return;
  }
  *pBuffer = m_pHead;
  *pBuffer += m_nPos;
  cz = tail - m_nInnerBufferValid - m_nSmartCache;   // cz is overed size of outer buffer
  if (cz > 0)
  {
    i = min(m_nFreeCacheSize, cz); // need copy how many bytes
    Move(m_pCacheHead, m_pCacheTail, i);
    m_pCacheTail += i;
    m_pCacheHead += i;
    m_nSmartCache += i;
    m_nFreeCacheSize -= i;
  }
}

void THVCB::Defrag()
{
  int bcopy, i;
  if (m_nInnerBufferValid == 0)
  {
	if (m_nOuterBufferValid > 0)     // only outer
	{
	  m_nInnerBufferValid = m_nOuterBufferValid;
	  m_nValidCount = m_nOuterBufferValid;
	  Move(m_pHead, m_pInnerBuffer, m_nOuterBufferValid);
	  m_nFreeCacheSize = m_nBufSize - m_nInnerBufferValid;
	  m_pHead = m_pInnerBuffer;
	  m_pCacheTail = m_pHead;
	  m_pCacheTail += m_nInnerBufferValid;
	  m_nOuterBufferValid = 0;
	  m_nSmartCache = 0;
	}
	else
	  SetEmpty();
  }
  else if (m_nOuterBufferValid > 0) // all have
  {
	if (m_nSmartCache < m_nOuterBufferValid)
	{
	  bcopy = min(m_nFreeCacheSize, m_nOuterBufferValid);
	  i = bcopy - m_nSmartCache;
	  if (i > 0) Move(m_pCacheHead, m_pCacheTail, i);
	}
	m_nInnerBufferValid += m_nOuterBufferValid;
	m_nFreeCacheSize -= m_nOuterBufferValid;
	m_pCacheTail = m_pHead;
	m_pCacheTail += m_nInnerBufferValid;
	m_nSmartCache = 0;
	m_nOuterBufferValid = 0;
	m_nValidCount = m_nInnerBufferValid;
  }
  else
  {  // only inner
	m_pCacheTail = m_pHead;
	m_pCacheTail += m_nInnerBufferValid;
	m_nFreeCacheSize = m_nBufSize - m_nInnerBufferValid - (THItemPointer(m_pHead) - THItemPointer(m_pInnerBuffer));
	m_nSmartCache = 0;
	m_nValidCount = m_nInnerBufferValid;
  }
}

#endif /* HVCB_CPP_ */
