#pragma once


namespace RoriObjectal { //Objects & Services.
	using namespace RoriUsefuls;

	class Service;
	class Object;

	std::vector<Object*> ObjectList;//stores pointers to object
	std::stack<int> EmptyPlaces;//stores empty spaces' ID in objectList.
	std::mutex ObjectListLock;//this is the lock for both objectList AND emptyPlaces.

	class Object { //the base class for game objects.
	private:

	protected:
		int ID;
		std::string name;
	public:
		Object(const Service&) = delete;

		Object(std::string name="") {
			this->name = name;

			//find the latest empty place and assign it
			ObjectListLock.lock();

			if (EmptyPlaces.empty()) {
				this->ID = int(ObjectList.size());
				ObjectList.push_back(this);
			} else {
				this->ID = EmptyPlaces.top();
				ObjectList[this->ID] = this;
				EmptyPlaces.pop();
			}

			ObjectListLock.unlock();
		}

		void SetName(std::string Name) { this->name = Name; }

		int GetID(void) { return this->ID; }
		std::string GetName(void) { return this->name; }

		virtual void startup(void) = 0;// {Explode("Base Object " + this->name + " is being startup()ed	directly"); }
		virtual void tick(void) = 0;// {Explode("Base Object " + this->name + " is being tick()ed	directly"); }
		virtual void cleanup(void) = 0;// {Explode("Base Object " + this->name + " is being cleanup()ed	directly"); }

		~Object() {
			//add the empty place to the list.
			ObjectListLock.lock();
			ObjectList[this->ID] = nullptr;
			EmptyPlaces.push(this->ID);
			ObjectListLock.unlock();
		}
	};


	std::map<std::string, Service*> RegisteredServices_Name;//int:Service Order,Service*:pointer to service.
	std::map<int, Service*> RegisteredServices_ID;//int:Service Order,Service*:pointer to service.
	std::mutex ServiceListLock;

	class Service { //a service handle all registered objects every tick.
	private:

	protected:
		int order;
		std::string name;
		std::mutex RegisteredObjectsLock;
		std::set<Object*> RegisteredObjects;
		bool Active = false;
	public:
		Service(void) = delete;
		Service(const Service&) = delete;

		Service(int ServiceOrder, std::string name) {
			ServiceListLock.lock();
			this->order = ServiceOrder;
			this->name = name;
			this->Active = false;
			if ((RegisteredServices_ID[ServiceOrder] != nullptr || RegisteredServices_Name[name] != nullptr ))
				Explode("Service Duplicated:"+this->name+" and "+RegisteredServices_ID[ServiceOrder]->name+"(ID "+std::to_string(ServiceOrder)+")");
			RegisteredServices_Name[name] = this;
			RegisteredServices_ID[order] = this;
			ServiceListLock.unlock();
			this->Active = true;
		}

		std::string GetName	(void) const { return this->name;	}
		const int	GetOrder(void) const { return this->order;	}
		const bool	IsActive(void) const { return this->Active; }

		void Regist(Object* obj) {
			RegisteredObjectsLock.lock();
			this->RegisteredObjects.insert(obj);
			RegisteredObjectsLock.unlock();
		}
		void unRegist(Object* obj) {
			RegisteredObjectsLock.lock();
			this->RegisteredObjects.erase(obj);
			RegisteredObjectsLock.unlock();
		}

		virtual void startup(void)	= 0;
		virtual void tick(void)		= 0;
		virtual void cleanup(void)	= 0;

		~Service() {
			ServiceListLock.lock();
			RegisteredServices_Name.erase(this->name);
			RegisteredServices_ID.erase(this->order);
			ServiceListLock.unlock();
		}
	};
}
