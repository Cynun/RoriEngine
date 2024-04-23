#pragma once

#ifndef _RORI_ENGINE_
#define _RORI_ENGINE_

#include<functional>
#include<iostream>
#include<chrono>
#include<string>
#include<vector>
#include<atomic>
#include<mutex>
#include<stack>
#include<list>
#include<map>
#include<set>

#include"RoriWarpper.h"
#include"RoriUsefuls.h"
#include"RoriObjectal.h"


namespace RoriEngine {

	unsigned long long EngineTick = 0;				//TickNo
	volatile float EngineTPS = RoriDef::TargetTPS;	//Real TPS

	using namespace RoriObjectal;

	namespace RoriObjectalService {

		using namespace RoriObjectal;

		std::chrono::system_clock::time_point ServiceStartTime, now;

		class RoriObjectalService :public Service {
		public:
			RoriObjectalService(int ServiceOrder, std::string name = "") : Service(ServiceOrder, name) {
				;
			}
			void Service::startup(void) {
				ServiceStartTime = std::chrono::system_clock::now();
			}

			void Service::tick(void) {
				RoriObjectal::ObjectListLock.lock();
				for (auto it : RoriObjectal::ObjectList) {
					it->tick();
				}
				RoriObjectal::ObjectListLock.unlock();

				EngineTick++;

				now = std::chrono::system_clock::now();
				EngineTPS = (EngineTick ? EngineTick : 1) * 1000000.0f / ((std::chrono::duration_cast<std::chrono::microseconds>(now - ServiceStartTime)).count());
				
				if (!(EngineTick % 30))RoriUsefuls::Notice("Objectal TPS:" + std::to_string(EngineTPS));
			}
			void Service::cleanup(void) {

			}
		};
	}

	namespace RoriPhysical {
		struct ObjectPhysicsDataType {
			bool freeze;
			float mass;
			float x, y;
			float ax, ay;
		};
		struct WorldPhysicsDataType {
			float g;
		};

		class Physical :public Service {
		private:
			WorldPhysicsDataType WorldPhysicsData;			//one and only PhysicsData

			std::mutex ObjectDataLock;
			std::map<int, ObjectPhysicsDataType> ObjectData;//[ID->data] map
		public:
			Physical(int ServiceOrder, std::string name = "") : Service(ServiceOrder, name) {//Constructor
				WorldPhysicsData.g = 9.8f;
			}

			void setPhysicsAttribute(WorldPhysicsDataType attr) {
				this->WorldPhysicsData = attr;
			}
			WorldPhysicsDataType getPhysicsAttribute(Object* obj) {
				return this->WorldPhysicsData;
			}
			void setObjAttribute(Object* obj, ObjectPhysicsDataType attr) {
				ObjectDataLock.lock();
				ObjectData[obj->GetID()] = attr;
				ObjectDataLock.unlock();
			}
			ObjectPhysicsDataType getObjAttribute(Object* obj) {
				return ObjectData[obj->GetID()];
			}

			void Service::startup(void) {
				RoriUsefuls::Log("Physics startuped!");

				WorldPhysicsData.g = 9.8f;
			}

			void Service::tick(void) {
				this->RegisteredObjectsLock.lock();
				for (auto it : this->RegisteredObjects) {//Process all the Objects.
					ObjectPhysicsDataType &curretObjData = ObjectData[it->GetID()];
					curretObjData.x += curretObjData.ax/EngineTPS;
					curretObjData.y += curretObjData.ay/EngineTPS;
					curretObjData.ay=WorldPhysicsData.g;

				//	RoriUsefuls::Log("Object "+it->getName()+"(ID "+std::to_string(it->getID())+") is falling!"
				//					+"x:"+std::to_string(curretObjData.x)+" y: "+ std::to_string(curretObjData.y));
				}
				this->RegisteredObjectsLock.unlock();
			}
			void Service::cleanup(void) {
				;
			}
		};
	}

	namespace RoriGraphical {

		using namespace RoriDef;

		volatile float GraphicsFPS = RoriDef::TargetFPS;
		
		struct ObjectGraphicalDataType {
			ipt3d pos = { 0,0,0 };
			RoriDef::Image img = RoriDef::EmptyImage;//pointer to an array
			short scale=1;
		};

		struct GraphDataPtrCmp{
			bool operator()(ObjectGraphicalDataType *a, ObjectGraphicalDataType *b) const{
				return a->pos.z > b->pos.z;
			}
		};

		class Graphical :public Service {
		private:

			std::mutex ObjectDataLock;
			std::map<int, ObjectGraphicalDataType> ObjectData;
			
			//Platformal
			PlatformAPI::Graphical::WindowType Window;

			std::mutex BufferLock;
			RoriDef::Image Buffer = RoriDef::EmptyImage;
			std::atomic<bool> StopRender = false;

			std::thread RenderThread; 
			
			//timings
			volatile unsigned long long FrameNo = 0;
			std::chrono::high_resolution_clock::time_point GraphicsStartTime;

