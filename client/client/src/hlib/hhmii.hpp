/*
 * hhmii.hpp
 *
 *  Created on: 2013-9-18
 *      Author: root
 */

#ifndef HHMII_HPP_
#define HHMII_HPP_

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
#include "hcrc32.hpp"
#include "hsp.hpp"

using namespace std;

namespace hhmii
{

//{$DEFINE H_HASH_DIRECT}
//{$DEFINE H_HASH_PRECHECK}
#define H_SIMPLE_KEY_TYPE
#define H_SIMPLE_VALUE_TYPE
//{$DEFINE H_EMPTY_VALUE_IS_OBJECT}

const int H_HASH_STRING_COUNT = 256;
enum THHashAddResult {harOK, harOverflow, harDupKey};

typedef unsigned char UCHAR;
typedef UCHAR* PUCHAR;

typedef struct THHashFindResult
{
  bool Exists;
  bool Occupy;
  bool Direct;
} *PHHashFindResult;

typedef struct THHashString
{
  int len;
  char buf[H_HASH_STRING_COUNT];
} *PHHashString;

typedef int THElementNameType;
typedef int THElementValueType;

const int H_KEY_SIZE = sizeof(THElementNameType);
const int H_HASHTABLE_SIZE = 4096;

typedef THElementNameType* PHElementNameType;
typedef THElementValueType* PHElementValueType;

#ifdef H_SIMPLE_KEY_TYPE
typedef THElementNameType THElementNameTypeF;
#else
typedef PHElementNameType THElementNameTypeF;
#endif

#ifdef H_SIMPLE_VALUE_TYPE
typedef THElementValueType THElementValueTypeF;
#else
typedef PHElementValueType THElementValueTypeF;
#endif

typedef struct THElement
{
	uint Index;   // For fast index control
	bool Using;    // DO NOT MOVE OR REMOVE THIS!
	// for treetable
	int bf;
	THElement* parent; // For hashtable, this chain is for reducing complexation of insert and delete ( Hash of element in the chain is different )
	THElement* left;   // If prev is nil, it means this is head.
	THElement* right;  // If prev is not nil, this is foot.
	// end
	THElementNameType Key;
	#ifdef H_HASH_PRECHECK
	uint KeyHash;
	#endif
	THElementValueType Value;
} *PHElement;

const int H_ELEMENT_SIZE = sizeof(THElement);
const int H_ELEMENT_COPY_SIZE = H_ELEMENT_SIZE - 4;
const int H_VALUE_SIZE = sizeof(THElementValueType);

typedef PHElement THElementPath[32];
typedef THElementPath* PHElementPath;

class THHMII
{
  private:
 	//{* need to modify for detail type of Key *}
	uint Hash(THElementNameTypeF Key);
	int Compare(PHElement e1, PHElement e2);
	#ifdef H_HASH_PRECHECK
	int CompareEx(THElementNameTypeF Key, uint Code, PHElement Element);
	#else
	int CompareEx(THElementNameTypeF Key, PHElement Element);
	#endif
	//{* end *}

  protected:
	uint m_nMask;
	PHElement GetElementByIndex(uint Index);
	PHElement GetFreeElement();
	bool RotateSubTree(PHElementPath path, int bf, int p);
	void RemoveNode(PHElementPath path, PHElement node, int p);
	void ReleaseNode(PHElement node);
	PHElement LL(PHElement root);
	PHElement RR(PHElement root);
	PHElement LR(PHElement root);
	PHElement RL(PHElement root);
	void ExchangeElement(PHElement root, PHElement new_);
	//{* for robust in extreme environment *}
    void ReleaseBuffer(PHElement push);
    PHElement AcquireBuffer();
  public:
    uint m_nDictSize;

    PHElement m_eDict;
	uint m_nCount;
	THSP *m_qFreeBuffer;
	PHElement Dict(uint Index); //  Read GetElementByIndex; Default;
	int Count();
	THHashAddResult Add(const THElementNameTypeF AKey, const THElementValueTypeF AValue);
	bool Remove(const THElementNameTypeF AKey);
	bool Exists(const THElementNameTypeF Key);
	bool IsFull();
	bool IsEmpty();
	PHElement FindFirst();
	PHElement FindNext(PHElement e);
	PHElement Find(const THElementNameTypeF Key);
	PHElement FindEx(const THElementNameTypeF Key, PHHashFindResult fr);
	THElementValueTypeF GetValue(const THElementNameTypeF Key);
	int CacheSize();
	void Clear();
	THHMII(uint Size = H_HASHTABLE_SIZE);
	~THHMII();
};

}

#endif /* HHMSI_HPP_ */
