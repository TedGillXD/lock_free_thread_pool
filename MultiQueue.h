#pragma once
#include <queue>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

class MessageBase {
public:
	virtual bool shouldTerminate() {
		return true;
	}

	virtual void execute() {
		std::printf("done work.\n");
	}
};

class MultiQueue {
public:
	friend class DistributorBase;
	class DistributorBase {
	public:
		static DistributorBase* getDefulatMonitor() { static DistributorBase monitor; return &monitor; }

		// derive from Distributor class and override this function to customize distribute rules
		virtual const size_t getIndex(MultiQueue* q) {
			return std::min_element(q->queues.begin(), q->queues.end(), [](auto& q1, auto& q2) { return q1.size() < q2.size(); }) - q->queues.begin();
		}
	};

private:
	int size = 0;		//the number of threads.
	int limit = 10;		//this limitation is soft limitation, the actual size of queue could exceed 1 or 2 to this value.
	std::vector<std::queue<MessageBase*>> queues;

	DistributorBase* monitor;

public:
	inline const int getSize() { return size; }
	inline const int getLimit() { return limit; }

public:
	//default constructor
	MultiQueue() : MultiQueue(DistributorBase::getDefulatMonitor(), 3, 10) { }

	MultiQueue(int size, int limit) : MultiQueue(DistributorBase::getDefulatMonitor(), size, limit) { }

	MultiQueue(DistributorBase* monitor, int size, int limit) {
		queues = std::vector<std::queue<MessageBase*>>(size);
		this->size = size;
		this->limit = limit;
		this->monitor = monitor;
	}

	void boardcast(MessageBase* msg) {
		std::for_each(queues.begin(), queues.end(), [&](std::queue<MessageBase*>& q) -> void { q.emplace(msg); });
	}

	void enqueue(MessageBase* msg) {
		size_t index = monitor->getIndex(this);
		while (queues[index].size() >= limit) {
			std::this_thread::yield(); //wait if the queue is full
		}
		queues[index].emplace(msg);
	}

	MessageBase* dequeue(int index) {
		while (queues[index].empty()) {
			std::this_thread::yield(); //wait if the queue is empty
		}

		MessageBase* ret = queues[index].front();
		queues[index].pop();
		return ret;
	}

	void clear() {
		std::for_each(queues.begin(), queues.end(), [](std::queue<MessageBase*>& q) -> void { std::queue<MessageBase*> empty; std::swap(q, empty); });
	}
};
