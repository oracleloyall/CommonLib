/*
 *  Project: Read-Write Lock for C
 *  File:   demo.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<iostream>
using namespace std;
#include"AtomicWord.h"
#include"ReadWriteLock.h"
#include"pasetime.hpp"
#include"spinlock.hpp"
#define PG
#include"lock.h"
#define SPINLOCK;
struct spinlock spinlock;
int read_ = 0;
int write_ = 0;
using namespace Ipc;
long long m = 160000;
ReadWriteLock RwLock;
long fibonacci_recursion(int n) {
	if (n <= 2)
		return 1;
	return fibonacci_recursion(n - 1) + fibonacci_recursion(n - 2);
}
#ifdef SPINLOCK
int TOTAL;
#else
Atomic::Word total;
#endif

#ifdef PG
struct test_lock_struct
{
	int total;
	slock_t lock;
};
volatile struct test_lock_struct test_lock;
#else
#endif

void* read_operation(void* vlock) {

	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000000; i++) {

#ifdef PG
		S_LOCK(&test_lock.lock);
		test_lock.total++;
		S_UNLOCK(&test_lock.lock);

#else
#ifdef SPINLOCK
		spinlock_lock(&spinlock);
		TOTAL++;
		spinlock_unlock(&spinlock);
#else

		total++;
#endif
#endif


	}
	std::cout << "Read Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << std::endl;
	return 0;
}

void* write_operation(void* vlock) {

	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000000; i++) {

#ifdef PG
		S_LOCK(&test_lock.lock);
		test_lock.total++;
		S_UNLOCK(&test_lock.lock);
#else
#ifdef SPINLOCK
		spinlock_lock(&spinlock);
		TOTAL++;
		spinlock_unlock(&spinlock);
#else
		total++;
#endif
#endif

	}
	std::cout << "Write Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << std::endl;
	return 0;
}

int main(void)
{

#ifdef PG
	S_INIT_LOCK(&test_lock.lock);
#else
#ifdef SPINLOCK
	spinlock_init(&spinlock);
#else
#endif
#endif
	pthread_t reader1, reader2, writer1;
	pthread_create(&reader1, NULL, read_operation, NULL);
	pthread_create(&reader2, NULL, read_operation, NULL);
	pthread_create(&writer1, NULL, write_operation, NULL);
	pthread_join(reader1, NULL);
	pthread_join(reader2, NULL);
	pthread_join(writer1, NULL);

#ifdef PG

	cout<<"total:<<	test_lock.total<<endl;

#else
#ifdef SPINLOCK
	cout << "total:" << TOTAL << endl;
#else
	cout << "total:" << total << endl;
#endif
#endif
    return 0;
}
/*
 *
 1:互斥锁
 Read Muti thread:140195507459840  use time :374809
 Write Muti thread:140195496969984  use time :374831
 Read Muti thread:140195517949696  use time :395231
 total:3000000

 real    0m0.401s
 user    0m0.312s
 sys     0m0.437s



 2：自实现自旋锁
 time ./a.out
 Read Muti thread:139929701537536  use time :322324
 Read Muti thread:139929712027392  use time :392547
 Write Muti thread:139929691047680  use time :393061
 total:3000000

 real    0m0.403s
 user    0m0.788s
 sys     0m0.006s


 3:原子操作
 time ./a.out
 Read Muti thread:140533642704640  use time :56148
 Write Muti thread:140533632214784  use time :80611
 Read Muti thread:140533653194496  use time :84148
 total:3000000

 real    0m0.089s
 user    0m0.166s
 sys     0m0.006s

 4:PG
 [dhcp3@dhcp89 thre]$ time ./a.out
 Read Muti thread:140332622989056  use time :48857
 Write Muti thread:140332612499200  use time :53693
 Read Muti thread:140332633478912  use time :58210
 total:3000000

 real    0m0.064s
 user    0m0.066s
 sys     0m0.005s
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