			void Render(void) {
				/*
				long long ElapsedMicroseconds;
				LARGE_INTEGER StartingTime, EndingTime;
				LARGE_INTEGER Frequency;


				// Activity to be timed

				QueryPerformanceCounter(&EndingTime);
				ElapsedMicroseconds = EndingTime.QuadPart - StartingTime.QuadPart;


				//
				// We now have the elapsed number of ticks, along with the
				// number of ticks-per-second. We use these values
				// to convert to the number of elapsed microseconds.
				// To guard against loss-of-precision, we convert
				// to microseconds *before* dividing by ticks-per-second.
				//

				ElapsedMicroseconds= (ElapsedMicroseconds*1000000)/ Frequency.QuadPart;
				*/
				GraphicsStartTime = std::chrono::high_resolution_clock::now();
				while (true) {
					if (StopRender) break;
					BufferLock.lock();
					PlatformAPI::Graphical::SyncToScreen(WindowW, WindowH, Buffer);
					BufferLock.unlock();
					PlatformAPI::Graphical::FlushDraw(Window);

					FrameNo++;
					auto now = std::chrono::high_resolution_clock::now();
					GraphicsFPS = (FrameNo?FrameNo:1) * 1000000.0f / ((std::chrono::duration_cast<std::chrono::microseconds>(now - GraphicsStartTime)).count());


					//RoriUsefuls::sleep_until_hr(now + (std::chrono::microseconds(long long(1000000.0 / TargetFPS))));
				}
			}
		public:
			Graphical(int ServiceOrder, std::string name = "") : Service(ServiceOrder, name) {
				Window = PlatformAPI::Graphical::EmptyWindow;
			}
			void Service::startup(void) {
				Window=PlatformAPI::Graphical::InitGraph(WindowW, WindowH);

				this->Buffer.width = WindowW; this->Buffer.height = WindowH;
				this->Buffer.data = new RoriDef::Color[WindowW * WindowH];
				
				PlatformAPI::Graphical::BeginDraw(this->Window);
				RenderThread = std::thread(&Graphical::Render,this);
			}

			void setXY(Object* obj, ipt2d xy) {
				ObjectData[obj->GetID()].pos.x = xy.x;
				ObjectData[obj->GetID()].pos.y = xy.y;
			}

			ipt2d getXY(Object* obj) {
				ipt2d xy;
				xy.x = ObjectData[obj->GetID()].pos.x;
				xy.y = ObjectData[obj->GetID()].pos.y;
				return xy;
			}

			void setX(Object* obj, int x) { ObjectDataLock.lock(); ObjectData[obj->GetID()].pos.x = x; ObjectDataLock.unlock(); }
			int getX(Object* obj) { return ObjectData[obj->GetID()].pos.x; }
			void setY(Object* obj, int x) { ObjectDataLock.lock(); ObjectData[obj->GetID()].pos.x = x; ObjectDataLock.unlock(); }
			int getY(Object* obj) { return ObjectData[obj->GetID()].pos.x; }
			void setZ(Object* obj, int x) { ObjectDataLock.lock(); ObjectData[obj->GetID()].pos.x = x; ObjectDataLock.unlock(); }
			int getZ(Object* obj) { return ObjectData[obj->GetID()].pos.x; }

			void setImg(Object* obj, RoriDef::Image Img) { ObjectDataLock.lock(); ObjectData[obj->GetID()].img = Img; ObjectDataLock.unlock(); }
			RoriDef::Image getImg(Object* obj) { return ObjectData[obj->GetID()].img; }

			void Service::tick(void) {
				BufferLock.lock();
				memset(Buffer.data, 0, (sizeof(*Buffer.data) * Buffer.width * Buffer.height));

				std::multiset<ObjectGraphicalDataType*,GraphDataPtrCmp> ReorderList;
				this->RegisteredObjectsLock.lock();
				for (auto it : this->RegisteredObjects) {
					ReorderList.insert(&ObjectData[it->GetID()]);
				}
				this->RegisteredObjectsLock.unlock();
				for (auto it : ReorderList) {
					int x, y, u, v;
					x = max(0,it->pos.x);
					y = max(0,it->pos.y);
					u = min(WindowW, it->pos.x + it->img.width);
					v = min(WindowH, it->pos.y + it->img.height);
					for (int py = y; py < v; py++) {
						for (int px = x; px < u; px++) {
							//Buffer.data[px + py * WindowW] = RoriDef::ColorBlendS(Buffer.data[px + py * WindowW],it->img.data[(px-x)+(py-y)*(it->img.width)]);
							Buffer.data[px + py * WindowW] = it->img.data[(px - x) + (py - y) * (it->img.width)];
							//Buffer.data[px + py * WindowW].a = 1;
						}
					}
				}

				BufferLock.unlock();

				if (!(EngineTick % 30)) {
					RoriUsefuls::Notice("Graph FPS:" + std::to_string(GraphicsFPS));
				}
			}
			void Service::cleanup(void) {
				this->StopRender = true;
				this->BufferLock.lock();
				delete (this->Buffer).data;
				this->BufferLock.unlock();
				PlatformAPI::Graphical::EndDraw(this->Window);
			}
		};
	}

	RoriPhysical::Physical PhysicsEngine(1500, "Physical");
	RoriGraphical::Graphical GraphicsEngine(2000, "Graphical");
	RoriObjectalService::RoriObjectalService ObjectalService(3000, "Objectal");

	void EngineStart(void) {
		//Service Physical;
		//Service Objectal(2500, dummy, "Objectal");
		//Service Graphics(3500, dummy, "Graphics");
	}
}
#endif // _RORI_ENGINE_