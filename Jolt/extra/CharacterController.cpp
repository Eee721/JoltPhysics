#include "CharacterController.h"


#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <cassert>

// All Jolt symbols are in the JPH namespace
JPH_NAMESPACE_BEGIN


CharacterController::CharacterController(const CharacterVirtualSettings* inSettings, RVec3Arg inPosition, QuatArg inRotation, PhysicsSystem* inSystem) :
	CharacterVirtual(inSettings, inPosition, inRotation, inSystem)
{
}


//void CharacterController::SetStandingShape(const Shape* shape)
//{
//	mStandingShape = shape;
//}
//
//void CharacterController::SetCrouchingShape(const Shape* shape)
//{
//	mCrouchingShape = shape;
//}

void CharacterController::CreateCapsuleShape(int iIndex, float height, float radius)
{
	assert((iIndex >= 0 && iIndex < shape_max));// , "[CharacterController::CreateCapsuleShape] iIndex out of range");
	mStandingShape[iIndex] = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * height + radius, 0), Quat::sIdentity(), new CapsuleShape(0.5f * height, radius)).Create().Get();
	if (GetShape() == nullptr)
	{
		SwitchSpape(iIndex);
	}
}

bool CharacterController::SwitchSpape(int iIndex)
{
	assert((iIndex >= 0 && iIndex < shape_max));// , "[CharacterController::SwitchSpape] iIndex out of range");
	mActiveShapeIndex = iIndex;
	return CharacterVirtual::SetShape(mStandingShape[iIndex], 1.5f * mSystem->GetPhysicsSettings().mPenetrationSlop, mSystem->GetDefaultBroadPhaseLayerFilter(mBroadPhaseLayerFilter), mSystem->GetDefaultLayerFilter(mObjectLayerFilter), { }, { }, *mTempAllocator);
}

//bool CharacterController::SetShape1(const Shape* inShape, uint16 layer1, uint16 layer2)
//{
//	//return false;
//	//SetShape
//	return SetShape(inShape, 1.5f * mSystem->GetPhysicsSettings().mPenetrationSlop, mSystem->GetDefaultBroadPhaseLayerFilter(layer1), mSystem->GetDefaultLayerFilter(layer2), { }, { }, *mTempAllocator);
//}
void CharacterController::Excute(float dt)
{

}
//CharacterVirtualSettings* CharacterController::GetSetting()
//{
//	if (mSettings == nullptr)
//	{
//		mSettings = new CharacterVirtualSettings();
//	}
//	return mSettings.GetPtr();
//}

//void CharacterController::Init()
//{
//}
JPH_NAMESPACE_END
