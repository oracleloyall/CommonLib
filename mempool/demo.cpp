/*
 * create :2018/3
 * author:zhaoxi
 */
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<queue>
#include<pthread.h>
#include"Allocate.h"
#include"pasetime.hpp"
#include"lock.h"
#define TIMES 3000000
struct test_lock_struct {
	slock_t lock;
};
volatile struct test_lock_struct test_lock;
using namespace std;

const int Channels = 4;
const int Ways = 4;
const int Blocks = 4096;
const int PagesPerBlocks = 256;
const int Pages = PagesPerBlocks * Blocks;
const int PageSize = 8192;

typedef struct {
	char buff[200];
} TEST;
typedef struct {
	Int32 fd;
	Int32 state;
	SByte buffer[PageSize];
} WayContext;
typedef struct {
	Int32 fd;
	Int32 state;
	SByte buffer[Blocks];
} ClientContext;

typedef struct {
	WayContext* wayContext;
} ChannelContext;

/*
 * socket id 自增长
 * channelContext 的下标可以是每个socketfd的fd值
 * thread %获取不同id处理
 */

void test() {
	 int i, j;
	 ChannelContext* channelContext;
	 Memory.Initialize((SByte*) malloc(1024 * 1024 * 512), 1024 * 1024 * 512);

	 channelContext = (ChannelContext*) Memory.Allocate(
	 sizeof(ChannelContext) * Channels);
	 for (i = 0; i < Channels; i++) {
	 channelContext[i].wayContext = (WayContext*) Memory.Allocate(
	 sizeof(WayContext) * Ways);
	 for (j = 0; j < Ways; j++) {
	 channelContext[i].wayContext[j].fd = -1;
	 channelContext[i].wayContext[j].state = 0;
	 strncpy(channelContext[i].wayContext[j].buffer, "zhaoxi buffer",
	 14);
	 }
	 }
	 //cout buff data
	 for (i = 0; i < Channels; i++) {
	 for (j = 0; j < Ways; j++) {
	 cout << channelContext[i].wayContext[j].buffer << endl;
	 }
	 }
	 //change buff data
	 for (i = 0; i < Channels; i++) {
	 for (j = 0; j < Ways; j++) {
	 strncpy(channelContext[i].wayContext[j].buffer,
	 "zhaoxi buffer change", 20);
	 }
	 }
	 //cout change data
	 for (i = 0; i < Channels; i++) {
	 for (j = 0; j < Ways; j++) {
	 cout << channelContext[i].wayContext[j].buffer << endl;
	 }
	 }

	 TAllocate<ClientContext> allocate(1024 * 1024 * 512, 100000);
	 int ret = allocate.Allocate();
	 if (ret == 0) {
	 ClientContext *p = allocate.GetContext();
	 for (int i = 0; i < 100; i++) {
	 strncpy(p[i].buffer, "Template data", 22);
	 p[i].fd = 1;
	 p[i].state = 2;
	 }
	 } else {
	 cout << "allocate error" << endl;
	 }
	 //cout element
	 for (int i = 0; i < 102; i++) {
	 ClientContext *p = allocate.GetContext();
	 cout << p[i].buffer << p[i].fd << p[i].state << endl;
	 }
	 unsigned int id;
	ClientContext *ptr = allocate.GetBuffer(id);
	 cout << "id" << id << endl;

}
//malloc free
void test1() {
	CPassedTime passedtime;
	passedtime.Start();
	for (int i = 0; i < TIMES; i++) {
		SByte*ptr = (SByte*) malloc(200);
		memset(ptr, '\0', 200);
		free(ptr);
		ptr = NULL;
	}
	std::cout << "System malloc use time :"
			<< passedtime.End() << std::endl;

}

void test2() {

	TAllocate<TEST> allocate(1024 * 1024 * 100, 30000);
	if (-1 == allocate.Allocate()) {
		cout << "allocate error\n";
		return;
	}
	CPassedTime passedtime;
	passedtime.Start();
	unsigned int id;
	unsigned int i = 0;
	for (id = 0; id < TIMES; id++) {
		S_LOCK(&test_lock.lock);
		TEST *ptr = allocate.GetBuffer(i);
		allocate.ReleaseBuffer(ptr, i);
		S_UNLOCK(&test_lock.lock);
	}
	std::cout << "System allocate use time :" << passedtime.End() << std::endl;
}
void *write_operation(void *) {
	test2();
}
void *read_operation(void *) {
	test1();
}
void test3() {
	TAllocate<TEST> allocate(1024 * 1024 * 100, 30000);
	if (-1 == allocate.Allocate()) {
		cout << "allocate error\n";
		return;
	}
	CPassedTime passedtime;
	passedtime.Start();
	unsigned int id;
	unsigned int i = 0;
	for (id = 0; id < TIMES; id++) {
		//S_LOCK(&test_lock.lock);
		TEST *ptr = allocate.GetElement();
		allocate.ReleaseElement(ptr);
		//S_UNLOCK(&test_lock.lock);
	}
	std::cout << "System list allocate use time :" << passedtime.End()
			<< std::endl;
}
int main() {
//	test();
	test1();
//	test2();

//	pthread_t reader1, reader2, writer1, writer2;
//	pthread_create(&writer1, NULL, read_operation, NULL);
//	pthread_create(&writer2, NULL, read_operation, NULL);
//	pthread_create(&reader1, NULL, read_operation, NULL);
//	pthread_create(&reader2, NULL, read_operation, NULL);
//
//	pthread_join(reader1, NULL);
//	pthread_join(reader2, NULL);
//	pthread_join(writer1, NULL);
//	pthread_join(writer2, NULL);
	test3();
	return 0;
}
/*
 System allocate use time :345423
 System allocate use time :678338
 System allocate use time :1139879
 System allocate use time :1562026


 System malloc use time :615326
 System malloc use time :611438
 System malloc use time :614435
 System malloc use time :620846


 */
