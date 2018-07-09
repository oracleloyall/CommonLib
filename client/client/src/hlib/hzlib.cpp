/*
 * hzlib.cpp
 *
 *  Created on: 2013-9-23
 *      Author: root
 */

#ifndef HZLIB_CPP_
#define HZLIB_CPP_


#include "hzlib.hpp"

int alloced = 0;

voidpf my_alloc_func(voidpf opaque, uInt items, uInt size)
{
	uInt x = items * size;
	return malloc(x);
}

void my_free_func(voidpf opaque, voidpf address)
{
    free(address);
}

bool Zip(const char* FilenameInZip, const char* src, int srclen, const char* FilenameOfZip)
{
	char* dst = NULL;
	uint dstlen = 0;
	if (!MemZip(FilenameInZip, src, srclen, &dst, &dstlen)) return false;
	if (FileExists(string(FilenameOfZip))) DeleteFile(string(FilenameOfZip));
	FILE * fp = fopen(FilenameOfZip, "wb+");
	if (!fp) return false;
	fwrite(dst, dstlen, 1, fp);
    fclose(fp);
    free(dst);
    return true;
}

bool MemZip(const char* FilenameInZip, const char* src, int srclen, char** dst, uint* dstlen)
{
	// 1. alloc mem
	int izipbufsize = srclen + (srclen >> 3) + 1024;
	char *zipBuf = new char[izipbufsize];
	char *filename = new char[256];
	sprintf(filename, "%x+%x", (uint)zipBuf[0], (uint)izipbufsize);
	zlib_filefunc_def *pzlib_filefunc_def = new zlib_filefunc_def;
	fill_memory_filefunc(pzlib_filefunc_def);
	zipFile z1 = zipOpen2(filename, 0, NULL, pzlib_filefunc_def);
    if (!z1)
    {
    	free(zipBuf);
    	free(filename);
    	free(pzlib_filefunc_def);
    	return false;
    }
	zip_fileinfo zi;
	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;
	ZPOS64_T t1;
	if (zipOpenNewFileInZip(z1, FilenameInZip, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION))
	{
		zipClose(z1, NULL, &t1);
		free(zipBuf);
		free(filename);
		free(pzlib_filefunc_def);
		return false;
	}
	if (zipWriteInFileInZip(z1, src, srclen))
	{
		zipCloseFileInZip(z1);
		zipClose(z1, NULL, &t1);
		free(zipBuf);
		free(filename);
		free(pzlib_filefunc_def);
		return false;
	}
	if (zipCloseFileInZip(z1))
	{
		zipClose(z1, NULL, &t1);
		free(zipBuf);
		free(filename);
		free(pzlib_filefunc_def);
		return false;
	}
    if (zipClose(z1, NULL, &t1))
    {
		free(zipBuf);
		free(filename);
		free(pzlib_filefunc_def);
		return false;
    }
	*dst = zipBuf;
	*dstlen = t1;
	free(filename);
	free(pzlib_filefunc_def);
	return true;
}

bool CompressAFile(const string& OriginalFilename, bool deleteOld)
{
  // 1. prepare
  if (!FileExists(OriginalFilename)) return false;
  if (!GetFileSize(OriginalFilename)) return false;
  string ZipName = ChangeFileExt(OriginalFilename, ".zip");
  string WorkingName = ZipName + ".working";
  if (FileExists(WorkingName)) DeleteFile(WorkingName);
  // 2. save
  int l = GetFileSize(OriginalFilename);
  FILE * f = fopen(OriginalFilename.c_str(), "r");
  if (!f) return false;
  char* buf = new char[l];
  fread(buf, l, 1, f);
  fclose(f);
  string InnerName = ExtractFilename(OriginalFilename);
  Zip(InnerName.c_str(), buf, l, WorkingName.c_str());
  free(buf);
  // 3. clear
  if (!FileExists(WorkingName) || (!GetFileSize(WorkingName))) return false;
  if (FileExists(ZipName)) DeleteFile(ZipName);
  if (!RenameFile(WorkingName, ZipName)) return false;
  if (!deleteOld) return true;
  if (!DeleteFile(OriginalFilename)) return false;
  return true;
}

