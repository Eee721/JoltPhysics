#include "JoltWorld.h"
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
JPH_NAMESPACE_BEGIN

//TempAllocator* JoltWorld::CreateTempAlloctor(int size)
//{
//	JoltWorld::sTempAllocator = new TempAllocatorImpl(16 * 1024 * 1024);
//	return JoltWorld::sTempAllocator;
//}

JoltWorld* JoltWorld::Create(int allocSize, int maxPhysicsJobs, int maxPhysicsBarriers)
{
	JoltWorld* world = new JoltWorld();
	world->mTempAllocator = new TempAllocatorImpl(allocSize);
	world->mJobSystemThreadPool = new JobSystemThreadPool(maxPhysicsJobs, maxPhysicsBarriers, thread::hardware_concurrency() - 1);
}

void JoltWorld::Shutdown()
{
	if (mTempAllocator != nullptr)
	{
		delete mTempAllocator;
		mTempAllocator = nullptr;
	}
	if (mJobSystemThreadPool != nullptr)
	{
		delete mJobSystemThreadPool;
		mJobSystemThreadPool = nullptr;
	}
	//TODO,other clean
}

JoltWorld::~JoltWorld()
{
	Shutdown();
}


JPH_NAMESPACE_END
