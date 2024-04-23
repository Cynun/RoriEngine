#pragma once

#include"RoriEngine.h"

using namespace RoriUsefuls;
using namespace RoriObjectal;

namespace RoriGamery {
	
	class Test :public Object {
	public:

		virtual void startup(void) {
			RoriEngine::PhysicsEngine.Regist(this);
			RoriEngine::PhysicsEngine.setObjAttribute(this, {false,1.0f,0,0,0,0});

			RoriEngine::GraphicsEngine.Regist(this);
			
			RoriDef::Image img,mask;
			PlatformAPI::FileAccess::LoadImage(img, _T(R"(..\Cynun.jpg)"));

			mask = RoriDef::EmptyImage;

			RoriEngine::GraphicsEngine.setImg(this, img);
			RoriEngine::GraphicsEngine.setXY(this, { 0, 0 });
		}
		virtual void tick(void) {
			RoriEngine::GraphicsEngine.setXY(this,{ RoriEngine::GraphicsEngine.getX(this) + 1, RoriEngine::GraphicsEngine.getY(this) + 1 });
		}
		virtual void cleanup(void) {
			RoriEngine::PhysicsEngine.unRegist(this);
			RoriEngine::GraphicsEngine.unRegist(this);
		}
	};

	Test test;
	void initialize(void) {
		test.startup();
	}


}