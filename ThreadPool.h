#pragma once
#include "MultiQueue.h"
#include <iostream>
#include <thread>

class ThreadPool {
private:
	MultiQueue* multiqueue;
	int size = 0;
	std::vector<std::thread> threads;

	void ExecuteTask(int threadID) {
		while (true) {
			MessageBase* msg = multiqueue->dequeue(threadID);

			if (msg->shouldTerminate()) {
				multiqueue->boardcast(msg);
				return;
			}

			//execute msg
			msg->execute();
		}
	}

public:
	ThreadPool(MultiQueue* queues) {
		multiqueue = queues;
		size = queues->getSize();
		for (int i = 0; i < size; i++) {
			threads.emplace_back(std::thread(&ThreadPool::ExecuteTask, this, i));
		}
	}

	void join() {
		std::for_each(threads.begin(), threads.end(), [](std::thread& t) -> void { t.join(); });
	}
};