/*
 * hsrswc.hpp
 *
 *  Created on: 2015-2-4
 *      Author: masanari
 */

#ifndef HSRSWC_HPP_
#define HSRSWC_HPP_

#include "global.hpp"
#include "hpasutils.hpp"
#include "hsp.hpp"

const int SRSWC_DEFAULT_SIZE = 4096;


typedef struct THDataNode
{
	THPointer sign;
	THPointer data;
} *PHDataNode;

const int H_SRSC_NODE_SIZE = sizeof(THDataNode);

class THSRSWC // single read single write controller
{
	private:
	  PHDataNode buf;
	  int m_nBufSize;
	  int m_readpos;
	  int m_writepos;
	public:
	  bool Read(THPointer& x);
	  bool CanRead();
	  bool Write(THPointer x);
	  bool CanWrite();
	  void SetEmpty();
	  THSRSWC(int nSize = SRSWC_DEFAULT_SIZE);
	  ~THSRSWC();
};


#endif /* HSRSWC_HPP_ */
