/*
 * hxcb.cpp
 *
 *  Created on: 2015-1-4
 *      Author: masanari
 */

#ifndef HXCB_CPP_
#define HXCB_CPP_

#include "hxcb.hpp"

THXCB::THXCB(int nSize)
{
  m_nBufSize = nSize;
  m_pBuffer = (pByte)malloc(m_nBufSize << 1);
  SetEmpty();
}

THXCB::~THXCB()
{
  free(m_pBuffer);
}

void THXCB::SetEmpty()
{
  m_nValidCount = 0;
  m_nBufferCount = m_nBufSize;
  m_pReadPos = m_pBuffer;
  m_pWritePos = m_pBuffer;
  m_nReadCounter = 0;
  m_nWriteCounter = 0;
  m_nSmartCache = 0;
}

int THXCB::GetSafeValidCount()
{
  return m_nValidCount;
}

bool THXCB::IsOverFlowBuffer(int nLen)
{
  return nLen > m_nBufferCount;
}

bool THXCB::HeadSafePosInc(int nStep)
{
  if ((nStep > m_nValidCount) || (nStep < 0)) return false;
  m_nValidCount -= nStep;
  m_nReadCounter += nStep;
  m_pReadPos += nStep;
  if (m_nReadCounter >= m_nBufSize)
  {
    m_pReadPos -= m_nBufSize;
    m_nReadCounter -= m_nBufSize;
    m_nSmartCache = 0;
  }
  m_nBufferCount += nStep;
  return true;
}

bool THXCB::WriteCirBuffer(pByte pBuffer, int m_nBufLen)
{
  if ((m_nBufLen > m_nBufferCount) || (m_nBufLen < 0)) return false;
  m_nBufferCount -= m_nBufLen;
  int i = m_nBufSize - m_nWriteCounter;
  if (i >= m_nBufLen) // direct copy
  {
    Move(pBuffer, m_pWritePos, m_nBufLen);
    m_nWriteCounter += m_nBufLen;
    m_pWritePos += m_nBufLen;
  }
  else
  { // copy twice
    Move(pBuffer, m_pWritePos, i);
    pBuffer += i;
    int j = m_nBufLen - i;
    Move(pBuffer, m_pBuffer, j);
    m_nWriteCounter = j;
    m_pWritePos = m_pBuffer;
    m_pWritePos += j;
  }
  if (m_nWriteCounter == m_nBufSize)
  {
    m_nWriteCounter = 0;
    m_pWritePos = m_pBuffer;
  }
  m_nValidCount += m_nBufLen;
  return true;
}

void THXCB::GetBufferData(pByte* pBuffer, int m_nBufLen, int m_nPos)
{
  int i, j, k, l, m;
  pByte p;
  i = m_nPos + m_nBufLen;
  if ((i < 0) || (i > m_nValidCount)) return;
  j = m_nPos + m_nReadCounter;
  k = i + m_nReadCounter;
  p = m_pBuffer;
  m = j % m_nBufSize;
  p += m;
  *pBuffer = p;
  if ((j >= m_nBufSize) == (k > m_nBufSize)) return; // copy once
  // copy twice
  k = m_nBufLen - (m_nBufSize - m);
  l = k - m_nSmartCache; // first
  if (l < 0) return; // in smartcache
  p = m_pBuffer;
  p += m_nBufSize + l;
  Move(m_pBuffer, p, l);
  m_nSmartCache = k;
}

#endif /* HPCB_CPP_ */


