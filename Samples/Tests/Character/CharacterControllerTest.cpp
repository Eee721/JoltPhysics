
#include <TestFramework.h>

#include <Tests/Character/CharacterControllerTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterControllerTest)
{
	JPH_ADD_BASE_CLASS(CharacterControllerTest, CharacterBaseTest)
}

void CharacterControllerTest::Initialize()
{
	CharacterBaseTest::Initialize();

	// Create 'player' character
	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
	/*	settings->mMaxSlopeAngle = sMaxSlopeAngle;
		settings->mMaxStrength = sMaxStrength;
		settings->mShape = mStandingShape;
		settings->mBackFaceMode = sBackFaceMode;
		settings->mCharacterPadding = sCharacterPadding;
		settings->mPenetrationRecoverySpeed = sPenetrationRecoverySpeed;
		settings->mPredictiveContactDistance = sPredictiveContactDistance;
		settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); */// Accept contacts that touch the lower sphere of the capsule
		//mCharacter = new CharacterVirtual(settings, RVec3::sZero(), Quat::sIdentity(), mPhysicsSystem);
		//mCharacter->SetListener(this);

	mCharacterController = new CharacterController(settings, RVec3::sZero(), Quat::sIdentity(), mPhysicsSystem);
	mCharacterController->SetTempAllocator(mTempAllocator);
	mCharacterController->SetPosition(RVec3::sZero());
	mCharacterController->SetRotation(Quat::sIdentity());
	mCharacterController->SetCosMaxSlopeAngle(sMaxSlopeAngle);
	mCharacterController->SetMaxStrength(sMaxStrength);
	mCharacterController->SetFilter(Layers::MOVING, Layers::MOVING);
	//static constexpr float	cCharacterHeightStanding = 1.35f;
	//static constexpr float	cCharacterRadiusStanding = 0.3f;
	//static constexpr float	cCharacterHeightCrouching = 0.8f;
	//static constexpr float	cCharacterRadiusCrouching = 0.3f;
	mCharacterController->CreateCapsuleShape(0, 1.35f, 0.3f);
	mCharacterController->CreateCapsuleShape(1, 0.8f, 0.3f);
	mCharacterController->SwitchSpape(0);
	mCharacterController->SetCharacterPadding(sCharacterPadding);
	mCharacterController->SetPenetrationRecoverySpeed(sPenetrationRecoverySpeed);
	mCharacterController->SetPredictiveContactDistance(sPredictiveContactDistance);
	mCharacterController->SetSupportingVolume(Vec3::sAxisY(), -cCharacterRadiusStanding);

	mCharacterController->SetListener(this);
}

void CharacterControllerTest::PrePhysicsUpdate(const PreUpdateParams& inParams)
{
	CharacterBaseTest::PrePhysicsUpdate(inParams);

	// Draw character pre update (the sim is also drawn pre update)
	RMat44 com = mCharacterController->GetCenterOfMassTransform();
#ifdef JPH_DEBUG_RENDERER
	mCharacter->GetShape()->Draw(mDebugRenderer, com, Vec3::sReplicate(1.0f), Color::sGreen, false, true);
#endif // JPH_DEBUG_RENDERER

	// Draw shape including padding (only implemented for capsules right now)
	if (static_cast<const RotatedTranslatedShape*>(mCharacterController->GetShape())->GetInnerShape()->GetSubType() == EShapeSubType::Capsule)
	{
		if (mCharacterController->GetShapeIndex() == 0)
			mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightStanding, cCharacterRadiusStanding + mCharacterController->GetCharacterPadding(), Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
		else
			mDebugRenderer->DrawCapsule(com, 0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching + mCharacterController->GetCharacterPadding(), Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	}

	// Remember old position
	RVec3 old_position = mCharacterController->GetPosition();

	// Settings for our update function
	CharacterVirtual::ExtendedUpdateSettings update_settings;
	if (!sEnableStickToFloor)
		update_settings.mStickToFloorStepDown = Vec3::sZero();
	else
		update_settings.mStickToFloorStepDown = -mCharacterController->GetUp() * update_settings.mStickToFloorStepDown.Length();
	if (!sEnableWalkStairs)
		update_settings.mWalkStairsStepUp = Vec3::sZero();
	else
		update_settings.mWalkStairsStepUp = mCharacterController->GetUp() * update_settings.mWalkStairsStepUp.Length();

	// Update the character position
	mCharacterController->ExtendedUpdate(inParams.mDeltaTime,
		-mCharacterController->GetUp() * mPhysicsSystem->GetGravity().Length(),
		update_settings,
		mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING),
		{ },
		{ },
		*mTempAllocator);

	// Calculate effective velocity
	RVec3 new_position = mCharacterController->GetPosition();
	Vec3 velocity = Vec3(new_position - old_position) / inParams.mDeltaTime;

	// Draw state of character
	DrawCharacterState(mCharacterController, mCharacterController->GetWorldTransform(), velocity);

	// Draw labels on ramp blocks
	for (size_t i = 0; i < mRampBlocks.size(); ++i)
		mDebugRenderer->DrawText3D(mBodyInterface->GetPosition(mRampBlocks[i]), StringFormat("PushesPlayer: %s\nPushable: %s", (i & 1) != 0 ? "True" : "False", (i & 2) != 0 ? "True" : "False"), Color::sWhite, 0.25f);
}

