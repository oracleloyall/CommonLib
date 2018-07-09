/*
 * hpcb.hpp
 *
 *  Created on: 2013-9-25
 *      Author: root
 */

#ifndef HPCB_HPP_
#define HPCB_HPP_

#include "global.hpp"
#include "hpasutils.hpp"

const int CB_DEFAULT_SIZE = 4096;

class THCB
{
	private:
		pByte m_pBuffer;
		pByte m_pReadPos;
		pByte m_pWritePos;
		int m_nBufSize;
		int m_nReadCounter;
		int m_nWriteCounter;
	private:
		bool IsOverFlowBuffer(int nLen);
	public:
		int m_nValidCount;
		int m_nBufferCount;
		void SetEmpty();
		int GetSafeValidCount();
		bool HeadSafePosInc(int nStep);
		bool WriteCirBuffer(pByte pBuffer, int m_nBufLen);
		void GetBufferData(pByte pBuffer, int m_nBufLen, int m_nPos);
		THCB(int nSize = CB_DEFAULT_SIZE); // The length of buffer must be twice of the max package
		~THCB();
};

#endif /* HPCB_HPP_ */
