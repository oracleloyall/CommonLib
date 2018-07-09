/*
 * actomic.hpp
 *
 *  Created on: 2018年1月9日
 *      Author: zhaoxi
 *
 */
/*
 *
 */
#ifndef LOCKFREEBUFFER_ACTOMIC_HPP_
#define LOCKFREEBUFFER_ACTOMIC_HPP_
//for compare （原子操作 比较和交换）
#define ATOM_CAS(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define ATOM_CAS_POINTER(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
//fect_and_sub 先fetch，然后自加，返回的是自加以前的值
#define ATOM_INC(ptr) __sync_add_and_fetch(ptr, 1)
#define ATOM_FINC(ptr) __sync_fetch_and_add(ptr, 1)
#define ATOM_DEC(ptr) __sync_sub_and_fetch(ptr, 1)
#define ATOM_FDEC(ptr) __sync_fetch_and_sub(ptr, 1)
#define ATOM_ADD(ptr,n) __sync_add_and_fetch(ptr, n)
#define ATOM_SUB(ptr,n) __sync_sub_and_fetch(ptr, n)
#define ATOM_AND(ptr,n) __sync_and_and_fetch(ptr, n)




#endif /* LOCKFREEBUFFER_ACTOMIC_HPP_ */
