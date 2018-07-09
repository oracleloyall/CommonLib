/*
 * hxcb.hpp
 *
 *  Created on: 2015-1-4
 *      Author: masanari
 */

#ifndef HXCB_HPP_
#define HXCB_HPP_

#include "global.hpp"
#include "hpasutils.hpp"

const int XCB_DEFAULT_SIZE = 4096;

class THXCB
{
	private:
		pByte m_pBuffer;
		pByte m_pReadPos;
		pByte m_pWritePos;
		int m_nBufSize;
		int m_nReadCounter;
		int m_nWriteCounter;
		int m_nSmartCache;
	private:
		bool IsOverFlowBuffer(int nLen);
	public:
		int m_nValidCount;
		int m_nBufferCount;
		void SetEmpty();
		int GetSafeValidCount();
		bool HeadSafePosInc(int nStep);
		bool WriteCirBuffer(pByte pBuffer, int m_nBufLen);
		void GetBufferData(pByte* pBuffer, int m_nBufLen, int m_nPos);
		THXCB(int nSize = XCB_DEFAULT_SIZE); // The length of buffer must be twice of the max package
		~THXCB();
};

#endif /* HXCB_HPP_ */
