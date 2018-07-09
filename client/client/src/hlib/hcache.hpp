/*
 * hcache.hpp
 *
 *  Created on: 2013-9-21
 *      Author: root
 */

#ifndef HCACHE_HPP_
#define HCACHE_HPP_

#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
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

const int H_DEFAULT_CACHE = 512;

typedef struct THCacheBlock
{
   THCacheBlock* prev;
   THCacheBlock* next;
   void* data;
   bool used;
} *PHCacheBlock;

typedef int (*TOnCacheLoop)(void* sender, PHCacheBlock block);

class THSCache
{
   public:
	int size;
    int fc; // free element counter
    PHCacheBlock buf;
    PHCacheBlock fb; // free element stack
    PHCacheBlock ub; // used element list
    bool Add(void* data); // for input
    int Loop(void* sender, TOnCacheLoop notify); // for output loop
    void Release(PHCacheBlock block); // for output event call
    THSCache(int ASize = H_DEFAULT_CACHE);
    ~THSCache();
};

class THQCache
{
   public:
	int size;
    int fc; // free element counter
    PHCacheBlock buf;
    PHCacheBlock fb; // free element stack
    PHCacheBlock uh; // used element queue head
    PHCacheBlock uf; // used element queue foot
    bool Add(void* data); // for input
    int Loop(void* sender, TOnCacheLoop notify); // for output loop
    void Release(PHCacheBlock block); // for output event call
    THQCache(int ASize = H_DEFAULT_CACHE);
    ~THQCache();
};

#endif /* HCACHE_HPP_ */
