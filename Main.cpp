// Test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include "ThreadPool.h"
using namespace std;

enum MessageType : uint8_t {
	PRINT,
	TERMINATE
};

class Message : public MessageBase {
	MessageType type;
	std::string msg;

public:
	Message(int t, std::string str) {
		type = t == 1 ? PRINT : TERMINATE;
		msg = str;
	}

	bool shouldTerminate() override {
		return type == TERMINATE;
	}

	void execute() override {
		cout << msg << endl;
	}
};

class CircularDistributor : public MultiQueue::DistributorBase {
private:
	int currentID = 0;

public:
	CircularDistributor() { currentID = 0; }

	const size_t getIndex(MultiQueue* q) override {
		int ret = currentID++;
		currentID %= q->getSize();
		return ret;
	}
};

void testMultiQueue() {
	//read instructions from file
	std::cout << "start read instructions from file\n";
	std::ifstream file("data.txt");
	std::string str;
	int ins;
	std::vector<Message> arr;
	while (file >> ins >> str) {
		arr.emplace_back(Message(ins, str));
	}

	std::cout << "start enqueue instructions.\n";

	CircularDistributor* monitor = new CircularDistributor();
	MultiQueue* queues = new MultiQueue(monitor, 2, 100);
	auto start = std::chrono::steady_clock::now();
	ThreadPool pool(queues);

	for (auto& i : arr) {
		queues->enqueue(&i);
	}

	pool.join();
	auto end = std::chrono::steady_clock::now();

	double time = (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000 / 1000;
	std::cout << time << std::endl;
}

int main() {

	testMultiQueue();

	return 0;
}