const Byte gzheader[10] = {0x1f, 0x8b, Z_DEFLATED, 0, 0, 0, 0, 0, 0, 3};
///const int DEF_MEM_LEVEL = 8;

bool GZCompress(const char* src, int srclen, char* dst, int* dstlen)
{
  int i, dlen, slen;
  uint x = 0;
  z_stream s;
  pByte	c;
  // Init
  slen = srclen;
  dlen = slen + (slen >> 7) + 18; // That should be enough
  // Translating AnsiString into pBytef
  memcpy(dst, gzheader, 10);
  // Initializing stream
  s.zalloc = (alloc_func)0;
  s.zfree = (free_func)0;
  s.opaque = (voidpf)0;
  s.next_in = (Bytef*)src;
  s.avail_in = slen;
  s.next_out = (Bytef*)(&dst[10]);
  s.avail_out = dlen - 10;
  // Entire deflation
  if (deflateInit2(&s, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY) != 0) return false;
  if (deflate(&s, Z_FINISH) != Z_STREAM_END) return false;
  if ((int)s.total_in != slen) return false;
  dlen = s.total_out;
  deflateEnd(&s);
  // Get CRC32
  x = crc32(x, Z_NULL, 0);
  x = crc32(x, (Bytef*)src, slen);
  // Write CRC32 and ISIZE in LSB order
  c = (Bytef*)(&dst[10 + dlen]);
  for (i = 0; i < 4; i++)
  {
	*c = x & 255;
	x = x >> 8;
	c++;
  }
  x = slen;
  for (i = 0; i < 4; i++)
  {
	*c = x & 255;
	x = x >> 8;
	c++;
  }
  *dstlen = dlen + 18;
  return true;
}

int gzcompress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata)
{
	z_stream c_stream;
	int err = 0;

	if(data && ndata > 0) {
		c_stream.zalloc = NULL; //my_alloc_func;
		c_stream.zfree = NULL; //my_free_func;
		c_stream.opaque = NULL;
		//只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
		if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
						MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) return -1;
		c_stream.next_in  = data;
		c_stream.avail_in  = ndata;
		c_stream.next_out = zdata;
		c_stream.avail_out  = *nzdata;
		while(c_stream.avail_in != 0 && c_stream.total_out < *nzdata) {
			if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
		}
		if(c_stream.avail_in != 0) return c_stream.avail_in;
		for(;;) {
			if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
			if(err != Z_OK) return -1;
		}
		if(deflateEnd(&c_stream) != Z_OK) return -1;
		*nzdata = c_stream.total_out;
		return 0;
	}
	return -1;
}

int gzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata)
 {
     int err = 0;
     z_stream d_stream = {0}; /* decompression stream */
     static char dummy_head[2] =
     {
         0x8 + 0x7 * 0x10,
         (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
     };
     d_stream.zalloc = (alloc_func)0;
     d_stream.zfree = (free_func)0;
     d_stream.opaque = (voidpf)0;
     d_stream.next_in  = zdata;
     d_stream.avail_in = 0;
     d_stream.next_out = data;
     if(inflateInit2(&d_stream, -MAX_WBITS) != Z_OK) return -1;
     //if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
     while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
         d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
         if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
         if(err != Z_OK )
         {
             if(err == Z_DATA_ERROR)
             {
                 d_stream.next_in = (Bytef*) dummy_head;
                 d_stream.avail_in = sizeof(dummy_head);
                 if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                 {
                     return -1;
                 }
             }
             else return -1;
         }
     }
     if(inflateEnd(&d_stream) != Z_OK) return -1;
     *ndata = d_stream.total_out;
     return 0;
 }

bool GZDecompress(const char *zdata, int nzdata, char *data, int *ndata)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] =
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in  = (Bytef*)zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = (Bytef*)data;
    if(inflateInit2(&d_stream, 47) != Z_OK) return false;
    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if(err != Z_OK )
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                {
                    return false;
                }
            }
            else return false;
        }
    }
    if(inflateEnd(&d_stream) != Z_OK) return false;
    *ndata = d_stream.total_out;
    return true;
}

#endif /* HZLIB_CPP_ */
