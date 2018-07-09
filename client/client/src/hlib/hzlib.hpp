/*
 * hzlib.hpp
 *
 *  Created on: 2013-9-23
 *      Author: root
 */

#ifndef HZLIB_HPP_
#define HZLIB_HPP_

#include "global.hpp"
#include "hpasutils.hpp"
#include "../zlib/zlib.h"
#include "../mzip/zip.h"
#include <tr1/unordered_map>
#include <stdlib.h>
using namespace std::tr1;

typedef unordered_map<void*, int> umap;

extern int alloced;

extern bool CompressAFile(const string& OriginalFilename, bool deleteOld = true);
extern bool GZCompress(const char* src, int srclen, char* dst, int* dstlen);
extern bool MemZip(const char* FilenameInZip, const char* src, int srclen, char** dst, uint* dstlen);
extern bool Zip(const char* FilenameInZip, const char* src, int srclen, const char* FilenameOfZip);
extern int gzcompress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata);
extern int gzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);
extern bool GZDecompress(const char *zdata, int nzdata, char *data, int *ndata);

#endif /* HZLIB_HPP_ */
