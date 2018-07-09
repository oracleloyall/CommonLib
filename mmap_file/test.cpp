/*
 * test.cpp
 *
 *  Created on: 2018Äê1ÔÂ10ÈÕ
 *      Author: zhaoxi
 */
#include <stdio.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <memory>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include"mmap_file_pool.h"
using namespace std;
using namespace mmap_allocator_namespace;

#define TESTFILE "testfile"
#define TESTFILE2 "testfile2"

void generate_test_file(int count, const char *fname)
{
	FILE *f;
	int i;

	f = fopen(fname, "w+");
	for (i=0;i<count;i++) {
		fwrite(&i, 1, sizeof(i), f);
	}
	fclose(f);
}

void test_mmap_file_pool(void)
{
	generate_test_file(1024, TESTFILE);
	int *f = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 1024, false, false);
	int *f2 = (int*)the_pool.mmap_file(string(TESTFILE), READ_ONLY, 0, 1024, false, false);
	int i;

	assert(f == f2);

	for (i=0;i<1024;i++) {
		assert(f[i] == i);
	}
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 1024);
	the_pool.munmap_file(string(TESTFILE), READ_ONLY, 0, 1024);
}

int main(int argc, char ** argv)
{
	set_verbosity(1);

	test_mmap_file_pool();

}


