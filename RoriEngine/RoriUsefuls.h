#pragma once

namespace RoriUsefuls { //Useful definitions & functions.

	void dummy(void) { ; };//dummy function.

	struct ipt2d {
		int x, y;
	};
	struct ipt3d {
		int x, y, z;
	};

	//Explode: Print the Reason and Stop the program.
	void Explode(std::string reason)noexcept {
		//output rules:
		//[.]Log [!]Notice [X]Error [i]Information(use with Error)
		std::cerr << "[X]RoriEngine Exploded.\a\n"
				  << "[i]Reason: " << reason << std::endl
				  << "[!]Program stopped." << std::endl;
		exit(1);
	}

	void Log(std::string para) {
		std::cout << "[.]" << para << std::endl;
	}
	void Notice(std::string para) {
		std::cout << "[!]" << para << std::endl;
	}

	void sleep_until(std::chrono::system_clock::time_point tp) {
		while (std::chrono::system_clock::now() < tp);
	}

}
#ifdef __RORIENGINE_DEMO__

using namespace RoriObjectal;

struct ObjectDataType {

};

class DemoService :public Service {
private:

	std::mutex ObjectDataLock;
	std::map<int, ObjectDataType> ObjectData;

	//timings
	volatile unsigned long long TickNo = 0;
	std::chrono::system_clock::time_point ServiceStartTime;

public:
	DemoService(int ServiceOrder, std::string name = "") : Service(ServiceOrder, name) {
		;
	}
	void Service::startup(void) {
		;
	}

	void setX(Object* obj, Type X) {
		ObjectDataLock.lock();
		ObjectData[obj->GetID()].X = X;
		ObjectDataLock.unlock();
	}

	Type getX(Object* obj) {
		return ObjectData[obj->GetID()].x;
	}


	void Service::tick(void) {
		this->RegisteredObjectsLock.lock();
		for (auto it : this->RegisteredObjects) {
			fun(& ObjectData[it->GetID()]);
		}
		this->RegisteredObjectsLock.unlock();

		if (!(EngineTick % 30)) {
			RoriUsefuls::Notice("Service TPS:" + std::to_string(ServiceTPS));
		}
	}
	void Service::cleanup(void) {

	}
};
#endif