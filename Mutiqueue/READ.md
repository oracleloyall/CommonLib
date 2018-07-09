Ë«ºËÐÄ
g++ -std=c++11 queue.cpp  -lpthread

SPSC bound queue completed 100000000 iterations in 18.3396 seconds. 5.45269 million enqueue/dequeue pairs per second.
MPMC bound queue completed 100000000 iterations in 9.61739 seconds. 10.3978 million enqueue/dequeue pairs per second.
SPMC dynamic queue completed 100000000 iterations in 34.4643 seconds. 2.90155 million enqueue/dequeue pairs per second.
MPSC dynamic queue completed 100000000 iterations in 45.2428 seconds. 2.2103 million enqueue/dequeue pairs per second.