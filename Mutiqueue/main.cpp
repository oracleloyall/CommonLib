#include <thread>
#include "sync_queue.h"

struct SAddContext {
	SAddContext() :
			a(0), b(b) {

	}
	int a;
	int b;
};

struct SAddResult {
	SAddResult() :
			a(0), b(0), result(0) {

	}
	int a;
	int b;
	int result;
};

int main() {
	moon::sync_queue<SAddContext> que1; //main thread - calculate thread
	moon::sync_queue<SAddResult> que2; //calculate thread - print thread

	std::thread calculate([&que1,&que2]() {
		while (1)
		{
			//如果队列为空 ，等待
			if (que1.size() == 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			//获取所有异步计算请求
			auto data = que1.move();
			for (auto& dat : data)
			{
				SAddResult sr;
				sr.a = dat.a;
				sr.b = dat.b;
				sr.result = dat.a + dat.b;
				que2.push_back(sr);
			}
		}
	});

	std::thread printThread([&que2]() {
		while (1)
		{
			//如果队列为空 ，等待
			if (que2.size() == 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			auto data = que2.move();
			for (auto& dat : data)
			{
				printf("%d + %d = %d\r\n", dat.a, dat.b, dat.result);
			}
		}
	});

	int x = 0;
	int y = 0;

	while (std::cin >> x >> y) {
		SAddContext sc;
		sc.a = x;
		sc.b = y;
		que1.push_back(sc);
	}
}
;
