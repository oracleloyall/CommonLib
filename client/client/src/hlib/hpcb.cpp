/*
 * hpcb.cpp
 *
 *  Created on: 2013-9-25
 *      Author: root
 */

#ifndef HPCB_CPP_
#define HPCB_CPP_

#include "hpcb.hpp"
#include "logout.hpp"

THCB::THCB(int nSize)
{
  m_nBufSize = nSize;
  m_pBuffer = (pByte)malloc(m_nBufSize);
  SetEmpty();
}

THCB::~THCB()
{
  free(m_pBuffer);
}

void THCB::SetEmpty()
{
  m_nValidCount = 0;
  m_nBufferCount = m_nBufSize;
  m_pReadPos = m_pBuffer;
  m_pWritePos = m_pBuffer;
  m_nReadCounter = 0;
  m_nWriteCounter = 0;
}

int THCB::GetSafeValidCount()
{
  return m_nValidCount;
}

bool THCB::IsOverFlowBuffer(int nLen)
{
  //Result := nLen >= m_nBufSize - m_nValidCount;
  return nLen > m_nBufferCount;
}

bool THCB::HeadSafePosInc(int nStep)
{
  if ((nStep > m_nValidCount) || (nStep < 0)) return false;
  m_nValidCount -= nStep;
  m_nReadCounter += nStep;
  m_pReadPos += nStep;
  if (m_nReadCounter >= m_nBufSize)
  {
    m_pReadPos -= m_nBufSize;
    m_nReadCounter -= m_nBufSize;
  }
  m_nBufferCount += nStep;
  return true;
}

bool THCB::WriteCirBuffer(pByte pBuffer, int m_nBufLen)
{
  if ((m_nBufLen > m_nBufferCount) || (m_nBufLen < 0)) return false;
 // logout("THCB:m_nBufLen=%d m-nBufferCount=%d\n",m_nBufLen,m_nBufferCount);
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

void THCB::GetBufferData(pByte pBuffer, int m_nBufLen, int m_nPos)
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
  if ((j >= m_nBufSize) == (k > m_nBufSize)) // copy once
    Move(p, pBuffer, m_nBufLen);
  else // copy twice
  {
    l = m_nBufSize - m; // first
    Move(p, pBuffer, l);
    pBuffer += l;
    l = m_nBufLen - l;
    Move(m_pBuffer, pBuffer, l);
  }
}

#endif /* HPCB_CPP_ */