void CharacterControllerTest::HandleInput(Vec3Arg inMovementDirection, bool inJump, bool inSwitchStance, float inDeltaTime)
{
	bool player_controls_horizontal_velocity = sControlMovementDuringJump || mCharacterController->IsSupported();
	if (player_controls_horizontal_velocity)
	{
		// Smooth the player input
		mDesiredVelocity = 0.25f * inMovementDirection * sCharacterSpeed + 0.75f * mDesiredVelocity;

		// True if the player intended to move
		mAllowSliding = !inMovementDirection.IsNearZero();
	}
	else
	{
		// While in air we allow sliding
		mAllowSliding = true;
	}

	// Update the character rotation and its up vector to match the up vector set by the user settings
	Quat character_up_rotation = Quat::sEulerAngles(Vec3(sUpRotationX, 0, sUpRotationZ));
	mCharacterController->SetUp(character_up_rotation.RotateAxisY());
	mCharacterController->SetRotation(character_up_rotation);

	// Determine new basic velocity
	Vec3 current_vertical_velocity = mCharacterController->GetLinearVelocity().Dot(mCharacterController->GetUp()) * mCharacterController->GetUp();
	Vec3 ground_velocity = mCharacterController->GetGroundVelocity();
	Vec3 new_velocity;
	if (mCharacterController->GetGroundState() == CharacterVirtual::EGroundState::OnGround // If on ground
		&& (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.1f) // And not moving away from ground
	{
		// Assume velocity of ground when on ground
		new_velocity = ground_velocity;

		// Jump
		if (inJump)
			new_velocity += sJumpSpeed * mCharacterController->GetUp();
	}
	else
		new_velocity = current_vertical_velocity;

	// Gravity
	new_velocity += (character_up_rotation * mPhysicsSystem->GetGravity()) * inDeltaTime;

	if (player_controls_horizontal_velocity)
	{
		// Player input
		new_velocity += character_up_rotation * mDesiredVelocity;
	}
	else
	{
		// Preserve horizontal velocity
		Vec3 current_horizontal_velocity = mCharacterController->GetLinearVelocity() - current_vertical_velocity;
		new_velocity += current_horizontal_velocity;
	}

	// Update character velocity
	mCharacterController->SetLinearVelocity(new_velocity);

	// Stance switch
	if (inSwitchStance)
	{
		mCharacterController->SwitchSpape(1 - mCharacterController->GetShapeIndex());
	}

	//mCharacterController->SetShape(mCharacterController->GetShape() == mStandingShape ? mCrouchingShape : mStandingShape, 1.5f * mPhysicsSystem->GetPhysicsSettings().mPenetrationSlop, mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING), { }, { }, *mTempAllocator);
}

void CharacterControllerTest::AddConfigurationSettings(DebugUI* inUI, UIElement* inSubMenu)
{
	inUI->CreateComboBox(inSubMenu, "CC Back Face Mode", { "Ignore", "Collide" }, (int)sBackFaceMode, [=](int inItem) { sBackFaceMode = (EBackFaceMode)inItem; });
	inUI->CreateSlider(inSubMenu, "Up Rotation X (degrees)", RadiansToDegrees(sUpRotationX), -90.0f, 90.0f, 1.0f, [](float inValue) { sUpRotationX = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Up Rotation Z (degrees)", RadiansToDegrees(sUpRotationZ), -90.0f, 90.0f, 1.0f, [](float inValue) { sUpRotationZ = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Slope Angle (degrees)", RadiansToDegrees(sMaxSlopeAngle), 0.0f, 90.0f, 1.0f, [](float inValue) { sMaxSlopeAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Strength (N)", sMaxStrength, 0.0f, 500.0f, 1.0f, [](float inValue) { sMaxStrength = inValue; });
	inUI->CreateSlider(inSubMenu, "Character Padding", sCharacterPadding, 0.01f, 0.5f, 0.01f, [](float inValue) { sCharacterPadding = inValue; });
	inUI->CreateSlider(inSubMenu, "Penetration Recovery Speed", sPenetrationRecoverySpeed, 0.0f, 1.0f, 0.05f, [](float inValue) { sPenetrationRecoverySpeed = inValue; });
	inUI->CreateSlider(inSubMenu, "Predictive Contact Distance", sPredictiveContactDistance, 0.01f, 1.0f, 0.01f, [](float inValue) { sPredictiveContactDistance = inValue; });
	inUI->CreateCheckBox(inSubMenu, "Enable Walk Stairs", sEnableWalkStairs, [](UICheckBox::EState inState) { sEnableWalkStairs = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Enable Stick To Floor", sEnableStickToFloor, [](UICheckBox::EState inState) { sEnableStickToFloor = inState == UICheckBox::STATE_CHECKED; });
}

void CharacterControllerTest::SaveState(StateRecorder& inStream) const
{
	CharacterBaseTest::SaveState(inStream);

	mCharacterController->SaveState(inStream);

	bool is_standing = mCharacterController->GetShape() == mStandingShape;
	inStream.Write(is_standing);

	inStream.Write(mAllowSliding);
	inStream.Write(mDesiredVelocity);
}

void CharacterControllerTest::RestoreState(StateRecorder& inStream)
{
	CharacterBaseTest::RestoreState(inStream);

	mCharacterController->RestoreState(inStream);

	bool is_standing = mCharacterController->GetShapeIndex() == 0;//mCharacterController->GetShape() == mStandingShape; // Initialize variable for validation mode
	inStream.Read(is_standing);
	//mCharacterController->SetShape(is_standing ? mStandingShape : mCrouchingShape, FLT_MAX, { }, { }, { }, { }, *mTempAllocator);
	mCharacterController->SwitchSpape(is_standing ? 0 : 1);
	inStream.Read(mAllowSliding);
	inStream.Read(mDesiredVelocity);
}

void CharacterControllerTest::OnAdjustBodyVelocity(const CharacterVirtual* inCharacter, const Body& inBody2, Vec3& ioLinearVelocity, Vec3& ioAngularVelocity)
{
	// Apply artificial velocity to the character when standing on the conveyor belt
	if (inBody2.GetID() == mConveyorBeltBody)
		ioLinearVelocity += Vec3(0, 0, 2);
}

void CharacterControllerTest::OnContactAdded(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	// Dynamic boxes on the ramp go through all permutations
	Array<BodyID>::const_iterator i = find(mRampBlocks.begin(), mRampBlocks.end(), inBodyID2);
	if (i != mRampBlocks.end())
	{
		size_t index = i - mRampBlocks.begin();
		ioSettings.mCanPushCharacter = (index & 1) != 0;
		ioSettings.mCanReceiveImpulses = (index & 2) != 0;
	}

	// If we encounter an object that can push us, enable sliding
	if (ioSettings.mCanPushCharacter && mPhysicsSystem->GetBodyInterface().GetMotionType(inBodyID2) != EMotionType::Static)
		mAllowSliding = true;
}

void CharacterControllerTest::OnContactSolve(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial* inContactMaterial, Vec3Arg inCharacterVelocity, Vec3& ioNewCharacterVelocity)
{
	// Don't allow the player to slide down static not-too-steep surfaces when not actively moving and when not on a moving platform
	if (!mAllowSliding && inContactVelocity.IsNearZero() && !inCharacter->IsSlopeTooSteep(inContactNormal))
		ioNewCharacterVelocity = Vec3::sZero();
}
