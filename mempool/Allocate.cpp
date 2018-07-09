#include "Allocate.h"

IncrementalAllocator Memory;
void IncrementalAllocator::Initialize(SByte* pool, UInt32 poolSize) {
	memset(pool, '\0', poolSize);
	this->pool = pool;
	this->poolSize = poolSize;
	this->allocationCursor = 0;
}
/*
 * if 0 if full
 */
SByte* IncrementalAllocator::Allocate(UInt32 size) {
	SByte* allocation = 0;
	if (allocationCursor + size <= poolSize) {
		allocation = (pool + allocationCursor);
		allocationCursor += size;
		return allocation;
	} else {
		SByte *pMemBackup = pool; /* �� realloc ֮ǰ���ڴ��ַ����һ�� */
		pool = (SByte *) realloc(pool, poolSize);

		if (pool) { /* realloc �ɹ� */
			pMemBackup = NULL;
			poolSize *= 2;
			if (allocationCursor + size <= poolSize) {
				allocation = (pool + allocationCursor);
				allocationCursor += size;
				return allocation;
			} else
				return 0;
		} else { /* realloc ʧ���ˣ����ʱ�򷵻ص��� NULL */
			pool = pMemBackup;
			return 0;
		}
		return 0;
	}
}
