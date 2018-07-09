/*
 *  Project: Read-Write Lock for C
 *  File:   demo.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<iostream>
using namespace std;
#include "rwlock.h"
#include"rwlock.hpp"
#include"pasetime.hpp"
#include"spinlock.hpp"
#include"lock.h"
struct rwlock rwlock;
struct test_lock_struct {
	slock_t lock;
};
spinlock spinlock;
volatile struct test_lock_struct test_lock;
long long m = 0;
long fibonacci_recursion(int n) {
	if (n <= 2)
		return 1;
	return fibonacci_recursion(n - 1) + fibonacci_recursion(n - 2);
}
// 1 PG;2 mutex;
#define TYPE 1
#if 1==TYPE
void* read_operation(void* vlock)
{

	ReadWriteLock lock = static_cast<ReadWriteLock>(vlock);
	pthread_t id = pthread_self();
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000000; i++) {
		//rwl_readLock(lock);
		//rwlock_rlock (&rwlock);
		S_LOCK(&test_lock.lock);
		++m;
		//	cout << "read m :" << m << endl;
		//	fibonacci_recursion(30);
		//rwlock_runlock(&rwlock);
		S_UNLOCK(&test_lock.lock);
		//fibonacci_recursion(25);
		//rwl_readUnlock(lock);
	}
	std::cout << "Read Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << " data:" << m << std::endl;
    return 0;
}

void* write_operation(void* vlock)
{
	ReadWriteLock lock = static_cast<ReadWriteLock>(vlock);
	pthread_t id = pthread_self();
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000000; i++) {
//	rwl_writeLock(lock);
		//	rwlock_wlock (&rwlock);
		S_LOCK(&test_lock.lock);
		++m; //1600
		//	cout << "write m :" << m << endl;
		//	fibonacci_recursion(30);
		//rwlock_wunlock(&rwlock);
		S_UNLOCK(&test_lock.lock);
		//	fibonacci_recursion(25);
//	rwl_writeUnlock(lock);
	}
	std::cout << "Write Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << " data:" << m << std::endl;
    return 0;
}
#elif 2==TYPE

void* read_operation(void* vlock) {

	ReadWriteLock lock = static_cast<ReadWriteLock>(vlock);
	pthread_t id = pthread_self();
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000000; i++) {
		//rwl_readLock(lock);
		//rwlock_rlock (&rwlock);
		spinlock_lock(&spinlock);
		++m;
		//	cout << "read m :" << m << endl;
		//	fibonacci_recursion(30);
		//rwlock_runlock(&rwlock);
		spinlock_unlock(&spinlock);
		//	fibonacci_recursion(25);
		//rwl_readUnlock(lock);
	}
	std::cout << "Read Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << " data:" << m << std::endl;
	return 0;
}

void* write_operation(void* vlock) {
	ReadWriteLock lock = static_cast<ReadWriteLock>(vlock);
	pthread_t id = pthread_self();
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000000; i++) {
//	rwl_writeLock(lock);
		//	rwlock_wlock (&rwlock);
		spinlock_lock(&spinlock);
		++m; //1600
		//	cout << "write m :" << m << endl;
		//	fibonacci_recursion(30);
		//rwlock_wunlock(&rwlock);
		spinlock_unlock(&spinlock);
		//	fibonacci_recursion(25);
//	rwl_writeUnlock(lock);
	}
	std::cout << "Write Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << " data:" << m << std::endl;
	return 0;
}
#else
#endif


//8 核心比较
int main(void)
{
	pthread_t reader1, reader2, writer1, writer2;
	pthread_t reader3, reader4, writer3, writer4;
    ReadWriteLock lock;
	spinlock_init(&spinlock);
	S_INIT_LOCK(&test_lock.lock);
	rwlock_init(&rwlock);
    if(rwl_init(&lock) == -1)                    //initialization of the lock
        return -1;
    printf("lock initialized.\n");
	pthread_create(&writer1, NULL, write_operation, lock);
	pthread_create(&writer2, NULL, write_operation, lock);
    pthread_create(&reader1, NULL, read_operation, lock);
    pthread_create(&reader2, NULL, read_operation, lock);

	pthread_create(&writer3, NULL, write_operation, lock);
	pthread_create(&writer4, NULL, write_operation, lock);
	pthread_create(&reader3, NULL, read_operation, lock);
	pthread_create(&reader4, NULL, read_operation, lock);

    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);
    pthread_join(writer1, NULL);
	pthread_join(reader3, NULL);
	pthread_join(reader4, NULL);
	pthread_join(writer2, NULL);
	pthread_join(writer3, NULL);
	pthread_join(writer4, NULL);
	cout << "m :" << m << endl;
    return 0;
}
/*
 *
 * real    0m13.494s
 user    0m22.449s
 sys     0m0.008s
 [dhcp3@dhcp89 thre]$ time ./a.out
 lock initialized.
 Read Muti thread:139777479374592  use time :9004702
 Read Muti thread:139777468884736  use time :9026119
 Write Muti thread:139777489864448  use time :13503516
 m :0

 real    0m13.510s
 user    0m22.479s
 sys     0m0.003s



 time ./a.out
 lock initialized.
 Write Muti thread:139760109197056  use time :6188423
 Read Muti thread:139760088217344  use time :14981761
 Read Muti thread:139760098707200  use time :16832620
 m :0

 real    0m16.846s
 user    0m31.767s
 sys     0m0.009s


 time ./a.out
 lock initialized.
 Read Muti thread:140003532015360  use time :8634289
 Read Muti thread:140003542505216  use time :8936850
 Write Muti thread:140003552995072  use time :13249111
 m :0

 real    0m13.256s
 user    0m21.853s
 sys     0m0.008s
 *
 *
 *
 *无锁
 *无锁./a.out
 lock initialized.
 Write Muti thread:140013453059840  use time :6832403
 Read Muti thread:140013432080128  use time :10010310
 Read Muti thread:140013442569984  use time :11665879


 lock initialized.
 Write Muti thread:140170332481280  use time :21562773
 m :0

 real    0m21.570s
 user    0m21.545s
 sys     0m0.010s
 *
 *
 *real时间是指挂钟时间，也就是命令开始执行到结束的时间。这个短时间包括其他进程所占用的时间片，和进程被阻塞时所花费的时间。
 user时间是指进程花费在用户模式中的CPU时间，这是唯一真正用于执行进程所花费的时间，其他进程和花费阻塞状态中的时间没有计算在内。
 sys时间是指花费在内核模式中的CPU时间，代表在内核中执系统调用所花费的时间，这也是真正由进程使用的CPU时间。


 Real指的是实际经过的时间，User和Sys指的是该进程使用的CPU时间。

 1. Real是墙上时间(wall clock time)，也就是进程从开始到结束所用的实际时间。这个时间包括其他进程使用的时间片和进程阻塞的时间（比如等待I/O完成）。

 2. User指进程执行用户态代码（核心之外）所使用的时间。这是执行此进程所消耗的实际CPU时间，其他进程和此进程阻塞的时间并不包括在内。

 3. Sys指进程在内核态消耗的CPU时间，即在内核执行系统调用所使用的CPU时间。

 那么，为什么进程开始到结束所经过的时间会比进程所消耗的用户时间和系统时间(user time + sys time)小呢？

 User+Sys为进程所使用的实际CPU时间。注意，如果有多个线程，User+Sys的时间有可能大于Real时间。同时，User和Sys时间包括子进程所使用的时间。

 time命令的输出数据是由几个不同的系统调用得来的。User time和Sys time从wait(2)或times(2)系统调用（依赖不同的系统）得来。Real time是由gettimeofday(2)中结束时间和起始时间相减得到。不同的操作系统还可能有其他的信息，比如time可以记录上下文切换的次数。

 在多处理器的系统上，一个进程如果有多个线程或者有多个子进程可能导致Real time比CPU time（User + Sys time）要小，这是因为不同的线程或进程可以并行执行。

 */

