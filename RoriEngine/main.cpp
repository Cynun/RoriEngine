#include"gameMain.h"
#include<cstdio>
#include<cstdlib>
#include<chrono>
#include<thread>
#include<ctime>
volatile std::atomic_bool running = true;

#ifdef DEBUG1
namespace RoriTestal {

	class Test :public Object {
		void startup(void) {
			;
		}
		void tick(void) {
			;
		}
		void cleanup(void) {
			;
		}
	};

	void objectListTest(void) {
		auto pt = new Test;
		printf("!%3d ", pt->getID());
		std::this_thread::sleep_for(std::chrono::seconds(rand() % 20));
		printf("x%3d ", pt->getID());
		delete pt;
	}
	int ObjectListTest(void) {

		srand(time(0));
		int threadNums = 1000;
		std::vector<std::thread> threadList;
		threadList.reserve(threadNums);

		// 1 创建 threadNums 个线程
		for (int idx = 0; idx < threadNums; ++idx) {
			threadList.emplace_back(std::thread(objectListTest));
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 20));
		}

		// 2 终止 threadNums 个线程
		for (int idx = 0; idx < threadNums; ++idx) {
			threadList[idx].join();
		}

		std::cout << "main thread end" << std::endl;
		return 0;
	}

}
#endif // DEBUG1

using namespace RoriDef;
using namespace RoriUsefuls;
using namespace RoriObjectal;
using namespace RoriEngine;

std::chrono::system_clock::time_point NextTargetTime, StartTime;
auto TPSdura(std::chrono::microseconds(1000000)/RoriDef::TargetTPS);


int main(void) {

	RoriEngine::EngineStart();
	RoriGamery::initialize();
	
	for (auto it : RegisteredServices_ID) {
		//Log("init " + it.second->GetName());
		it.second->startup();
	}

	NextTargetTime = StartTime = std::chrono::system_clock::now();

	while (running) {

		NextTargetTime = NextTargetTime + TPSdura;

		for (auto it : RegisteredServices_ID) {
			//Log("ticking " + it.second->GetName());
			if(it.second->IsActive())it.second->tick();
		}

		RoriUsefuls::sleep_until(NextTargetTime);
		//std::this_thread::sleep_until(NextTargetTime);
	}

	return 0;
}