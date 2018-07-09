/*
 * hcache.cpp
 *
 *  Created on: 2013-9-21
 *      Author: root
 */

#ifndef HCACHE_CPP_
#define HCACHE_CPP_

#include "hcache.hpp"

THSCache::THSCache(int ASize)
{
  size = ASize;
  buf = (PHCacheBlock)malloc(sizeof(THCacheBlock) * size);
  fc = size;
  fb = NULL;
  PHCacheBlock b = buf;
  for(int i = 0; i < size; i++)
  {
    b->next = fb;
    b->used = false;
    fb = b++;
  }
  ub = NULL;
}

THSCache::~THSCache()
{
  free(buf);
}

bool THSCache::Add(void* data) // for input
{
   if (!fb) return false;
   PHCacheBlock x = fb->next;    // new fb keep
   fb->data = data; // new ub set
   fb->next = ub;
   fb->prev = NULL;
   fb->used = true;
   if (ub) ub->prev = fb; // ub brother
   ub = fb;         // new ub
   fb = x;          // fb
   fc--;
   return true;
}

int THSCache::Loop(void* sender, TOnCacheLoop notify) // for output loop
{
  // 1. get head
  int i = 0;
  PHCacheBlock p = ub;
  PHCacheBlock n;
  // 2. run
  while (p)
  {
    n = p->next;
    try
    {
      i += (*notify)(sender, p);
    }
    catch(...)
    {
    }
    p = n;
  }
  return i;
}

void THSCache::Release(PHCacheBlock block) // for output event call
{
   // fix used-chain                       //    o1 o2 o3
   if (block->prev)
    block->prev->next = block->next;  // front
   else
    ub = block->next;                // base addr
   if (block->next) block->next->prev = block->prev; // backward
   block->used = false;
   block->data = NULL;
   block->next = fb;
   fb = block;
   fc++;
}

// new queue cache

THQCache::THQCache(int ASize)
{
  size = ASize;
  buf = (PHCacheBlock)malloc(sizeof(THCacheBlock) * size);
  fc = size;
  fb = NULL;
  PHCacheBlock b = buf;
  for(int i = 0; i < size; i++)
  {
    b->next = fb;
    b->used = false;
    fb = b++;
  }
  uh = NULL;
  uf = NULL;
}

THQCache::~THQCache()
{
  free(buf);
}

bool THQCache::Add(void* data) // for input
{
   if (!fb)
{
	   cout<<"hcace.cpp£¬add->dataÖ¸ÕëÎª¿Õ\n";
	   return false;
}
   PHCacheBlock x = fb->next;    // new fb keep
   fb->data = data; // new ub set
   fb->next = NULL;
   fb->prev = uf;
   fb->used = true;
   if (uf) uf->next = fb; // ub brother
   uf = fb;         // new ub
   if (!uh) uh = fb;
   //printf("%d, %d\n", (long long)uf, (long long)fb);
   fb = x;          // fb
   fc--;
   return true;
}

int THQCache::Loop(void* sender, TOnCacheLoop notify) // for output loop
{
  // 1. get head
  int i = 0;
  PHCacheBlock p = uh;
  PHCacheBlock n;
  // 2. run
  while (p)
  {
    n = p->next;
    try
    {
      i += (*notify)(sender, p);
    }
    catch(...)
    {
    }
    p = n;
  }
  return i;
}

void THQCache::Release(PHCacheBlock block) // for output event call
{
   // fix used-chain                       //    o1 o2 o3
   if (block->prev)
    block->prev->next = block->next;  // front
   else
    uh = block->next;                // base addr
   if (block->next)
	block->next->prev = block->prev; // backward
   else
	uf = NULL;
   block->used = false;
   block->data = NULL;
   block->next = fb;
   fb = block;
   fc++;
}

#endif /* HCACHE_CPP_ */
