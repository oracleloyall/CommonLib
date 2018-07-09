/*
 * hsp.hpp
 *
 *  Created on: 2013-9-18
 *      Author: root
 */

#ifndef HSP_HPP_
#define HSP_HPP_

#include "global.hpp"

#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/time.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <pthread.h>
#include <fcntl.h>
#include <list>

using namespace std;

typedef void* THItemType;
typedef THItemType* THItemPointer;
#ifdef H_64BIT
typedef uint64_t THPointer;
const int H_STEP = 3;
#else
typedef uint THPointer;
const int H_STEP = 2;
#endif

const int H_STACK_SIZE = 4096;
const int H_ITEM_SIZE = sizeof(THItemType);

class THSP
{
	public:
		int m_nSize;
		THItemPointer m_pPos;
		THItemPointer m_pMax;
		THItemPointer m_apDict;
		bool Push(THItemType p);
		THItemPointer Pop();
		THItemType PopValue();
		bool IsEmpty();
		bool IsFull();
		int Count();
		THItemPointer Items(int index);
		void Clear();
		THSP(int Size = H_STACK_SIZE);
		~THSP();
};


#endif /* HSP_HPP_ */
