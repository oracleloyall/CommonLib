/*
 * compare.cpp
 *
 *  Created on: 2018年2月8日
 *      Author: zhaoxi
 */

#include<pthread.h>
#include<iostream>
using namespace std;
#include"pasetime.hpp"
#include"spinlock.hpp"
#include"lock.h"
long long m = 0;
struct spinlock spinlock;
struct test_lock_struct {
	int pad1;
	slock_t lock;
	int pad2;
};

volatile struct test_lock_struct test_lock;
long fibonacci_recursion(int n) {
	if (n <= 2)
		return 1;
	return fibonacci_recursion(n - 1) + fibonacci_recursion(n - 2);
}
void * thread_proxy_func(void * args) {
//	Thread * pThread = static_cast<Thread *>(args);
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < 1000; i++) {
		S_LOCK(&test_lock.lock);
		//	spinlock_lock(&spinlock);
		fibonacci_recursion(30);
		//	++m;
//		spinlock_unlock(&spinlock);
		S_UNLOCK(&test_lock.lock);

	}
	std::cout << "Muti thread:" << pthread_self() << "  use time :"
			<< passedtime.End() << std::endl;
	return NULL;
}
void test_time() {
	CPassedTime passedtime;
	passedtime.Start();
	{

		CPassedTime passedtime;
	passedtime.Start();
		passedtime.End();
		pthread_self();
		pthread_self();
		usleep(500);
		std::cout << "Muti thread1:" << pthread_self() << "  use time :"
			<< passedtime.End() << std::endl;
	}
	std::cout << "Muti thread2:" << pthread_self() << "  use time :"
			<< passedtime.End() << std::endl;
}
/*
 * spin lock
 * [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140151659734784  use time :4462061
 Muti thread:140151638755072  use time :4482963
 Muti thread:140151649244928  use time :5495058
 Muti thread:140151670224640  use time :5822883
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:139797188052736  use time :1742532
 Muti thread:139797198542592  use time :3526542
 Muti thread:139797219522304  use time :5360752
 Muti thread:139797209032448  use time :5357425
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:139657389762304  use time :3726204
 Muti thread:139657421231872  use time :5384990
 Muti thread:139657400252160  use time :5454165
 Muti thread:139657410742016  use time :6320504


 mutex
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:139779561072384  use time :3488092
 Muti thread:139779529602816  use time :3526011
 Muti thread:139779540092672  use time :3581177
 Muti thread:139779550582528  use time :3702425
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140186509977344  use time :3168122
 Muti thread:140186520467200  use time :3420308
 Muti thread:140186530957056  use time :3602817
 Muti thread:140186499487488  use time :3696331
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140252057016064  use time :3240991
 Muti thread:140252046526208  use time :3387753
 Muti thread:140252067505920  use time :3451474
 Muti thread:140252077995776  use time :3715968

 mutex

 Muti thread:139957788182272  use time :948
 Muti thread:139957809161984  use time :1409
 Muti thread:139957777692416  use time :1813
 Muti thread:139957798672128  use time :2129
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:139702056941312  use time :1521
 Muti thread:139702088410880  use time :1516
 Muti thread:139702067431168  use time :1691
 Muti thread:139702077921024  use time :2731
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140065061033728  use time :974
 Muti thread:140065050543872  use time :2520
 Muti thread:140065082013440  use time :2694
 Muti thread:140065071523584  use time :2741
 [dhcp3@dhcp89 thre]$

 spinlock
 Muti thread:139998884202240  use time :659
 Muti thread:139998915671808  use time :641
 Muti thread:139998905181952  use time :221
 Muti thread:139998894692096  use time :207
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140699420538624  use time :548
 Muti thread:140699431028480  use time :1396
 Muti thread:140699441518336  use time :992
 Muti thread:140699452008192  use time :206
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140087652013824  use time :1163
 Muti thread:140087683483392  use time :1234
 Muti thread:140087672993536  use time :353
 Muti thread:140087662503680  use time :386

 Muti thread:140610814109440  use time :881430
 Muti thread:140610845579008  use time :1893201
 Muti thread:140610835089152  use time :2988005
 Muti thread:140610824599296  use time :3951592
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140583657723648  use time :874767
 Muti thread:140583678703360  use time :1849456
 Muti thread:140583668213504  use time :2876847
 Muti thread:140583689193216  use time :3783318
 [dhcp3@dhcp89 thre]$ ./a.out
 Muti thread:140688447817472  use time :876613
 Muti thread:140688458307328  use time :2124160
 Muti thread:140688479287040  use time :3007946
 Muti thread:140688468797184  use time :4590603


 Muti thread:140224327898880  use time :1762235
 Muti thread:140224338388736  use time :3987576


 Muti thread:140548239054592  use time :3535485
 */
/*临界区无操作
 * 一个线程 2000 次 30下无锁 耗时Muti thread:140615502128896  use time :21851003
 * 两个线程每个1000下 30 有锁耗时Muti thread:140525883979520  use time :10838283
 Muti thread:140525894469376  use time :11520033
 * 第一种：
 * 第二种：互斥锁Muti thread:139719338403584  use time :10853418
 Muti thread:139719348893440  use time :11575063
 * 第三种：Muti thread:139943347443456  use time :10711449
 Muti thread:139943357933312  use time :11674901
 *
 *
 *临界区有操作 简单++
 *临界区有操作 1：Muti thread:140385518851840  use time :10835287
 Muti thread:140385529341696  use time :11531001
 *临界区有操作 2：./a.out
 Muti thread:140134074255104  use time :10755161
 Muti thread:140134084744960  use time :11732622
 *临界区有操作 3：Muti thread:139809252873984  use time :10725709
 Muti thread:139809263363840  use time :11709626



 临界区操作较为复杂：
 1：Muti thread:139981419656960  use time :10870151
 Muti thread:139981430146816  use time :11544469
 2：Muti thread:139866178533120  use time :10744818
 Muti thread:139866189022976  use time :11696265
 3：Muti thread:140546929764096  use time :10816644
 Muti thread:140546940253952  use time :11640481

 * 四个线程
 */
int main(int argc, char **argv) {
//	spinlock_init(&spinlock);
	test_time();
	S_INIT_LOCK(&test_lock.lock);
	//test for mutex && spinlock
	pthread_t tid, tid1, tid2, tid3;
	pthread_create(&tid, NULL, thread_proxy_func, NULL);
	pthread_create(&tid1, NULL, thread_proxy_func, NULL);
//	pthread_create(&tid2, NULL, thread_proxy_func, NULL);
//	pthread_create(&tid3, NULL, thread_proxy_func, NULL);

	pthread_join(tid, NULL);
	pthread_join(tid1, NULL);
//	pthread_join(tid2, NULL);
//	pthread_join(tid3, NULL);
	return 0;
}

