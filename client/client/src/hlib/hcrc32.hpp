/*
 * crc32.hpp
 *
 *  Created on: 2013-9-19
 *      Author: root
 */

#ifndef CRC32_HPP_
#define CRC32_HPP_

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

extern uint crc32(uint PartialCrc, char* Buffer, uint Length);
extern void crc32(uint* crc, char* buf, int len);

#endif /* CRC32_HPP_ */
