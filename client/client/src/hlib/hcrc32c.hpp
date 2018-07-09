/*
 * hcrc32c.hpp
 *
 *  Created on: 2015-1-5
 *      Author: masanari
 */

#ifndef HCRC32C_HPP_
#define HCRC32C_HPP_

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

extern uint crc32c(uint PartialCrc, char* Buffer, int Length);
extern void crc32c(uint* crc, char* buf, int len);


#endif /* HCRC32C_HPP_ */
