#include <iostream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include "MemoryPool.hpp"
#include"pasetime.hpp"
using namespace std;
#include"lock.h"
slock_t lock;
#define TIMES 10000
void *test1(void *p) {
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < TIMES; i++) {
		char *ptr = (char *) malloc(4);
		memset(ptr, '\0', 4);
		free(ptr);
		ptr = NULL;
	}
	std::cout << "System malloc use time :" << passedtime.End() << std::endl;
	return 0;
}
struct Test {
	char buff[4];
};
MemoryPool<Test> pool;
void *test2(void *p) {
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < TIMES; i++) {
		S_LOCK(&lock);
		Test* x = pool.allocate();
		cout << "ID:" << pthread_self() << "Adress：" << x << endl;
		pool.deallocate(x);
		S_UNLOCK(&lock);
		memset(x->buff, '\0', 4);
	}
	std::cout << "pool use time :" << passedtime.End() << std::endl;
	return 0;
}

int main() {
	pthread_t reader1, reader2, writer1;
	pthread_create(&writer1, NULL, test2, NULL);
	pthread_create(&reader1, NULL, test2, NULL);
	pthread_create(&reader2, NULL, test2, NULL);

	pthread_join(reader1, NULL);
	pthread_join(reader2, NULL);
	pthread_join(writer1, NULL);

	return 0;
}
/*
 * 非线程安全
 * pool use time :2063743
 pool use time :3074079
 pool use time :3112914

 System malloc use time :15172818
 System malloc use time :16111989
 System malloc use time :16144188


 System malloc use time :10484317

 pool use time :2074194

 线程安全 10000 00001亿次

 pool use time :7484443
 pool use time :7761399
 pool use time :7853786

 System malloc use time :12130621
 System malloc use time :12721435
 System malloc use time :12721896
 */
