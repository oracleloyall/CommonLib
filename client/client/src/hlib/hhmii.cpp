/*
 * hhmii.cpp
 *
 *  Created on: 2013-9-18
 *      Author: root
 */
#ifndef HHMII_CPP_
#define HHMII_CPP_

#include "hhmii.hpp"

namespace hhmii
{

PHElement THHMII::AcquireBuffer()
{
	PHElement p = m_eDict;
	for(uint i = 1; i < m_nDictSize; i++)
	{
		if (!p->Using) m_qFreeBuffer->Push(p);
		p++;
	}
	return (PHElement)(m_qFreeBuffer->PopValue());
}

PHElement THHMII::GetFreeElement()
{
	if (m_nDictSize == m_nCount) return NULL;
	THItemPointer p;
	do
	{
		p = m_qFreeBuffer->Pop();
		if (!p) return AcquireBuffer();
	}
	while ((*((PHElement)(*p))).Using);
	return (PHElement)(*p);
}

int THHMII::CacheSize()
{
	int ret = 0;
	PHElement p = m_eDict;
	int j = m_nDictSize - 1;
	for(int i = 0; i < j; i++)
	{
		if (!(*p++).Using) ret++;
	}
	return ret;
}

inline uint THHMII::Hash(THElementNameTypeF Key)
{
	return Key;
}

bool THHMII::RotateSubTree(PHElementPath path, int bf, int p)
{
	int leftBF, rightBF;

	bool tallChange = true;
	PHElement root = (*path)[p];
	PHElement newRoot = NULL;

	if (bf == 2)
	{
		leftBF = root->left->bf;
		if (leftBF == -1)
			newRoot = LR(root);
		else if (leftBF == 1)
			newRoot = LL(root);
		else // bf of left child is 0, only happens when delete
		{
			newRoot = LL(root);
			tallChange = false;
		}
	}

	if (bf == -2)
	{
		rightBF = root->right->bf;
		if (rightBF == 1)
			newRoot = RL(root);
		else if (rightBF == -1)
			newRoot = RR(root);
		else // bf of left child is 0, only happens when delete
		{
			newRoot = RR(root);
			tallChange = false;
		}
	}

	if (p > 0)
	{
		PHElement e = (*path)[p - 1];
		if (Compare(root, e) < 0) e->left = newRoot; else e->right = newRoot;
		newRoot->parent = e;
	}
	else
		ExchangeElement(root, newRoot); // root changed, force memory change

	return tallChange;
}

PHElement THHMII::LL(PHElement root)
{
	PHElement rootNext = root->left;
	root->left = rootNext->right;
	rootNext->right = root;
	// add for parent
	rootNext->parent = root->parent;
	root->parent = rootNext;
	if ((*root).left) (*(*root).left).parent = root;
	// add end
	if ((*rootNext).bf == 1)
	{
		(*root).bf = 0;
		(*rootNext).bf = 0;
	}
	else // rootNext.BF = 0, for delete
	{
		(*root).bf = 1;
		(*rootNext).bf = -1;
	}
	return rootNext;
}

PHElement THHMII::LR(PHElement root)
{
 	PHElement rootNext = (*root).left;
 	PHElement newRoot = (*rootNext).right;
 	(*root).left = (*newRoot).right;
 	(*rootNext).right = (*newRoot).left;
 	(*newRoot).left = rootNext;
 	(*newRoot).right = root;
 	// add for parent
 	(*newRoot).parent = (*root).parent;
 	(*root).parent = newRoot;
 	(*rootNext).parent = newRoot;
 	if ((*root).left) (*(*root).left).parent = root;
 	if ((*rootNext).right) (*(*rootNext).right).parent = rootNext;
 	// add end
 	switch ((*newRoot).bf)
 	{
 		case 0:
 		{
 			(*root).bf = 0;  (*rootNext).bf = 0; break;
 		}
 		case 1:
 		{
 			(*root).bf = -1; (*rootNext).bf = 0; break;
 		}
 		case -1:
 		{
 			(*root).bf = 0; (*rootNext).bf = 1; break;
 		}
 	}
 	(*newRoot).bf = 0;
 	return newRoot;
}

PHElement THHMII::RR(PHElement root)
{
	PHElement rootNext = (*root).right;
	(*root).right = (*rootNext).left;
	(*rootNext).left = root;
	// add for parent
	(*rootNext).parent = (*root).parent;
	(*root).parent = rootNext;
	if ((*root).right) (*(*root).right).parent = root;
	// add end
	if ((*rootNext).bf == -1)
	{
		(*root).bf = 0;
		(*rootNext).bf = 0;
	}
	else // rootNext.BF = 0, for delete
	{
		(*root).bf = -1;
		(*rootNext).bf = 1;
	}
	return rootNext;
}

PHElement THHMII::RL(PHElement root)
{
	PHElement rootNext = (*root).right;
	PHElement newRoot = (*rootNext).left;
	(*root).right = (*newRoot).left;
	(*rootNext).left = (*newRoot).right;
	(*newRoot).right = rootNext;
	(*newRoot).left = root;
	// add for parent
	(*newRoot).parent = (*root).parent;
	root->parent = newRoot;
	rootNext->parent = newRoot;
	if (root->right) root->right->parent = root;
	if (rootNext->left) rootNext->left->parent = rootNext;
	// add end
	switch (newRoot->bf)
	{
		case 0:
		{
			root->bf = 0; rootNext->bf = 0; break;
		}
		case 1:
		{
			root->bf = 0; rootNext->bf = -1; break;
		}
		case -1:
		{
			root->bf = 1; rootNext->bf = 0; break;
		}
	}
	newRoot->bf = 0;
  	return newRoot;
}

void THHMII::ExchangeElement(PHElement root, PHElement new_)
{
	THElement e;
	// 1. prepare
	void* ep = &e.Using;
	void* rp = &root->Using;
	void* np = &new_->Using;
	// 2. copy
	memcpy(ep, rp, H_ELEMENT_COPY_SIZE);
	memcpy(rp, np, H_ELEMENT_COPY_SIZE);
	memcpy(np, ep, H_ELEMENT_COPY_SIZE);
	// 3. fix: root parent is OK, no need modify; Fix childs
	bool r = false;

    if (root->left)
    {
    	if (root->left == root)
    	{
    		root->left = new_;
    		new_->parent = root;
    		r = true;
    	}
    	else
    		root->left->parent = root;
    }

    do {

    if (root->right)
    {
    	if (root->right == root)
    	{
    		root->right = new_;
    		new_->parent = root;
    		break;
    	}
    	else
    		root->right->parent = root;
    }

  // to fix new root, 2 conditions: new root has relation or not
  // 1. is child of root, parent is already fixed
  // 2. no relation, parent is old, change father's child

    if (!r)
    {
    	if (new_->parent->left == root)
    		new_->parent->left = new_;
    	else
    		new_->parent->right = new_;
    }
  } while (false);

  if (new_->left) new_->left->parent = new_;
  if (new_->right) new_->right->parent = new_;
}

inline int THHMII::Compare(PHElement e1, PHElement e2)
{
	if (e1->Key < e2->Key)
		return -1;
	else if (e1->Key > e2->Key)
		return 1;
	else
        return 0;
}

//#ifdef H_HASH_PRECHECK
inline int THHMII::CompareEx(THElementNameTypeF Key, /*uint Code,*/ PHElement Element)
{
  if (Key < Element->Key)
   return -1;
  else if (Key > Element->Key)
   return 1;
  else
   return 0;
}
//#endif

THHashAddResult THHMII::Add(const THElementNameTypeF AKey, const THElementValueTypeF AValue)
{
  // 1, check full
  if (m_nDictSize == m_nCount) return harOverflow;
  // 2, check direct
  uint hc; int i; int bf; PHElement current;
  #ifdef H_SIMPLE_KEY_TYPE
  hc = (uint)AKey;
  #else
  hc = 0;
  //CalcCRC32(pByte(@AKey.buf[0]), AKey.len, hc);
  for (i = 0; i < AKey->len; i++)
	  hc = ((hc << 2) | (hc >> (sizeof(hc) * 8 - 2))) xor AKey->buf[i];
  #endif

  uint idx = hc & m_nMask;
  PHElement node = m_eDict;
  node += idx;
  if (!node->Using) // can direct
  {
    // 2.1 direct set
    node->Using = true;
	#ifdef H_SIMPLE_KEY_TYPE
    node->Key = AKey;
    #else
    memcpy(&node->Key, AKey, H_KEY_SIZE);
	#endif
	#ifdef H_HASH_PRECHECK
    node->KeyHash = hc;
	#endif
	#ifdef H_SIMPLE_VALUE_TYPE
    node->Value = AValue;
	#else
    memcpy(&node->Value, AValue, H_VALUE_SIZE);
	#endif
    node->bf = 0;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    m_nCount++;
    return harOK;
  }

  THElementPath path;
  THItemPointer buf;
  // 3, occupied, check whether match or not
  if (!node->parent) // chain is OK;
  {
    // 3.1 check pos
    int p = 0; i = 0;
    PHElement prev = NULL; PHElement current = node;
    while (current)
    {
      path[p++] = current;
	  #ifndef H_SIMPLE_KEY_TYPE
      i = CompareEx(AKey, current);
	  #else
      if ((int)AKey == (int)current->Key)
       i = 0;
      else if ((int)AKey < (int)current->Key)
       i = -1;
      else
       i = 1;
	  #endif
      if (!i) return harDupKey;
      prev = current;
      if (i < 0) current = prev->left; else current = prev->right;
    }

    do {
     buf = m_qFreeBuffer->Pop();
     if (!buf)
     {
       current = AcquireBuffer();
       break;
     }
     current = (PHElement)(*buf);
    } while (current->Using);

    if (i < 0) prev->left = current; else prev->right = current;
    path[p] = current;
    // 3.2 save data
    current->Using = true;
	#ifdef H_SIMPLE_KEY_TYPE
    current->Key = AKey;
	#else
    memcpy(&current->Key, AKey, H_KEY_SIZE);
	#endif
	#ifdef H_HASH_PRECHECK
    current->KeyHash = hc;
	#endif
	#ifdef H_SIMPLE_VALUE_TYPE
    current->Value = AValue;
	#else
    memcpy(&current->Value, AValue, H_VALUE_SIZE);
	#endif
    current->bf = 0;
    current->parent = prev;
    current->left = NULL;
    current->right = NULL;

    // 3.3 check balance and rotate
    m_nCount++;
    while (p > 0)
    {
      p--;
	  #ifndef H_SIMPLE_KEY_TYPE
      i = CompareEx(AKey, path[p]);
      if (i < 0) bf = 1; else bf = -1;
	  #else
      if (AKey == path[p]->Key)
       bf = 0;
      else if ((int)AKey < (int)(path[p]->Key))
       bf = 1;
      else
       bf = -1;
	  #endif
      path[p]->bf += bf;
      bf = path[p]->bf;
      switch (bf)
      {
      	  case 0: return harOK;
      	  case 2:
      	  case -2:
      	  {
      		  RotateSubTree(&path, bf, p);
      		  return harOK;
      	  }
      }
    }
    return harOK;
  }
  // 4, occupied, and not match; alloc and exchange

  do {
   buf = m_qFreeBuffer->Pop();
   if (!buf)
   {
     current = AcquireBuffer();
     break;
   }
   current = (PHElement)(*buf);
  } while (current->Using);

  current->Using = true;
  #ifdef H_SIMPLE_KEY_TYPE
  current->Key = AKey;
  #else
  memcpy(&current->Key, AKey, H_KEY_SIZE);
  #endif
  #ifdef H_HASH_PRECHECK
  current->KeyHash = hc;
  #endif
  #ifdef H_SIMPLE_VALUE_TYPE
  current->Value = AValue;
  #else
  memcpy(&current->Value, AValue, H_VALUE_SIZE);
  #endif
  current->bf = 0;
  current->parent = NULL;
  current->left = NULL;
  current->right = NULL;
  ExchangeElement(node, current);
  m_nCount++;
  return harOK;
}

bool THHMII::Remove(const THElementNameTypeF AKey)
{
  //hc, idx: Cardinal;
  //node: ;
  //path: THElementPath;
  //p{$IFNDEF H_SIMPLE_KEY_TYPE}, i{$ENDIF}: Integer;

  // 2, check root
  uint hc; uint idx; int i;
  #ifdef H_SIMPLE_KEY_TYPE
  hc = uint(AKey);
  #else
  hc = 0;
  //CalcCRC32(pByte(@AKey.buf[0]), AKey.len, hc);
  for (i = 0; i < AKey->len; i++)
	hc = ((hc << 2) | (hc >> (sizeof(hc) * 8 - 2))) xor AKey->buf[i];
  #endif
  idx = hc & m_nMask;
  PHElement node = m_eDict;
  node += idx;
  if ((!node->Using) || (node->parent)) return false;
  // 3, do tree search
  int p = -1;
  THElementPath path;
  while (node)
  {
    p++;
    path[p] = node;
	#ifndef H_SIMPLE_KEY_TYPE
    i = CompareEx(AKey, node);
    if (!i)
    {
      RemoveNode(&path, node, p);
      return true;
    }
    else if (i < 0)
     node = node->left;
    else
     node = node->right;
	#else
    if (AKey == node->Key)
    {
      RemoveNode(&path, node, p);
      return true;
    }
    else if (AKey < node->Key)
     node = node->left;
    else
     node = node->right;
	#endif
  }
  return false;
}

inline void THHMII::ReleaseNode(PHElement node)
{
  node->Using = false;
  //If m_qFreeBuffer.m_nSize = m_qFreeBuffer.m_nCount Then
  if (m_qFreeBuffer->m_pPos == m_qFreeBuffer->m_pMax)
   ReleaseBuffer(node);
  else
   m_qFreeBuffer->Push(node);
  m_nCount--;
}

void THHMII::RemoveNode(PHElementPath path, PHElement node, int p)
{
  PHElement t; int bf;

  // 1. delete
  PHElement tmp = NULL;
  if ((node->left) && (node->right))
  {
    tmp = node->left;
    (*path)[++p] = tmp;
    while (tmp->right)
    {
      tmp = tmp->right;
      (*path)[++p] = tmp;
    }
	#ifdef H_HASH_PRECHECK
    memcpy(&node->Key, &tmp->Key, H_VALUE_SIZE + H_KEY_SIZE + 4);
	#else
    memcpy(&node->Key, &tmp->Key, H_VALUE_SIZE + H_KEY_SIZE);
	#endif
    t = (*path)[p - 1];
    if (t == node) t->left = tmp->left; else t->right = tmp->left;
    if (tmp->left) tmp->left->parent = t;
    tmp->Using = false;
    //If m_qFreeBuffer.m_nSize = m_qFreeBuffer.m_nCount Then
    if (m_qFreeBuffer->m_pPos == m_qFreeBuffer->m_pMax)
     ReleaseBuffer(tmp);
    else
     m_qFreeBuffer->Push(tmp);
    m_nCount--;
  }
  else
  {
    tmp = node->left;
    if (!tmp) tmp = node->right;
    if (p > 0)
    {
      t = (*path)[p - 1];
      if (t->left == node) t->left = tmp; else t->right = tmp;
      if (tmp) tmp->parent = t;
      node->Using = false;
      //If m_qFreeBuffer.m_nSize = m_qFreeBuffer.m_nCount Then
      if (m_qFreeBuffer->m_pPos == m_qFreeBuffer->m_pMax)
       ReleaseBuffer(node);
      else
       m_qFreeBuffer->Push(node);
      m_nCount--;
    }
    else
    {
      if (tmp) // node is root, check child
      {
         memcpy(&node->Using, &tmp->Using, H_ELEMENT_COPY_SIZE);
         node->parent = NULL;
         node = tmp;
      }
      node->Using = false;
      //If m_qFreeBuffer.m_nSize = m_qFreeBuffer.m_nCount Then
      if (m_qFreeBuffer->m_pPos == m_qFreeBuffer->m_pMax)
       ReleaseBuffer(node);
      else
       m_qFreeBuffer->Push(node);
      m_nCount--;
    }
  }

  // 2, rotate
  while (p > 0)    // bf表示平衡因子的改变量，当删除的是左子树中的结点时，平衡因子-1
  {                // 当删除的是右子树的孩子时，平衡因子+1
    t = (*path)[--p];
	#ifndef H_SIMPLE_KEY_TYPE
    if (Compare(node, t) <= 0) bf = -1; else bf = 1;
	#else
    if ((int)node->Key <= (int)t->Key) bf = -1; else bf = 1;
	#endif
    t->bf += bf;
    bf = t->bf;
    if (bf) // 如果bf==0，表明高度降低，继续后上回溯
      // 如果bf为1或-1则说明高度未变，停止回溯，如果为2或-2，则进行旋转
      // 当旋转后高度不变，则停止回溯
     if ((bf == 1) || (bf == -1) || (!RotateSubTree(path, bf, p))) break;
  }
}

bool THHMII::Exists(const THElementNameTypeF Key)
{
  return Find(Key);
}

PHElement THHMII::Find(const THElementNameTypeF Key)
{
  uint hc; uint idx;
  int i;
  // 2. check direct
  #ifdef H_SIMPLE_KEY_TYPE
  hc = (uint)Key;
  #else
  hc = 0;
  //CalcCRC32(pByte(@AKey.buf[0]), AKey.len, hc);
  for (i = 0; i < Key->len; i++)
  	hc = ((hc << 2) | (hc >> (sizeof(hc) * 8 - 2))) xor Key->buf[i];
  #endif
  idx = hc & m_nMask;
  PHElement node = m_eDict + idx;
  if ((!node->Using) || (node->parent)) return NULL;
  // 3. node is occupied, searching in the chain
  do {
   #ifndef H_SIMPLE_KEY_TYPE
   i = CompareEx(Key, node);
   if (!i)
	return node;
   else
    if (i < 0)
     if (node->left)
      node = node->left;
     else
      return NULL;
    else if (node->right)
     node = node->right;
    else
     return NULL;
   #else
   if (Key == node->Key)
	return node;
   else if ((int)Key < (int)node->Key)
    if (node->left)
     node = node->left;
    else
     return NULL;
   else
    if (node->right)
     node = node->right;
    else
     return NULL;
   #endif
  } while (true);
}

PHElement THHMII::FindEx(const THElementNameTypeF Key, PHHashFindResult fr)
{
  int i;
  fr->Occupy = false;
  // 1. no data, pass
  if (IsEmpty())
  {
    fr->Exists = false;
    return NULL;
  }
  // 2. check direct
  uint hc = Hash(Key);
  //idx := hc Mod m_nDictSize;
  uint idx = hc & m_nMask;
  // If idx > m_nDictSize Then Dec(idx, m_nDictSize);
  PHElement node = m_eDict;
  node += idx;
  if (!node->Using)
  {
    fr->Exists = false;
    return NULL;
  }
  if (node->parent)
  {
    fr->Exists = false;
    fr->Occupy = true;
    return NULL;
  }
  // 3. node is occupied
  i = CompareEx(Key, node);
  if (!i)
  {
    fr->Exists = true;
    fr->Direct = true;
    return node;
  }
  // 4. searching in the chain
  fr->Direct = false;
  do {
   if (i < 0)
    if (node->left)
     node = node->left;
    else
    {
      fr->Exists = false;
      return NULL;
    }
   else
    if (node->right)
     node = node->right;
    else
    {
      fr->Exists = false;
      return NULL;
    }
   i = CompareEx(Key, node);
  } while (i);
  fr->Exists = true;
  return node;
}

THElementValueTypeF THHMII::GetValue(const THElementNameTypeF Key)
{
 // hc, idx: Cardinal;
 // node: ;
 // i: Integer;
  int i; PHElement node;
  uint hc; uint  idx;
  // 2. check direct
  #ifdef H_SIMPLE_KEY_TYPE
  hc = (uint)Key;
  #else
  hc = 0;
  //CalcCRC32(pByte(@AKey.buf[0]), AKey.len, hc);
  for (i = 0; i < Key->len; i++)
  	hc = ((hc << 2) | (hc >> (sizeof(hc) * 8 - 2))) xor Key->buf[i];
  #endif
  idx = hc & m_nMask;
  node = m_eDict;
  node += idx;
  if ((!node->Using) || (node->parent)) return NULL;
  // 3. node is occupied, searching in the chain
  do {
   #ifndef H_SIMPLE_KEY_TYPE
   i = CompareEx(Key, node);
   if (!i)
    #ifndef H_SIMPLE_VALUE_TYPE
	return &node->Value;
	#else
    return node->Value;
	#endif
   else
    if (i < 0)
     if (node->left)
      node = node->left;
     else
      return NULL;
    else if (node->right)
     node = node->right;
    else
     return NULL;
   #else
   if (Key == node->Key)
	#ifndef H_SIMPLE_VALUE_TYPE
	return &node->Value;
	#else
	return node->Value;
	#endif
   else if ((int)Key < (int)node->Key)
    if (node->left)
     node = node->left;
    else
     return 0;
   else
    if (node->right)
     node = node->right;
    else
     return 0;
   #endif
  } while (true);
}

void THHMII::Clear()
{
  m_nCount = 0;
  PHElement p = m_eDict;
  m_qFreeBuffer->Clear();
  for(uint i = 0; i < m_nDictSize; i++)
  {
    p->Index = i;
    p->Using = false;
    m_qFreeBuffer->Push(p++);
  }
}

THHMII::THHMII(uint Size)
{
  m_nDictSize = Size;
  // normalize
  m_nMask = 1;
  do {
   m_nMask <<= 1;
  } while (m_nMask < m_nDictSize);
  m_nDictSize = m_nMask--;
  // end;
  m_eDict = (PHElement)malloc(m_nDictSize * H_ELEMENT_SIZE);
  THSP *tmp = new THSP((int)m_nDictSize << 1);
  m_qFreeBuffer = tmp;
  Clear();
}

THHMII::~THHMII()
{
  delete m_qFreeBuffer;
  free(m_eDict);
}

bool THHMII::IsFull()
{
  return m_nDictSize == m_nCount;
}

bool THHMII::IsEmpty()
{
  return !m_nCount;
}

int THHMII::Count()
{
  return m_nCount;
}

PHElement THHMII::FindFirst()
{
  if (IsEmpty()) return NULL;
  PHElement ret = m_eDict;
  for(uint i = 0; i < m_nDictSize; i++)
  {
    if (ret->Using) return ret;
    ret++;
  }
  return NULL;
}

PHElement THHMII::FindNext(PHElement e)
{
  if (IsEmpty()) return NULL;
  if (e->Index == m_nDictSize - 1) return NULL;
  PHElement ret = m_eDict;
  ret += e->Index + 1;
  for(uint i = e->Index + 1; i < m_nDictSize; i++)
  {
    if (ret->Using) return ret;
    ret++;
  }
  return NULL;
}

inline PHElement THHMII::GetElementByIndex(uint Index)
{
  return m_eDict + Index;
}

void THHMII::ReleaseBuffer(PHElement push)
{
  THItemPointer head = m_qFreeBuffer->m_apDict;
  do {
    if (!((PHElement)(*head))->Using)
    {
      m_qFreeBuffer->m_pPos--;
      m_nCount--;
      *head = *(m_qFreeBuffer->m_pPos);
    }
    else
     head++;
   } while (head != m_qFreeBuffer->m_pPos);
  m_qFreeBuffer->Push(push);
}

}

#endif /* HHMII_CPP_ */
