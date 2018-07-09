#ifndef ALLOCATE_H_H
#define  ALLOCATE_H_H
/*
 * ģ�廯
 */
#include "Common.h"
#include <stdlib.h>
#include<string.h>
#include<queue>
#include <algorithm>
using namespace std;
//�ڴ���Դ;�����֣���дRelease��ʵ�־��������
//���� small middle large ���������ͷ���
//��ͬ���͵Ĺ���
//�ṩ��ȡ���ͷŽӿ�
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

//λͼ��˼�룺��ȡλͼλ����0,id++���ͷ�λͼλ��++
class BitMap {
public:
	BitMap(UInt32 range) {
		//��ʱ�࿪��һ���ռ�
		_bits.resize(range / 32 + 1);
	}
	inline void Set(UInt32 x) {
		int index = x / 32; //ȷ���ĸ����ݣ����䣩
		int temp = x % 32; //ȷ���ĸ�Bitλ
		_bits[index] |= (1 << temp); //λ��������
	}
	inline void Reset(UInt32 x) {
		int index = x / 32;
		int temp = x % 32;
		_bits[index] &= ~(1 << temp); //ȡ��
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
		//���������ݰ汾
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
	//�ҵ��յ�vectorλ��,Ȼ�󷵻�Ԫ��λ�ã�ɾ����Ԫ�ػ���ͨ����־λ�����sizeof(T)��С�Ŀռ䣬
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
			id = Element.front(); //���ض�ͷԪ��
			Element.pop(); //ɾ����ͷԪ��
			return (&Context[id]);
		} else
			return 0;
	}
	;
	inline void ReleaseBuffer(T*ptr, UInt32 id) {
		Element.push(id); //��β����Ԫ��
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
