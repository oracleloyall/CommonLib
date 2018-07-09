/*
 * hvcb.hpp
 *
 *  Created on: 2014-12-22
 *      Author: masanari
 */

#ifndef HVCB_HPP_
#define HVCB_HPP_

#include "global.hpp"
#include "hpasutils.hpp"
#include "hsp.hpp"

const int VCB_DEFAULT_SIZE = 4096;

class THVCB
{
	private:
	  pByte m_pInnerBuffer;
	  pByte m_pOuterBuffer;
	  int m_nBufSize;
	  pByte m_pHead;
	  pByte m_pCacheHead; // on the outer buffer
	  pByte m_pCacheTail; // on the inner buffer
	  int m_nSmartCache;
      int m_nFreeCacheSize;
      int m_nInnerBufferValid;
      int m_nOuterBufferValid;
	private:
	  bool IsOverFlowBuffer(int nLen);
	public:
	  int m_nValidCount;
	  void SetEmpty();
	  int GetSafeValidCount();
	  bool HeadSafePosInc(int nStep);
	  bool WriteCirBuffer(pByte pBuffer, int m_nBufLen);
	  void GetBufferData(pByte* pBuffer, int m_nBufLen, int m_nPos);
	  void Defrag();
	  THVCB(int nSize = VCB_DEFAULT_SIZE); // The length of buffer must be twice of the max package
	  ~THVCB();
};

#endif /* HVCB_HPP_ */
