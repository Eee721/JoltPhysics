#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include "Jolt/Physics/Character/CharacterVirtual.h"

JPH_NAMESPACE_BEGIN

class CharacterController : public CharacterVirtual
{
public:
	CharacterController(const CharacterVirtualSettings* inSettings, RVec3Arg inPosition, QuatArg inRotation, PhysicsSystem* inSystem);
	//void SetStandingShape(const Shape* shape);
	//void SetCrouchingShape(const Shape* shape);

	void CreateCapsuleShape(int iIndex, float height, float radius);
	bool SwitchSpape(int iIndex);


	//CharacterVirtualSettings* GetSetting();
	//void Init();
	void SetFilter(ObjectLayer bf, ObjectLayer of) { mBroadPhaseLayerFilter = bf; mObjectLayerFilter = of; }
	//bool SetShape1(const Shape* inShape, uint16 layer1, uint16 layer2);
	//void SetSupportingVolume(Plane* plane) { mSupportingVolume = *plane; }	//		Offset(float inDistance) const { return Plane(mNormalAndConstant - Vec4(Vec3::sZero(), inDistance)); }
	//Plane* GetSupportingVolume() { return &mSupportingVolume; }
	void SetSupportingVolume(Vec3Arg inNormal, float inConstant) { mSupportingVolume = Plane(inNormal, inConstant); }
	void Excute(float dt);
	void SetTempAllocator(TempAllocator* ta) { mTempAllocator = ta; }
	int GetShapeIndex() const { return mActiveShapeIndex; }
protected:
	static const int		shape_max = 4;
	// The different stances for the character	
	RefConst<JPH::Shape>				mStandingShape[shape_max];
	int mActiveShapeIndex = -1;
	//RefConst<JPH::Shape>				mCrouchingShape;
	//Ref<CharacterVirtualSettings>		mSettings;
	TempAllocator* mTempAllocator = nullptr;
	bool		sEnableWalkStairs = true;
	bool		sEnableStickToFloor = true;
	ObjectLayer mBroadPhaseLayerFilter = 0;
	ObjectLayer mObjectLayerFilter = 0;
	bool SetShape(const Shape* inShape, float inMaxPenetrationDepth, const BroadPhaseLayerFilter& inBroadPhaseLayerFilter, const ObjectLayerFilter& inObjectLayerFilter, const BodyFilter& inBodyFilter, const ShapeFilter& inShapeFilter, TempAllocator& inAllocator) { return false; }
	//using CharacterVirtual::SetShape;
};

JPH_NAMESPACE_END

