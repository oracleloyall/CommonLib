
#include<iostream>
#include<string>
#include<map>
#include <thread>
#include<vector>
#include "mpmc-bounded-queue.hpp"
using namespace std;
//#define TEST

class MemoryBlock {
public:

	// Simple constructor that initializes the resource.
	explicit MemoryBlock(size_t length = 20) :
			_length(length), _data(new int[length]) {
		std::cout << "In MemoryBlock(size_t). length = " << _length << "."
				<< std::endl;
	}

	// Destructor.
	~MemoryBlock() {
		std::cout << "In ~MemoryBlock(). length = " << _length << ".";

		if (_data != nullptr) {
			std::cout << " Deleting resource.";
			// Delete the resource.
			delete[] _data;
		}

		std::cout << std::endl;
	}

	// Copy constructor.
	MemoryBlock(const MemoryBlock& other) :
			_length(other._length), _data(new int[other._length]) {
		std::cout << "In MemoryBlock(const MemoryBlock&). length = "
				<< other._length << ". Copying resource." << std::endl;

		std::copy(other._data, other._data + _length, _data);
	}

	// Copy assignment operator.
	MemoryBlock& operator=(const MemoryBlock& other) {
		std::cout << "In operator=(const MemoryBlock&). length = "
				<< other._length << ". Copying resource." << std::endl;

		if (this != &other) {
			// Free the existing resource.
			delete[] _data;

			_length = other._length;
			_data = new int[_length];
			std::copy(other._data, other._data + _length, _data);
		}
		return *this;
	}

	// Retrieves the length of the data resource.
	size_t Length() const {
		return _length;
	}

	// Move constructor.
	MemoryBlock(MemoryBlock&& other) :
			_data(nullptr), _length(0) {
		std::cout << "In MemoryBlock(MemoryBlock&&). length = " << other._length
				<< ". Moving resource." << std::endl;

		// Copy the data pointer and its length from the
		// source object.
		_data = other._data;
		_length = other._length;

		// Release the data pointer from the source object so that
		// the destructor does not free the memory multiple times.
		other._data = nullptr;
		other._length = 0;
	}

	// Move assignment operator.
	MemoryBlock& operator=(MemoryBlock&& other) {
		std::cout << "In operator=(MemoryBlock&&). length = " << other._length
				<< "." << std::endl;

		if (this != &other) {
			// Free the existing resource.
			delete[] _data;

			// Copy the data pointer and its length from the
			// source object.
			_data = other._data;
			_length = other._length;

			// Release the data pointer from the source object so that
			// the destructor does not free the memory multiple times.
			other._data = nullptr;
			other._length = 0;
		}
		return *this;
	}

private:
	size_t _length; // The length of the resource.
	int* _data; // The resource.
};

std::vector<string> v;
//string str="zhaoxi";
//string name=std::move(str)
struct async_msg {
	int age;
	MemoryBlock block;
	void GetName() {
		cout << "age:" << age << endl;
	}
	//当对象比较大的场景支持move语义减少拷贝
	//当返回临时对象一般不需要move，编译器本身实现优化，一份对象
	async_msg& operator=(async_msg&& other) {
		if (this != &other)
		{
			age = other.age;
			block = std::move(other.block);
//			string name("zhaoxi");
//			v.push_back(std::move(name));//多线程不安去
			//name = std::move(other.name);多线程下这是不安全的
			std::cout << "In operator=(async_msg&&). length = " << age << "."
					<< std::endl;
		}
		return *this;
	}
//	async_msg(async_msg&& src) noexcept :name(src.name)
//	{
//
//	}
};
class SynsQueue {
public:
	SynsQueue(int num) :
			queue(num) {
	}
	;
	using item_type = async_msg;
	using q_type = mpmc_bounded_queue_t<item_type>;
private:
	q_type queue;
};
#define COUNT 10000
//queue为空情况休眠还是让出
template<typename T>
void bounded_producer_func(T* queue) {
	size_t count = COUNT;
	while (count > 0) {
#ifdef TEST
		string name("zhaoxi");
		v.push_back(std::move(name));
#endif
		async_msg msg;
		msg.age = 24;
		if (queue->enqueue(msg)) {
			--count;
//			cout << "enqueue :" << count << endl;
		}
	}
}
template<typename T>
void consumer_func(T* queue) {
	size_t count = COUNT;
	size_t value = 0;
	async_msg msg;
	msg.age = 24;
	while (count > 0) {
		if (queue->dequeue(msg)) {
#ifdef TEST
			string name("zhaoxi");
			v.push_back(std::move(name));
#endif
			--count;
//			cout << "denqueue :" << count << endl;
		}
	}
}
//std::move(new_msg)
int main(int argc, char **argv) {
	//SynsQueue queue(100000);
	typedef mpmc_bounded_queue_t<async_msg> queue_t;
	queue_t queue(65536);
	std::thread producer(std::bind(&bounded_producer_func<queue_t>, &queue));
	std::thread producer1(std::bind(&bounded_producer_func<queue_t>, &queue));
	std::thread producer2(std::bind(&bounded_producer_func<queue_t>, &queue));
	std::thread producer3(std::bind(&bounded_producer_func<queue_t>, &queue));
	std::thread consumer(std::bind(&consumer_func<queue_t>, &queue));
	std::thread consumer1(std::bind(&consumer_func<queue_t>, &queue));
	std::thread consumer2(std::bind(&consumer_func<queue_t>, &queue));
	std::thread consumer3(std::bind(&consumer_func<queue_t>, &queue));
	producer.join();
	producer1.join();
	producer2.join();
	producer3.join();

	consumer.join();
	consumer1.join();
	consumer2.join();
	consumer3.join();
	return 0;
}
