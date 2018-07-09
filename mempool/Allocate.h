#ifndef ALLOCATE_H_H
#define  ALLOCATE_H_H
/*
 * 模板化
 */
#include "Common.h"
#include <stdlib.h>
#include<string.h>
#include<queue>
#include <algorithm>
using namespace std;
//内存来源途径多种，重写Release，实现具体的子类
//管理 small middle large 的三中类型分配
//不同类型的管理
//提供获取，释放接口
class IncrementalAllocator {
public:
	void Initialize(SByte* pool, UInt32 poolSize);

	SByte* Allocate(UInt32 size);
	IncrementalAllocator() {
	}
	;
	~IncrementalAllocator() {
		Release();
	}
	;
	virtual void Release() {
		free(pool);
	}
	;
	inline UInt32 GetPoolSize() {
		return poolSize;
	};
private:
	SByte* pool;
	UInt32 poolSize;
	UInt32 allocationCursor;
};

//位图的思想：获取位图位置置0,id++，释放位图位置++
class BitMap {
public:
	BitMap(UInt32 range) {
		//此时多开辟一个空间
		_bits.resize(range / 32 + 1);
	}
	inline void Set(UInt32 x) {
		int index = x / 32; //确定哪个数据（区间）
		int temp = x % 32; //确定哪个Bit位
		_bits[index] |= (1 << temp); //位操作即可
	}
	inline void Reset(UInt32 x) {
		int index = x / 32;
		int temp = x % 32;
		_bits[index] &= ~(1 << temp); //取反
	}
	bool Test(UInt32 x) {
		int index = x / 32;
		int temp = x % 32;
		if (_bits[index] & (1 << temp))
			return 1;
		else
			return 0;
	}

private:
	vector<int> _bits;
};
template<class T>
class TAllocate {
public:
	typedef T value_type;
	typedef T* pointer;
	TAllocate(UInt32 size, UInt32 AllocSize) :
			Size(size), ASize(AllocSize), UsedId(0) {
		currentBlock_ = 0;
		currentSlot_ = 0;
		lastSlot_ = 0;
		freeSlots_ = 0;
		Memory.Initialize((SByte*) malloc(Size), Size);
	}
	;
	~TAllocate() {

	}
	;
	bool Allocate(void) {
		//不允许扩容版本
		if (Memory.GetPoolSize() >= (sizeof(T) * ASize)) {
		Context = (T*) Memory.Allocate(sizeof(T) * ASize);
			reinterpret_cast<slot_pointer_>(Context)->next = currentBlock_;
			currentBlock_ = reinterpret_cast<slot_pointer_>(Context);
			freeSlots_ = currentBlock_;
			if (!Context) {
				return -1;
			}
			for (int i = 0; i <= ASize; i++) {
				Element.push(i);
			}
			return 0;
		} else
			return -1;
	}
	;
	inline T* GetContext() {
		return Context;
	}
	;
	//找到空的vector位置,然后返回元素位置，删掉该元素还是通过标志位，清空sizeof(T)大小的空间，
	inline T*GetElement() {
		if (freeSlots_ != 0) {
			pointer result = reinterpret_cast<pointer>(freeSlots_);
			freeSlots_ = (freeSlots_++)->next;
			return result;
		} else
			return 0;
	}
	inline T*ReleaseElement(T*p) {
		if (p != 0) {
			reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
			freeSlots_ = reinterpret_cast<slot_pointer_>(p);
		}
	}
	inline T*GetBuffer(UInt32 &id) {
//		if (!Element.empty()) {
//			if (UsedId < ASize) {
//				id = UsedId++;
//			return (&Context[id]);
//			} else
//				return 0;
//		} else
//			return 0;
		if (!Element.empty()) {
			id = Element.front(); //返回队头元素
			Element.pop(); //删除队头元素
			return (&Context[id]);
		} else
			return 0;
	}
	;
	inline void ReleaseBuffer(T*ptr, UInt32 id) {
		Element.push(id); //队尾插入元素
		memset(ptr, '\0', sizeof(T));
	}
	;
private:
	UInt32 Size;
	UInt32 ASize;
	UInt32 UsedId;

	union Slot_ {
		value_type element;
		Slot_* next;
	};

	typedef char* data_pointer_;
	typedef Slot_ slot_type_;
	typedef Slot_* slot_pointer_;

	slot_pointer_ currentBlock_;
	slot_pointer_ currentSlot_;
	slot_pointer_ lastSlot_;
	slot_pointer_ freeSlots_;
	T *Context;
	queue<UInt32> Element;
	IncrementalAllocator Memory;
};
extern IncrementalAllocator Memory;

#endif
