/*
 * hsp.cpp
 *
 *  Created on: 2013-9-18
 *      Author: root
 */

#ifndef HSP_CPP_
#define HSP_CPP_

#include "hsp.hpp"

bool THSP::Push(THItemType p)
{
  if (m_pPos == m_pMax) return false;
  *m_pPos = p;
  m_pPos++;
  return true;
}

THItemPointer THSP::Pop()
{
  if (m_pPos == m_apDict) return NULL;
  return --m_pPos;
}

THItemType THSP::PopValue()
{
  if (m_pPos == m_apDict) return NULL;
  return *(--m_pPos);
}

void THSP::Clear()
{
  m_pPos = m_apDict;
}

int THSP::Count()
{
  return ((THPointer)m_pPos - (THPointer)m_apDict) >> H_STEP;
}

THSP::THSP(int Size)
{
  m_nSize = Size;
  m_apDict = (THItemPointer)malloc(m_nSize << H_STEP);
  m_pPos = m_apDict;
  m_pMax = m_apDict;
  m_pMax += m_nSize;
}

THSP::~THSP()
{
  m_pPos = m_apDict;
  free(m_apDict);
}

bool THSP::IsFull()
{
  return m_pPos == m_pMax;
}

bool THSP::IsEmpty()
{
  return m_pPos == m_apDict;
}

THItemPointer THSP::Items(int index)
{
  return THItemPointer(THPointer(m_apDict) + (index >> H_STEP));
}

#endif /* HSP_CPP_ */
