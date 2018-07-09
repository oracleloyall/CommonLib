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
 1:������
 Read Muti thread:140195507459840  use time :374809
 Write Muti thread:140195496969984  use time :374831
 Read Muti thread:140195517949696  use time :395231
 total:3000000

 real    0m0.401s
 user    0m0.312s
 sys     0m0.437s



 2����ʵ��������
 time ./a.out
 Read Muti thread:139929701537536  use time :322324
 Read Muti thread:139929712027392  use time :392547
 Write Muti thread:139929691047680  use time :393061
 total:3000000

 real    0m0.403s
 user    0m0.788s
 sys     0m0.006s


 3:ԭ�Ӳ���
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
 *realʱ����ָ����ʱ�䣬Ҳ�������ʼִ�е�������ʱ�䡣�����ʱ���������������ռ�õ�ʱ��Ƭ���ͽ��̱�����ʱ�����ѵ�ʱ�䡣
 userʱ����ָ���̻������û�ģʽ�е�CPUʱ�䣬����Ψһ��������ִ�н��������ѵ�ʱ�䣬�������̺ͻ�������״̬�е�ʱ��û�м������ڡ�
 sysʱ����ָ�������ں�ģʽ�е�CPUʱ�䣬�������ں���ִϵͳ���������ѵ�ʱ�䣬��Ҳ�������ɽ���ʹ�õ�CPUʱ�䡣


 Realָ����ʵ�ʾ�����ʱ�䣬User��Sysָ���Ǹý���ʹ�õ�CPUʱ�䡣

 1. Real��ǽ��ʱ��(wall clock time)��Ҳ���ǽ��̴ӿ�ʼ���������õ�ʵ��ʱ�䡣���ʱ�������������ʹ�õ�ʱ��Ƭ�ͽ���������ʱ�䣨����ȴ�I/O��ɣ���

 2. Userָ����ִ���û�̬���루����֮�⣩��ʹ�õ�ʱ�䡣����ִ�д˽��������ĵ�ʵ��CPUʱ�䣬�������̺ʹ˽���������ʱ�䲢���������ڡ�

 3. Sysָ�������ں�̬���ĵ�CPUʱ�䣬�����ں�ִ��ϵͳ������ʹ�õ�CPUʱ�䡣

 ��ô��Ϊʲô���̿�ʼ��������������ʱ���Ƚ��������ĵ��û�ʱ���ϵͳʱ��(user time + sys time)С�أ�

 User+SysΪ������ʹ�õ�ʵ��CPUʱ�䡣ע�⣬����ж���̣߳�User+Sys��ʱ���п��ܴ���Realʱ�䡣ͬʱ��User��Sysʱ������ӽ�����ʹ�õ�ʱ�䡣

 time���������������ɼ�����ͬ��ϵͳ���õ����ġ�User time��Sys time��wait(2)��times(2)ϵͳ���ã�������ͬ��ϵͳ��������Real time����gettimeofday(2)�н���ʱ�����ʼʱ������õ�����ͬ�Ĳ���ϵͳ����������������Ϣ������time���Լ�¼�������л��Ĵ�����

 �ڶദ������ϵͳ�ϣ�һ����������ж���̻߳����ж���ӽ��̿��ܵ���Real time��CPU time��User + Sys time��ҪС��������Ϊ��ͬ���̻߳���̿��Բ���ִ�С�

 */
