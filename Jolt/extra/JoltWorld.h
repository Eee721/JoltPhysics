#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

JPH_NAMESPACE_BEGIN
class JoltWorld
{
public:
	static JoltWorld* Create(int allocSize, int maxPhysicsJobs, int maxPhysicsBarriers);
	void Shutdown();
	~JoltWorld();
	//static TempAllocator* CreateTempAlloctor(int size);
	//static TempAllocator* GetTempAlloctor() { return sTempAllocator; }
private:
	PhysicsSystem physics_system;
	TempAllocator* mTempAllocator;
	JobSystemThreadPool* mJobSystemThreadPool;
};
JPH_NAMESPACE_END