/*
 * 下面8核心数据比较
 *
 */

/*
 * time ./pg
 lock initialized.
 Write Muti thread:140335045146368  use time :25641419 data:775516076
 Write Muti thread:140335053539072  use time :25663711 data:776280718
 Write Muti thread:140335078717184  use time :25815293 data:781053005
 Read Muti thread:140335061931776  use time :25945331 data:787124269
 Read Muti thread:140335028360960  use time :25962905 data:788022784
 Read Muti thread:140335036753664  use time :26109298 data:795694696
 Write Muti thread:140335087109888  use time :26179080 data:799873184
 Read Muti thread:140335070324480  use time :26181641 data:800000000
 m :800000000

 real    0m26.185s
 user    1m7.044s
 sys     0m0.311s
 [dhcp3@10-13DHCP-SERVER sync]$ g++ demo.cpp rwlock.c lock.cpp  -lpthread
 [dhcp3@10-13DHCP-SERVER sync]$ time ./a.out
 lock initialized.
 Write Muti thread:139785184081664  use time :120444926 data:776943950
 Write Muti thread:139785175688960  use time :121913175 data:786440887
 Write Muti thread:139785217652480  use time :122413142 data:789680737
 Write Muti thread:139785209259776  use time :123010207 data:793509303
 Read Muti thread:139785167296256  use time :123811306 data:799224292
 Read Muti thread:139785192474368  use time :123813955 data:799240446
 Read Muti thread:139785200867072  use time :123827619 data:799434094
 Read Muti thread:139785158903552  use time :123846420 data:800000000
 m :800000000

 real    2m3.850s
 user    2m42.202s
 sys     5m31.382s
 */
