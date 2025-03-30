#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "font.cpp"
#include "mesh.cpp"
#include "entity.cpp"
#include "animation.cpp"
#include "asset.cpp"
#include "render.cpp"
#include "debug_draw.cpp"
#include "ui.cpp"
#include "collision.cpp"

internal void
AudioInitialize(audio_state *AudioState, memory_arena *Arena)
{
	AudioState->Arena = Arena;
	AudioState->Channels = 0;
	AudioState->FreeChannels = 0;
}

internal void
SoundToOutput(audio_state *AudioState, game_sound_buffer *SoundBuffer, asset_manager *Assets, memory_arena *TempArena)
{
}

inline void
DefaultOrientationSet(entity *Entity)
{
	// Facing away

	f32 Radians = DirectionToEuler(V3(0.0f, 0.0f, -1.0f)).yaw;
	Entity->Theta = RadToDegrees(Radians);
	Entity->ThetaTarget = Entity->Theta; 
	Entity->dTheta = 0.0f;
	Entity->Orientation = Quaternion(V3(0.0f, -1.0f, 0.0f), Radians);
}

inline void
OBBInitialize(collision_volume *Volume, v3 Dim, quaternion Orientation)
{
	quaternion Q = Conjugate(Orientation);
	Volume->Dim = 0.99f*Dim;
	Volume->Center = 0.5f*V3(0.0f, Volume->Dim.y, 0.0f);
	Volume->Offset = 0.5f*V3(0.0f, Volume->Dim.y, 0.0f);
	Volume->X = Q * XAxis();
	Volume->Y = Q * YAxis();
	Volume->Z = Q * (-1.0f*ZAxis());
}

internal entity * 
PlayerAdd(game_state *GameState, v3 P, f32 Height, f32 Radius)
{
	entity *Entity = EntityAdd(GameState, EntityType_Player);

	FlagAdd(Entity, EntityFlag_YSupported);
	FlagAdd(Entity, EntityFlag_Collides);
	FlagAdd(Entity, EntityFlag_Moveable);
	FlagAdd(Entity, EntityFlag_Visible);

	Entity->P = P;
	Entity->dP = {};
	Entity->ddP = {};

	DefaultOrientationSet(Entity);

	Entity->MovementState = MovementState_Idle;
	Entity->Height = Height;
	Entity->MovementColliders.VolumeCount = 1;
	Entity->MovementColliders.Volumes = PushArray(&GameState->Arena, 1, collision_volume);

	collision_volume *Volume = Entity->MovementColliders.Volumes;
	Volume->Type = CollisionVolumeType_Capsule;
	Volume->Min = V3(0.0f, Radius, 0.0f);
	Volume->Max = V3(0.0f, Entity->Height - Radius, 0.0f);
	Volume->Radius = Radius;

	if(GameState->PlayerEntityIndex == 0)
	{
		GameState->PlayerEntityIndex = Entity->ID;
	}

	return(Entity);
}

internal entity * 
XBotAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P, 1.8f, 0.4f);
	Player->Acceleration = 50.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 15.0f;
	Player->VisualScale = V3(0.01f);

	model *Model		= FindModel(&GameState->AssetManager, "XBot").Model;
	animation_graph *G  = FindGraph(&GameState->AssetManager, "XBot_AnimationGraph").Graph;
	asset_entry Entry	= FindAnimation(&GameState->AssetManager, "XBot_IdleLeft");

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlag_Looping, 0.2f);

	return(Player);
}

internal entity *
YBotAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P, 1.8f, 0.4f);
	Player->Acceleration = 50.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 15.0f;
	Player->VisualScale = V3(0.011f);

	collision_group *Group = &Player->CombatColliders;
	Group->VolumeCount = 4;
	Group->Volumes = PushArray(&GameState->Arena, Group->VolumeCount, collision_volume);
	for(u32 VolumeIndex = 0; VolumeIndex < Group->VolumeCount; ++VolumeIndex)
	{
		collision_volume *Volume = Group->Volumes + VolumeIndex;
		Volume->Dim = V3(0.2f);
	}

	Player->Attacks[AttackType_Neutral1].Type = AttackType_Neutral1;
	Player->Attacks[AttackType_Neutral1].Duration = 0.2f;
	Player->Attacks[AttackType_Neutral1].Power = 0.07f;

	Player->Attacks[AttackType_Neutral2].Type = AttackType_Neutral2;
	Player->Attacks[AttackType_Neutral2].Duration = 0.3f;
	Player->Attacks[AttackType_Neutral2].Power = 0.07f;

	model *Model = FindModel(&GameState->AssetManager, "YBot").Model;
	animation_graph *G  = FindGraph(&GameState->AssetManager, "YBot_AnimationGraph").Graph;
	asset_entry Entry = FindAnimation(&GameState->AssetManager, "YBot_FightIdleLeft");

	Assert(Model);
	Assert(G);
	Assert(Entry.SampledAnimation);

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlag_Looping, 0.2f);

	return(Player);
}

internal entity * 
KnightAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P, 1.8f, 0.4f);
	Player->Acceleration = 50.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 15.0f;
	Player->VisualScale = V3(0.011f);

	collision_group *Group = &Player->CombatColliders;
	Group->VolumeCount = 4;
	Group->Volumes = PushArray(&GameState->Arena, Group->VolumeCount, collision_volume);
	for(u32 VolumeIndex = 0; VolumeIndex < Group->VolumeCount; ++VolumeIndex)
	{
		collision_volume *Volume = Group->Volumes + VolumeIndex;
		Volume->Dim = V3(0.2f);
	}

	Player->Attacks[AttackType_Neutral1].Type = AttackType_Neutral1;
	Player->Attacks[AttackType_Neutral1].Duration = 0.4f;
	Player->Attacks[AttackType_Neutral1].Power = 3.0f;

	Player->Attacks[AttackType_Neutral2].Type = AttackType_Neutral2;
	Player->Attacks[AttackType_Neutral2].Duration = 0.5f;
	Player->Attacks[AttackType_Neutral2].Power = 5.0f;

	Player->Attacks[AttackType_Neutral3].Type = AttackType_Neutral3;
	Player->Attacks[AttackType_Neutral3].Duration = 0.5f;
	Player->Attacks[AttackType_Neutral3].Power = 7.0f;

	Player->Attacks[AttackType_Forward].Type = AttackType_Forward;
	Player->Attacks[AttackType_Forward].Duration = 0.7f;
	Player->Attacks[AttackType_Forward].Power = 12.0f;

	Player->Attacks[AttackType_Sprint].Type = AttackType_Sprint;
	Player->Attacks[AttackType_Sprint].Duration = 0.7f;
	Player->Attacks[AttackType_Sprint].Power = 12.0f;

	model *Model = FindModel(&GameState->AssetManager, "PaladinWithProp").Model;
	animation_graph *G  = FindGraph(&GameState->AssetManager, "Paladin_AnimationGraph").Graph;
	asset_entry Entry = FindAnimation(&GameState->AssetManager, "Paladin_SwordAndShieldIdle_00");

	Assert(Model);
	Assert(G);
	Assert(Entry.SampledAnimation);

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlag_Looping, 0.2f);

	return(Player);
}

internal entity * 
BruteAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P, 2.3f, 0.5f);
	Player->Acceleration = 40.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 10.0f;
	Player->VisualScale = V3(0.01f);

	model *Model = FindModel(&GameState->AssetManager, "Brute").Model;
	animation_graph *G  = FindGraph(&GameState->AssetManager, "Brute_AnimationGraph").Graph;
	asset_entry Entry = FindAnimation(&GameState->AssetManager, "Brute_Idle");

	Assert(Model);
	Assert(G);
	Assert(Entry.SampledAnimation);

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlag_Looping, 0.2f);

	collision_group *Group = &Player->CombatColliders;
	Group->VolumeCount = 1;
	Group->Volumes = PushArray(&GameState->Arena, Group->VolumeCount, collision_volume);
	collision_volume *Volume = Group->Volumes;

	Player->Attacks[AttackType_Neutral1].Type = AttackType_Neutral1;
	Player->Attacks[AttackType_Neutral1].Duration = 0.4f;
	Player->Attacks[AttackType_Neutral1].Power = 3.0f;

	Player->Attacks[AttackType_Neutral2].Type = AttackType_Neutral2;
	Player->Attacks[AttackType_Neutral2].Duration = 0.3f;
	Player->Attacks[AttackType_Neutral2].Power = 5.0f;

	Player->Attacks[AttackType_Neutral3].Type = AttackType_Neutral3;
	Player->Attacks[AttackType_Neutral3].Duration = 0.4f;
	Player->Attacks[AttackType_Neutral3].Power = 7.0f;

	Player->Attacks[AttackType_Strong].Type = AttackType_Strong;
	Player->Attacks[AttackType_Strong].Duration = 0.5f;
	Player->Attacks[AttackType_Strong].Power = 18.0f;

	Player->Attacks[AttackType_Sprint].Type = AttackType_Sprint;
	Player->Attacks[AttackType_Sprint].Duration = 2.0f;
	Player->Attacks[AttackType_Sprint].Power = 3.0f;

	return(Player);
}

inline void
EntityOrientationUpdate(entity *Entity, f32 dt, f32 AngularSpeed)
{
	v3 FacingDirection = Entity->dP;

	f32 TargetAngleInRad = DirectionToEuler(FacingDirection).yaw;
	Entity->ThetaTarget = RadToDegrees(TargetAngleInRad);
	if(Entity->Theta != Entity->ThetaTarget)
	{
		// TODO(Justin): Simplify this!
		f32 TargetAngleInDegrees = RadToDegrees(TargetAngleInRad);
		quaternion QTarget;
		if((TargetAngleInDegrees == 0.0f) ||
		   (TargetAngleInDegrees == -0.0f) ||
		   (TargetAngleInDegrees == 180.0f) ||
		   (TargetAngleInDegrees == -180.0f))
		{
			QTarget	= Quaternion(V3(0.0f, 1.0f, 0.0f), TargetAngleInRad);
		}
		else
		{
			QTarget	= Quaternion(V3(0.0f, 1.0f, 0.0f), -1.0f*TargetAngleInRad);
		}

		quaternion QCurrent = Entity->Orientation;
		Entity->Orientation = RotateTowards(QCurrent, QTarget, dt, AngularSpeed);
		Entity->Theta = -1.0f*RadToDegrees(YawFromQuaternion(Entity->Orientation));
	}
}

internal quad
QuadDefault(void)
{
	quad Result = {};

	v3 N = V3(0.0f, 1.0f, 0.0f);

	Result.Vertices[0].P = V3(-0.5f, 0.0f, 0.5f);
	Result.Vertices[1].P = V3(0.5f,	 0.0f, 0.5f);
	Result.Vertices[2].P = V3(0.5f,	 0.0f, -0.5f);
	Result.Vertices[3].P = V3(0.5f,	 0.0f, -0.5f);
	Result.Vertices[4].P = V3(-0.5f, 0.0f, -0.5f);
	Result.Vertices[5].P = V3(-0.5f, 0.0f, 0.5f);

	Result.Vertices[0].N = N;
	Result.Vertices[1].N = N;
	Result.Vertices[2].N = N;
	Result.Vertices[3].N = N;
	Result.Vertices[4].N = N;
	Result.Vertices[5].N = N;

	Result.Vertices[0].UV = V2(0.0f);
	Result.Vertices[1].UV = V2(1.0f, 0.0f);
	Result.Vertices[2].UV = V2(1.0f, 1.0f);
	Result.Vertices[3].UV = V2(1.0f, 1.0f);
	Result.Vertices[4].UV = V2(0.0f, 1.0f);
	Result.Vertices[5].UV = V2(0.0f);

	return(Result);
}

internal void
CameraSet(camera *Camera, v3 P, f32 Yaw, f32 Pitch)
{
	Camera->P = P;
	Camera->Direction.x = Cos(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
	Camera->Yaw = Yaw;
	Camera->Pitch = Pitch;
}

internal void
CameraDirectionUpdate(camera *Camera, f32 dXMouse, f32 dYMouse, f32 dt)
{
	f32 Yaw = Camera->Yaw + dt*dXMouse;
	f32 Pitch = Camera->Pitch + dt*dYMouse;
	Pitch = Clamp(-89.0f, Pitch, 89.0f);

	Camera->Direction.x = Cos(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
	Camera->Yaw = Yaw;
	Camera->Pitch = Pitch;
}

internal void
CameraTransformUpdate(game_state *GameState)
{
	GameState->CameraTransform =
		Mat4Camera(GameState->Camera.P, GameState->Camera.P + GameState->Camera.Direction);
}

internal void
PerspectiveTransformUpdate(game_state *GameState, f32 WindowWidth, f32 WindowHeight)
{
	// TODO(Justin): Probably dont want to hide the fact that we are re-calculating aspect here..
	GameState->Aspect = WindowWidth / WindowHeight;
	GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);
}

inline b32
EntitiesCanCollide(game_state *GameState, entity *A, entity *B, collision_type *DestType)
{
	b32 Result = false;

	if(A->ID != B->ID)
	{
		if(FlagIsSet(A, EntityFlag_Collides) && FlagIsSet(B, EntityFlag_Collides))
		{
			Result = true;
		}

		if(A->Type == EntityType_Player && B->Type == EntityType_Player)
		{
			Result = false;
		}
	}

	return(Result);
}

internal capsule
WeaponCapsule(entity *Entity)
{
	capsule Result;

	model *Model = Entity->AnimationPlayer->Model;
	mat4 T = Mat4Translate(Entity->P);
	mat4 R = QuaternionToMat4(Entity->Orientation);
	mat4 S = Mat4Scale(Entity->VisualScale);
	mat4 ModelToWorld = T*R*S;

	v3 CapsuleCenter = {};
	f32 Radius = 0.0f;
	f32 Length = 0.0f;
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		if(Mesh->Flags & MeshFlag_Weapon)
		{
			v3 Dim = AABBDim(Mesh->BoundingBox);
			Length = Max3(Dim);
			if(Length == Dim.x)
			{
				Radius = 0.5f*Max(Dim.y, Dim.z);
			}
			else if(Length == Dim.y)
			{
				Radius = 0.5f*Max(Dim.x, Dim.z);
			}
			else
			{
				Radius = 0.5f*Max(Dim.x, Dim.y);
			}

			Length = Entity->VisualScale.x * Length;
			Radius = Entity->VisualScale.x * Radius;

			mat4 WeaponTransform = Mesh->JointTransforms[Model->WeaponJointIndex];
			v3 JointP = Mat4ColumnGet(WeaponTransform, 3);
			v3 JointWorldP = ModelToWorld*(JointP);

			affine_decomposition D = Mat4AffineDecomposition(WeaponTransform);
			R = R*D.R;
			v3 Z = Mat4ColumnGet(R, 2);
			CapsuleCenter = JointWorldP + 0.4f*Z;

			R = QuaternionToMat4(Entity->Orientation)*D.R*Mat4XRotation(DegreeToRad(-90.0f));
			break;
		}
	}

	mat4 Transform = Mat4Translate(CapsuleCenter)*R*Mat4Scale(1.0f);
	v3 Min = Transform*V3(0.0f, Radius, 0.0f);
	v3 Max = Transform*V3(0.0f, Length - Radius, 0.0f);

	Result = CapsuleMinMaxRadius(Min, Max, Radius);
	return(Result);
}

internal void
EntityMove(game_state *GameState, entity *Entity, v3 ddP, f32 dt)
{
	if(!FlagIsSet(Entity, EntityFlag_YSupported))
	{
		ddP += V3(0.0f, -GameState->Gravity, 0.0f);
	}

	v3 DeltaP = {};
	animation_player *AnimationPlayer = Entity->AnimationPlayer;
	if(AnimationPlayer && AnimationPlayer->ControlsPosition)
	{
		// TODO(Justin): Robustness
		// TODO(Justin): Update velocity 
		v3 AnimationDelta = AnimationPlayer->RootMotionAccumulator;
		v3 AnimationVelocity = AnimationPlayer->RootVelocityAccumulator;

		v3 GameDelta = V3(Entity->VisualScale.x * AnimationDelta.x,
						  Entity->VisualScale.y * AnimationDelta.y,
						  Entity->VisualScale.z * AnimationDelta.z);
		v3 GameVelocity = V3(Entity->VisualScale.x * AnimationVelocity.x,
						  Entity->VisualScale.y * AnimationVelocity.y,
						  Entity->VisualScale.z * AnimationVelocity.z);

		DeltaP = Conjugate(Entity->Orientation)*GameDelta;
		v3 dPForFrame = Conjugate(Entity->Orientation)*GameVelocity;
		
		//ddP = Normalize(DeltaP);

		AnimationPlayer->RootMotionAccumulator = {};
		AnimationPlayer->RootVelocityAccumulator = {};

		Entity->ddP = ddP;
		Entity->dP += dPForFrame;
	}
	else
	{
		DeltaP = 0.5f * dt * dt * ddP + dt * Entity->dP;
	}

	v3 OldP = Entity->P;
	v3 DesiredP = OldP + DeltaP;
	Entity->dP = dt * ddP + Entity->dP;

	// TODO(Justin): Figure out the "correct" length here. Or have delta normalized instead of t normalized.
	v3 GroundP = {};
	v3 GroundDelta = V3(0.0f, -20.0f,0.0f);
	entity *EntityBelow = 0;
	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		b32 Collided = false;

		f32 tMin = 1.0f;
		v3 Normal = {};

		f32 tGround = 1.0f;
		v3 GroundNormal = {};

		entity *HitEntity = 0;
		entity *AttackedEntity = 0;
		entity *Region = 0;

		DesiredP = Entity->P + DeltaP;
		for(u32 TestEntityIndex = 1; TestEntityIndex < GameState->EntityCount; ++TestEntityIndex)
		{
			entity *TestEntity = GameState->Entities + TestEntityIndex;
			collision_type CollisionType = CollisionType_None;

			// TODO(Justin): Early out sphere intersection test.
			if(EntitiesCanCollide(GameState, Entity, TestEntity, &CollisionType))
			{
				v3 CurrentP = Entity->P; 
				v3 TestP = TestEntity->P;

				capsule Capsule = CapsuleGet(&Entity->MovementColliders);
				obb TestOBB = OrientedBoundingBoxGet(&TestEntity->MovementColliders);

				// TODO(Justin): Make sure the chosen sphere center is correct.
				v3 SphereCenter = CapsuleSphereCenterVsOBB(CurrentP, Capsule, TestEntity->P, TestOBB);
				v3 SphereRel = SphereCenter - (TestP + TestOBB.Center);
				if(MovingSphereHitOBB(SphereRel, Capsule.Radius, DeltaP, TestOBB, &Normal, &tMin))
				{
					Collided = true;
					HitEntity = TestEntity;
				}

				// GroundCheck
				v3 RelP		= (CurrentP + V3(0.0f, Capsule.Radius, 0.0f)) - (TestP + TestOBB.Center);
				if(MovingSphereHitOBB(RelP, Capsule.Radius, GroundDelta, TestOBB, &GroundNormal, &tGround))
				{
					EntityBelow = TestEntity;
				}

				// Attack check
				if(Entity->Flags & EntityFlag_AttackCollisionCheck)
				{
					if(TestEntity->Type == EntityType_Player)
					{
						capsule A = WeaponCapsule(Entity);
						capsule B = CapsuleGet(&TestEntity->MovementColliders);
						v3 CenterA = CapsuleCenter(A);
						v3 CenterB = TestEntity->P + CapsuleCenter(B);

						if(SpheresIntersect(CenterA, A.Radius, CenterB, B.Radius))
						{
							FlagAdd(TestEntity, EntityFlag_Attacked);
						}
					}
				}
			}

			if(TestEntity->Type == EntityType_WalkableRegion)
			{
				obb TestOBB = OrientedBoundingBoxGet(&TestEntity->MovementColliders);
				if(InAABB(AABBCenterDim(TestEntity->P, TestOBB.Dim), Entity->P))
				{
					Region = TestEntity;
				}
			}
		}

		v3 TestP = Entity->P + tMin * DeltaP; 
		f32 GroundY = Region->P.y + 0.01f;
		if(TestP.y < GroundY && IsGrounded(Entity) && (Normal.y < 0.0f))
		{
			// NOTE(Justin): OBB collision edge case. Do not accept the move by setting the velocity to 0 and breaking.
			// TODO(Justin): Fixing sticking
			Entity->dP = {};
			break;
		}

		Entity->P += tMin * DeltaP; 
		GroundP = Entity->P;
		GroundP += tGround * GroundDelta;

		Assert(EntityBelow);
		if(Collided)
		{
			Entity->dP = Entity->dP - Dot(Normal, Entity->dP) * Normal;
			DeltaP = DesiredP - Entity->P;
			DeltaP = DeltaP - Dot(Normal, DeltaP) * Normal;

			FlagAdd(Entity, EntityFlag_Collided);
		}
		else
		{
			break;
		}
	}

	Assert(EntityBelow);
	f32 YThreshold = 0.01f;
	f32 dY = Entity->P.y - GroundP.y;
	Entity->DistanceFromGround = dY;
	if(dY > YThreshold)
	{
		FlagClear(Entity, EntityFlag_YSupported);
	}
	else
	{
		// NOTE(Justin): At the start of a jump the distance from the ground is below the threshold
		// so we only add the y supported flag after the player is no longer considered as starting a jump 
		if(!FlagIsSet(Entity, EntityFlag_Jumping))
		{
			FlagAdd(Entity, EntityFlag_YSupported);
		}
	}
}

internal game_variable_data 
GameStateVariableData(u8 *VariableName)
{
	game_variable_data Result = {};

	if(StringsAreSame(VariableName, "camera_speed"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, Camera) + OffsetOf(camera, Speed);
	}
	else if(StringsAreSame(VariableName, "camera_yaw"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, Camera) + OffsetOf(camera, DefaultYaw);
	}
	else if(StringsAreSame(VariableName, "camera_pitch"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, Camera) + OffsetOf(camera, DefaultPitch);
	}
	else if(StringsAreSame(VariableName, "camera_position"))
	{
		Result.Type = game_state_variable_type_v3;
		Result.Offset = OffsetOf(game_state, Camera) + OffsetOf(camera, P);
	}
	else if(StringsAreSame(VariableName, "camera_offset_from_player"))
	{
		Result.Type = game_state_variable_type_v3;
		Result.Offset = OffsetOf(game_state, CameraOffsetFromPlayer);
	}
	else if(StringsAreSame(VariableName, "fov"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, FOV);
	}
	else if(StringsAreSame(VariableName, "z_near"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, ZNear);
	}
	else if(StringsAreSame(VariableName, "z_far"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, ZFar);
	}
	else if(StringsAreSame(VariableName, "gravity"))
	{
		Result.Type = game_state_variable_type_f32;
		Result.Offset = OffsetOf(game_state, Gravity);
	}
	else if(StringsAreSame(VariableName, "texture_width"))
	{
		Result.Type = game_state_variable_type_s32;
		Result.Offset = OffsetOf(game_state, Texture) + OffsetOf(texture, Width);
	}
	else if(StringsAreSame(VariableName, "texture_width"))
	{
		Result.Type = game_state_variable_type_s32;
		Result.Offset = OffsetOf(game_state, Texture) + OffsetOf(texture, Height);
	}
	else
	{
		// TODO(Justin): Error message and log
	}

	return(Result);
}

internal void
GameStateVariablesLoad(game_state *GameState, char *FileName)
{
	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		Assert(0);
	}

	u8 *Content = (u8 *)File.Content;

	u8 LineBuffer_[4096];
	MemoryZero(LineBuffer_, sizeof(LineBuffer_));
	u8 *LineBuffer = &LineBuffer_[0];

	u8 Word_[4096];
	MemoryZero(Word_, sizeof(Word_));
	u8 *Word = &Word_[0];

	while(*Content)
	{
		BufferLine(&Content, LineBuffer);
		u8 *Line = LineBuffer;
		BufferNextWord(&Line, Word);

		game_variable_data VariableData = GameStateVariableData(Word);
		u8 *VariablePtr = (u8 *)GameState + VariableData.Offset;
		switch(VariableData.Type)
		{
			case game_state_variable_type_u32:
			{
				ParseUnsignedInt(&Line, Word, (u32 *)VariablePtr);
			} break;
			case game_state_variable_type_s32:
			{
				ParseInt(&Line, Word, (s32 *)VariablePtr);
			} break;
			case game_state_variable_type_f32:
			{
				ParseFloat(&Line, Word, (f32 *)VariablePtr);
			} break;
			case game_state_variable_type_v3:
			{
				ParseV3(&Line, Word, (v3 *)VariablePtr);
			} break;
		}

		AdvanceLine(&Content);
	}

	GameState->Quad = QuadDefault();

	Platform.DebugFileFree(File.Content);
}

// TODO(Justin): Parse entity by type?
#if 1
internal void
LevelLoad(game_state *GameState, char *FileName)
{
	debug_file File = Platform.DebugFileReadEntire(FileName);
	//debug_file PlayerFile = Platform.DebugFileReadEntire("../src/players.variables");
	if(File.Size == 0)//k || PlayerFile.Size == 0)
	{
		Assert(0);
	}

	u8 *Content = (u8 *)File.Content;
	//u8 *PlayerContent = (u8 *)PlayerFile.Content;

	u8 LineBuffer_[4096];
	MemoryZero(LineBuffer_, sizeof(LineBuffer_));
	u8 *LineBuffer = &LineBuffer_[0];

	u8 Word_[4096];
	MemoryZero(Word_, sizeof(Word_));
	u8 *Word = &Word_[0];

	b32 ParsingEntity = false;
	entity *Current = GameState->Entities + GameState->EntityCount;
	while(*Content)
	{
		BufferLine(&Content, LineBuffer);
		u8 *Line = LineBuffer;
		BufferNextWord(&Line, Word);

		if(StringsAreSame(Word, "entity"))
		{
			Current->ID = GameState->EntityCount;
		}
		else if(StringsAreSame(Word, "type"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			if(StringsAreSame(Word, "null"))
			{
				Current->Type = EntityType_Null;
			}
			else if(StringsAreSame(Word, "walkable_region"))
			{
				Current->Type = EntityType_WalkableRegion;
			}
			else if(StringsAreSame(Word, "cube"))
			{
				Current->Type = EntityType_Cube;
			}
			else if(StringsAreSame(Word, "player"))
			{
				Current->Type = EntityType_Player;
			}
			else if(StringsAreSame(Word, "light"))
			{
				Current->Type = EntityType_Light;
				Current->Flags |= EntityFlag_Visible;
			}
		}
		else if(StringsAreSame(Word, "flags"))
		{
			ParseUnsignedInt(&Line, Word, &Current->Flags);
		}
		else if(StringsAreSame(Word, "position"))
		{
			ParseV3(&Line, Word, &Current->P);
		}
		else if(StringsAreSame(Word, "axis_angle"))
		{
			ParseQuaternion(&Line, Word, &Current->Orientation);
		}
		else if(StringsAreSame(Word, "movement_collider_count"))
		{
			ParseUnsignedInt(&Line, Word, &Current->MovementColliders.VolumeCount);
			Current->MovementColliders.Volumes = PushArray(&GameState->Arena, Current->MovementColliders.VolumeCount, collision_volume);
		}
		else if(StringsAreSame(Word, "movement_collider_type"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			if(StringsAreSame(Word, "obb"))
			{
				Current->MovementColliders.Volumes[0].Type = CollisionVolumeType_OBB; 
			}
		}
		else if(StringsAreSame(Word, "dim"))
		{
			v3 Dim = {};
			ParseV3(&Line, Word, &Dim);
			collision_volume *Volume = Current->MovementColliders.Volumes;
			OBBInitialize(Volume, Dim, Current->Orientation);
		}
		else if(StringsAreSame(Word, "visual_scale"))
		{
			// TODO(Justin): Check this.
			f32 Scale = 0.0f;
			ParseFloat(&Line, Word, &Scale);
			Current->VisualScale = Scale * Current->MovementColliders.Volumes[0].Dim;
		}
		else if(StringsAreSame(Word, "light_type"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			light *Light = GameState->Lights + Current->ID;
			if(StringsAreSame(Word, "directional"))
			{
				Light->Type = LightType_Directional;
			}
		}
		else if(StringsAreSame(Word, "direction"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseV3(&Line, Word, &Light->Dir);
		}
		else if(StringsAreSame(Word, "left"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Left);
		}
		else if(StringsAreSame(Word, "right"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Right);
		}
		else if(StringsAreSame(Word, "bottom"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Bottom);
		}
		else if(StringsAreSame(Word, "top"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Top);
		}
		else if(StringsAreSame(Word, "near"))
		{
			light *Light = GameState->Lights + Current->ID;
			Light->Near = GameState->ZNear;
		}
		else if(StringsAreSame(Word, "far"))
		{
			light *Light = GameState->Lights + Current->ID;
			Light->Far = GameState->ZFar;

			Light->Ortho = Mat4OrthographicProjection(Light->Left, Light->Right, Light->Bottom, Light->Top, Light->Near, Light->Far);
			Light->View = Mat4Camera(Current->P, Light->Dir);
		}
		else if(StringsAreSame(Word, "next"))
		{
			// TODO(Justin): Should we compute per entity final information here? E.g. the lights mat4's?
			GameState->EntityCount++;
			Current = GameState->Entities + GameState->EntityCount;
		}

		AdvanceLine(&Content);
	}

	Platform.DebugFileFree(File.Content);
	//Platform.DebugFileFree(PlayerFile.Content);
}
#else

internal entity_member_data
EntityMemberData(u8 *MemberName)
{
	entity_member_data Result = {};

	if(StringsAreSame(MemberName, "flags"))
	{
		Result.Type = entity_member_variable_type_u32;
		Result.Offset = OffsetOf(entity, Flags);
	}
	else if(StringsAreSame(MemberName, "position"))
	{
		Result.Type = entity_member_variable_type_v3;
		Result.Offset = OffsetOf(entity, P);
	}
	else if(StringsAreSame(MemberName, "axis_angle"))
	{
		Result.Type = entity_member_variable_type_quaternion;
		Result.Offset = OffsetOf(entity, Orientation);
	}
	else if(StringsAreSame(MemberName, "movement_collider_count"))
	{
		Result.Type = entity_member_variable_type_u32;
		Result.Offset = OffsetOf(entity, MovementColliders) + OffsetOf(collision_group, VolumeCount);
		Result.ArrayOffset = OffsetOf(entity, MovementColliders) + OffsetOf(collision_group, Volumes);
		Result.Initialize = true;
		Result.Size = sizeof(collision_volume);
	}
	else if(StringsAreSame(MemberName, "movement_collider_type"))
	{
		Result.Type = entity_member_variable_type_enum;
	}
	else if(StringsAreSame(MemberName, "dim"))
	{
		Result.Type = entity_member_variable_type_v3;
		Result.Offset = OffsetOf(entity, MovementColliders) + OffsetOf(collision_group, Volumes) + OffsetOf(collision_volume, Dim);
	}
	else if(StringsAreSame(MemberName, "dim"))
	{
		Result.Type = entity_member_variable_type_v3;
		Result.Offset = OffsetOf(entity, MovementColliders) + OffsetOf(collision_group, Volumes) + OffsetOf(collision_volume, Dim);
	}
	else if(StringsAreSame(MemberName, "visual_scale"))
	{
		Result.Type = entity_member_variable_type_f32;
		Result.Offset = 
	}

	return(Result);
}


internal void
LevelLoad(game_state *GameState, char *FileName)
{
	debug_file File = Platform.DebugFileReadEntire(FileName);
	//debug_file PlayerFile = Platform.DebugFileReadEntire("../src/players.variables");
	if(File.Size == 0)//k || PlayerFile.Size == 0)
	{
		Assert(0);
	}

	u8 *Content = (u8 *)File.Content;
	//u8 *PlayerContent = (u8 *)PlayerFile.Content;

	u8 LineBuffer_[4096];
	MemoryZero(LineBuffer_, sizeof(LineBuffer_));
	u8 *LineBuffer = &LineBuffer_[0];

	u8 Word_[4096];
	MemoryZero(Word_, sizeof(Word_));
	u8 *Word = &Word_[0];

	entity *Current = GameState->Entities + GameState->EntityCount;
	while(*Content)
	{
		BufferLine(&Content, LineBuffer);
		u8 *Line = LineBuffer;
		BufferNextWord(&Line, Word);

		if(StringsAreSame(Word, "entity"))
		{
			Current->ID = GameState->EntityCount;

			AdvanceLine(&Content);
			BufferLine(&Content, LineBuffer);
			Line = LineBuffer;
			BufferNextWord(&Line, Word);
			Assert(StringsAreSame(Word, "type"))

			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			if(StringsAreSame(Word, "null"))
			{
				Current->Type = EntityType_Null;
			}
			else if(StringsAreSame(Word, "walkable_region"))
			{
				Current->Type = EntityType_WalkableRegion;
			}
			else if(StringsAreSame(Word, "cube"))
			{
				Current->Type = EntityType_Cube;
			}
			else if(StringsAreSame(Word, "player"))
			{
				Current->Type = EntityType_Player;
			}
			else if(StringsAreSame(Word, "light"))
			{
				Current->Type = EntityType_Light;
			}

			// TODO(Justin): Figure out a way to handle this in the parsing code.. 
			Current->MovementColliders.Volumes = PushArray(&GameState->Arena, 1, collision_volume);
		}

		AdvanceLine(&Content);
		BufferLine(&Content, LineBuffer);
		Line = LineBuffer;
		BufferNextWord(&Line, Word);

		switch(Current->Type)
		{
			case EntityType_Null:
			{
				while(*Content && !StringsAreSame(Word, "next"))
				{
					AdvanceLine(&Content);
					BufferLine(&Content, LineBuffer);
					Line = LineBuffer;
					BufferNextWord(&Line, Word);
				}

				GameState->EntityCount++;
				Current = GameState->Entities + GameState->EntityCount;
			} break;
			case EntityType_Player:
			{
			} break;
			case EntityType_Cube:
			case EntityType_WalkableRegion:
			{
				while(*Content && !StringsAreSame(Word, "next"))
				{
					entity_member_data Data = EntityMemberData(Word);
					u8 *Ptr = (u8 *)Current + Data.Offset;

					switch(Data.Type)
					{
						case entity_member_variable_type_u32:
						{
							ParseUnsignedInt(&Line, Word, (u32 *)Ptr);
						} break;
						case entity_member_variable_type_f32:
						{
							ParseFloat(&Line, Word, (f32 *)Ptr);
						} break;
						case entity_member_variable_type_v3:
						{
							ParseV3(&Line, Word, (v3 *)Ptr);
						} break;
						case entity_member_variable_type_quaternion:
						{
							ParseQuaternion(&Line, Word, (quaternion *)Ptr);
						} break;
						case entity_member_variable_type_enum:
						{
						} break;
					}

					AdvanceLine(&Content);
					BufferLine(&Content, LineBuffer);
					Line = LineBuffer;
					BufferNextWord(&Line, Word);
				}

				Current->MovementColliders.Volumes[0].Type = CollisionVolumeType_OBB;

				GameState->EntityCount++;
				Current = GameState->Entities + GameState->EntityCount;
			} break;
			case EntityType_Light:
			{

			} break;
		}




#if 0
		else if(StringsAreSame(Word, "flags"))
		{
			ParseUnsignedInt(&Line, Word, &Current->Flags);
		}
		else if(StringsAreSame(Word, "position"))
		{
			ParseV3(&Line, Word, &Current->P);
		}
		else if(StringsAreSame(Word, "axis_angle"))
		{
			ParseQuaternion(&Line, Word, &Current->Orientation);
		}
		else if(StringsAreSame(Word, "movement_collider_count"))
		{
			ParseUnsignedInt(&Line, Word, &Current->MovementColliders.VolumeCount);
			Current->MovementColliders.Volumes = PushArray(&GameState->Arena, Current->MovementColliders.VolumeCount, collision_volume);
		}
		else if(StringsAreSame(Word, "movement_collider_type"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			if(StringsAreSame(Word, "obb"))
			{
				Current->MovementColliders.Volumes[0].Type = CollisionVolumeType_OBB; 
			}
		}
		else if(StringsAreSame(Word, "dim"))
		{
			v3 Dim = {};
			ParseV3(&Line, Word, &Dim);
			collision_volume *Volume = Current->MovementColliders.Volumes;
			OBBInitialize(Volume, Dim, Current->Orientation);
		}
		else if(StringsAreSame(Word, "visual_scale"))
		{
			// TODO(Justin): Check this.
			f32 Scale = 0.0f;
			ParseFloat(&Line, Word, &Scale);
			Current->VisualScale = Scale * Current->MovementColliders.Volumes[0].Dim;
		}
		else if(StringsAreSame(Word, "light_type"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			light *Light = GameState->Lights + Current->ID;
			if(StringsAreSame(Word, "directional"))
			{
				Light->Type = LightType_Directional;
			}
		}
		else if(StringsAreSame(Word, "direction"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseV3(&Line, Word, &Light->Dir);
		}
		else if(StringsAreSame(Word, "left"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Left);
		}
		else if(StringsAreSame(Word, "right"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Right);
		}
		else if(StringsAreSame(Word, "bottom"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Bottom);
		}
		else if(StringsAreSame(Word, "top"))
		{
			light *Light = GameState->Lights + Current->ID;
			ParseFloat(&Line, Word, &Light->Top);
		}
		else if(StringsAreSame(Word, "near"))
		{
			light *Light = GameState->Lights + Current->ID;
			Light->Near = GameState->ZNear;
		}
		else if(StringsAreSame(Word, "far"))
		{
			light *Light = GameState->Lights + Current->ID;
			Light->Far = GameState->ZFar;

			Light->Ortho = Mat4OrthographicProjection(Light->Left, Light->Right, Light->Bottom, Light->Top, Light->Near, Light->Far);
			Light->View = Mat4Camera(Current->P, Light->Dir);
		}
		else if(StringsAreSame(Word, "next"))
		{
			// TODO(Justin): Should we compute per entity final information here? E.g. the lights mat4's?
			GameState->EntityCount++;
			Current = GameState->Entities + GameState->EntityCount;
		}
#endif

		AdvanceLine(&Content);
	}

	Platform.DebugFileFree(File.Content);
	//Platform.DebugFileFree(PlayerFile.Content);
}
#endif

internal void
LevelReload(game_state *GameState)
{
	v3 PlayerP = GameState->Entities[GameState->PlayerEntityIndex].P;

	GameState->EntityCount = 0;
	GameState->PlayerEntityIndex = 0;
	MemoryZero(GameState->Entities, ArrayCount(GameState->Entities) * sizeof(entity));
	LevelLoad(GameState, "../src/test.level");

	entity *Player = XBotAdd(GameState, PlayerP);
	entity *YBot = YBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
	entity *Knight = KnightAdd(GameState, V3(0.0f, 0.01f, -5.0f));
	entity *Brute = BruteAdd(GameState, V3(0.0f, 0.01f, -5.0f));

	GameState->PlayerIDForController[1] = Player->ID;
	GameState->CharacterIDs[GameState->CurrentCharacter] = Player->ID;
	GameState->CharacterIDs[GameState->CurrentCharacter + 1] = YBot->ID;
	GameState->CharacterIDs[GameState->CurrentCharacter + 2] = Knight->ID;
	GameState->CharacterIDs[GameState->CurrentCharacter + 3] = Brute->ID;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert(sizeof(game_state) <= GameMemory->PermanentStorageSize);
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	Platform = GameMemory->PlatformAPI;
	if(!GameMemory->IsInitialized)
	{
		//
		// NOTE(Justin): Arena.
		//

		ArenaInitialize(&GameState->Arena, (u8 *)GameMemory->PermanentStorage + sizeof(game_state),
												 GameMemory->PermanentStorageSize - sizeof(game_state)); 

		//
		// NOTE(Justin): Assets.
		//

		//model TestModel = TestLoadModel(&GameState->Arena, "test/Beta_JointsMesh.mesh");
		//Platform.UploadAnimatedModelToGPU(&TestModel);

		ArenaSubset(&GameState->Arena, &GameState->AssetManager.Arena, Megabyte(16));
		AssetManagerInitialize(&GameState->AssetManager);
		asset_manager *Assets = &GameState->AssetManager;
		//Assets->TestModel = TestModel;


		GameStateVariablesLoad(GameState, "../src/all.variables");
		LevelLoad(GameState, "../src/test.level");

		CameraSet(&GameState->Camera, GameState->Camera.P + GameState->CameraOffsetFromPlayer, GameState->Camera.DefaultYaw, GameState->Camera.DefaultPitch);
		CameraTransformUpdate(GameState);
		GameState->TimeScale = 1.0f;
		GameState->Aspect = (f32)GameInput->BackBufferWidth / (f32)GameInput->BackBufferHeight;
		GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);

		GameMemory->IsInitialized = true;
	}

	Assert(sizeof(temp_state) <= GameMemory->TemporaryStorageSize);
	temp_state *TempState = (temp_state *)GameMemory->TemporaryStorage;
	if(!TempState->IsInitialized)
	{
		ArenaInitialize(&TempState->Arena,
				(u8 *)GameMemory->TemporaryStorage + sizeof(temp_state),
					  GameMemory->TemporaryStorageSize - sizeof(temp_state));

		TempState->IsInitialized = true;

	}

	//
	// NOTE(Justin): Input
	//

	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(GameInput->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = ControllerGet(GameInput, ControllerIndex);
		u32 ID = GameState->PlayerIDForController[ControllerIndex];
		if(ID == 0)
		{
			if(Controller->Space.EndedDown || Controller->Start.EndedDown)
			{
				entity *Player = XBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
				GameState->PlayerIDForController[ControllerIndex] = Player->ID;

				entity *YBot = YBotAdd(GameState, V3(-10.0f, 0.01f, -2.0f));
				entity *Knight = KnightAdd(GameState, V3(-5.0f, 0.01f, -2.0f));
				entity *Brute = BruteAdd(GameState, V3(5.0f, 0.01f, -2.0f));

				GameState->CurrentCharacter = 0;
				GameState->CharacterIDs[GameState->CurrentCharacter] = Player->ID;
				GameState->CharacterIDs[GameState->CurrentCharacter + 1] = YBot->ID;
				GameState->CharacterIDs[GameState->CurrentCharacter + 2] = Knight->ID;
				GameState->CharacterIDs[GameState->CurrentCharacter + 3] = Brute->ID;
			}
		}
		else
		{
			// Debug swap character by pressing start

#if 1
			if((WasPressed(Controller->Start) || WasPressed(Controller->Enter)))
			{

				u32 CurrentID = GameState->PlayerEntityIndex;
				entity *Current = GameState->Entities + CurrentID;

				GameState->CurrentCharacter++;
				if(GameState->CurrentCharacter > 3)
				{
					GameState->CurrentCharacter = 0;
				}

				CurrentID = GameState->CharacterIDs[GameState->CurrentCharacter];
				GameState->PlayerIDForController[ControllerIndex] = CurrentID;
				GameState->PlayerEntityIndex = CurrentID;
				Current = GameState->Entities + GameState->PlayerEntityIndex;
			}
#else
#endif
			entity *Entity = GameState->Entities + GameState->PlayerIDForController[ControllerIndex];
			move_info *MoveInfo = &Entity->MoveInfo;
			*MoveInfo = {};
			f32 AccelerationSq = 0.0f;
			b32 Sprinting = false;

#if DEVELOPER
			if(GameState->Camera.IsFree && !GameState->Camera.IsLocked) continue;
#endif

			if(Controller->IsAnalog)
			{
				MoveInfo->ddP = V3(Controller->StickAverageX, 0.0f, -1.0f*Controller->StickAverageY);
				MoveInfo->StickDelta = V2(Controller->StickdX, Controller->StickdY);

				if(WasDown(Controller->ActionRight) && IsGrounded(Entity))
				{
					MoveInfo->CanJump = true;
					MoveInfo->AnyAction = true;
				}

				if(WasDown(Controller->ActionDown))
				{
					MoveInfo->Attacking = true;
					MoveInfo->AnyAction = true;
				}
				
				AccelerationSq = Dot(MoveInfo->ddP, MoveInfo->ddP);
				if(AccelerationSq >= 1.0f)
				{
					Sprinting = true;
					MoveInfo->AnyAction = true;
				}
			}
			else
			{
				if(IsDown(Controller->MoveForward))
				{
					MoveInfo->ddP += V3(0.0f, 0.0f, -1.0f);
				}
				if(IsDown(Controller->MoveLeft))
				{
					MoveInfo->ddP += V3(-1.0f, 0.0f, 0.0f);
				}
				if(IsDown(Controller->MoveBack))
				{
					MoveInfo->ddP += V3(0.0f, 0.0f, 1.0f);
				}
				if(IsDown(Controller->MoveRight))
				{
					MoveInfo->ddP += V3(1.0f, 0.0f, 0.0f);
				}
				if(IsDown(Controller->Shift))
				{
					Sprinting = true;
					MoveInfo->AnyAction = true;
				}
				if(IsDown(Controller->Space) && IsGrounded(Entity))
				{
					MoveInfo->CanJump = true;
					MoveInfo->AnyAction = true;
				}
				if(IsDown(Controller->Ctrl) && IsGrounded(Entity))
				{
					MoveInfo->Crouching = true;
					MoveInfo->AnyAction = true;
				}
				if(WasPressed(GameInput->MouseButtons[MouseButton_Left]))
				{
					MoveInfo->Attacking = true;
					MoveInfo->AnyAction = true;
				}

				AccelerationSq = Dot(MoveInfo->ddP, MoveInfo->ddP);
			}

			if(AccelerationSq != 0.0f)
			{
				MoveInfo->ddP = Normalize(MoveInfo->ddP);
				MoveInfo->AnyAction = true;
				MoveInfo->Accelerating = true;
			}

			if(Sprinting && MoveInfo->Accelerating)
			{
				MoveInfo->CanSprint = true;
			}

			MoveInfo->Speed = Length(Entity->dP);
			if(MoveInfo->Speed < 0.1f)
			{
				MoveInfo->NoVelocity = true;
			}

			if(MoveInfo->NoVelocity && MoveInfo->Attacking)
			{
				MoveInfo->CanStrongAttack = true;
			}

			if(MoveInfo->NoVelocity && !MoveInfo->AnyAction)
			{
				MoveInfo->StandingStill = true;
			}
		}
	}

	//
	// NOTE(Justin): Simulate
	//

	f32 dt = GameInput->DtForFrame;
#if DEVELOPER
	dt *= GameState->TimeScale;
#endif
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				move_info MoveInfo = Entity->MoveInfo;
				switch(Entity->MovementState)
				{
					case MovementState_Idle:	EvaluateIdle(Entity, MoveInfo); break;
					case MovementState_Crouch:	EvaluateCrouch(Entity, MoveInfo); break;
					case MovementState_Run:		EvaluateRun(Entity, MoveInfo); break;
					case MovementState_Sprint:	EvaluateSprint(Entity, MoveInfo); break;
					case MovementState_Jump:	EvaluateJump(Entity, MoveInfo, dt); break;
					case MovementState_InAir:	EvaluateInAir(Entity, MoveInfo); break;
					case MovementState_Land:	EvaluateLand(Entity, MoveInfo); break;
					case MovementState_Sliding:	EvaluateSliding(Entity, MoveInfo); break;
					case MovementState_Attack:	EvaluateAttack(Entity, MoveInfo, dt); break;
				};
			} break;
			case EntityType_Light:
			{
			} break;
		}

		b32 Moved = false;
		if(!Equal(Entity->MoveInfo.ddP, V3(0.0f)) && CanMove(Entity))
		{
			if(IsMoving(Entity))
			{
				EntityMove(GameState, Entity, Entity->MoveInfo.ddP, dt);
				EntityOrientationUpdate(Entity, dt, Entity->AngularSpeed);
				Moved = true;
			}
		}

		if(Equal(Entity->MoveInfo.ddP, V3(0.0f)))
		{
			if(Entity->AnimationPlayer && Entity->AnimationPlayer->ControlsPosition)
			{
				EntityMove(GameState, Entity, Entity->MoveInfo.ddP, dt);
				Moved = true;
			}
		}

		if(!Moved && Entity->Flags & EntityFlag_AttackCollisionCheck)
		{
			EntityMove(GameState, Entity, Entity->MoveInfo.ddP, dt);
			Moved = true;
		}
	}

	asset_manager *Assets = &GameState->AssetManager;
	entity *Player = GameState->Entities + GameState->PlayerEntityIndex;
	camera *Camera = &GameState->Camera;

	if(!GameState->Camera.IsFree)
	{
		v3 CameraP = Player->P + GameState->CameraOffsetFromPlayer;
		CameraSet(Camera, CameraP, GameState->Camera.DefaultYaw, GameState->Camera.DefaultPitch);
	}

	//
	// NOTE(Justin): Render.
	//

	PerspectiveTransformUpdate(GameState, (f32)GameInput->BackBufferWidth, (f32)GameInput->BackBufferHeight);
	CameraTransformUpdate(GameState);

	temporary_memory AnimationMemory = TemporaryMemoryBegin(&TempState->Arena);
	temporary_memory RenderMemory = TemporaryMemoryBegin(&TempState->Arena);
	render_buffer *RenderBuffer = RenderBufferAllocate(&TempState->Arena, Megabyte(512),
														GameState->CameraTransform,
														GameState->Perspective,
														Assets,
														Camera->P);

	PushClear(RenderBuffer, V4(0.3f, 0.4f, 0.4f, 1.0f));

	mat4 T;
	mat4 R;
	mat4 S;
	asset_entry Entry = {};
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;

		if(!FlagIsSet(Entity, EntityFlag_Visible)) continue;

		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				Animate(Entity, Assets);
				AnimationPlayerUpdate(Entity->AnimationPlayer, &TempState->Arena, dt);
				animation_player *AnimationPlayer = Entity->AnimationPlayer;
				animation_graph *AnimationGraph = Entity->AnimationGraph;
				ModelJointsUpdate(Entity);
				AnimationGraphPerFrameUpdate(Assets, AnimationPlayer, AnimationGraph);

				T = Mat4Translate(Entity->P);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(Entity->VisualScale);

				if(AnimationPlayer->UpdateLockedP)
				{
					AnimationPlayer->EntityPLockedAt = Entity->P;
					AnimationPlayer->UpdateLockedP = false;
				}

				if(AnimationPlayer->ControlsPosition)
				{
					T = Mat4Translate(AnimationPlayer->EntityPLockedAt);
				}

				if(Entity->Flags & EntityFlag_Attacking)
				{
					if(AnimationPlayer->SpawnAttackCollider)
					{
						FlagAdd(Entity, EntityFlag_AttackCollisionCheck);
					}
				}
				else
				{
					FlagClear(Entity, EntityFlag_AttackCollisionCheck);
				}

				//ModelTPose(AnimationPlayer->Model);
				PushModel(RenderBuffer, CString(AnimationPlayer->Model->Name), T*R*S);
			} break;
			case EntityType_Cube:
			{
				v3 EntityP = Entity->P;
				EntityP.y += Entity->VisualScale.y;
				T = Mat4Translate(EntityP);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(Entity->VisualScale);
				PushModel(RenderBuffer, "Cube", T*R*S);
			} break;
			case EntityType_WalkableRegion:
			{
				v3 P = Entity->P;
				v3 Dim = Entity->MovementColliders.Volumes[0].Dim;
				T = Mat4Translate(P);
				R = Mat4Identity();
				S = Mat4Scale(Dim.x);

				quad GroundQuad = GameState->Quad;
				for(u32 Index = 0; Index < ArrayCount(GroundQuad.Vertices); ++Index)
				{
					GroundQuad.Vertices[Index].UV *= 0.5f*Dim.x;
				}

				Entry = FindTexture(Assets, "texture_01");
				PushTexture(RenderBuffer, Entry.Texture, Entry.Index);
				PushQuad3D(RenderBuffer, GroundQuad.Vertices, T*R*S, Entry.Index);

				T = Mat4Translate(Entity->P + Entity->MovementColliders.Volumes[0].Center);
				S = Mat4Scale(Dim);
				PushDebugVolume(RenderBuffer, &Assets->Cube, T*S, V3(1.0f));
			} break;
			case EntityType_Light:
			{
				light *Light = GameState->Lights + Entity->ID;
				Assert(Light->Type == LightType_Directional);
				RenderBuffer->LightDir = Light->Dir;
				RenderBuffer->LightTransform = Light->Ortho * Light->View;
			};
		};
	}

	//
	// NOTE(Justin): Asset reload
	//

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.LevelFileInfo))
	{
		LevelReload(GameState);
	}

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.XBotGraphFileInfo))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		AnimationGraphReload(&GameState->AssetManager, "XBot_AnimationGraph");
		animation_graph *G = FindGraph(&GameState->AssetManager, "XBot_AnimationGraph").Graph;
		Entity->AnimationGraph = G;
	}

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.YBotGraphFileInfo))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		AnimationGraphReload(&GameState->AssetManager, "YBot_AnimationGraph");
		animation_graph *G = FindGraph(&GameState->AssetManager, "YBot_AnimationGraph").Graph;
		Entity->AnimationGraph = G;
	}

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.PaladinGraphFileInfo))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		AnimationGraphReload(&GameState->AssetManager, "Paladin_AnimationGraph");
		animation_graph *G = FindGraph(&GameState->AssetManager, "Paladin_AnimationGraph").Graph;
		Entity->AnimationGraph = G;
	}

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.BruteGraphFileInfo))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		AnimationGraphReload(&GameState->AssetManager, "Brute_AnimationGraph");
		animation_graph *G = FindGraph(&GameState->AssetManager, "Brute_AnimationGraph").Graph;
		Entity->AnimationGraph = G;
	}

	//
	// NOTE(Justin): Debug ui.
	//

	temporary_memory UiMemory = TemporaryMemoryBegin(&TempState->Arena);
	UiBegin(RenderBuffer, UiMemory.Arena, GameInput, Assets);
	//UiBegin(RenderBuffer, &TempState->Arena, GameInput, Assets);

	game_controller_input *Keyboard = ControllerGet(GameInput, 0);
	if(IsDown(Keyboard->Add))
	{
		GameState->TimeScale *= 1.1f;
	}
	if(IsDown(Keyboard->Subtract))
	{
		GameState->TimeScale *= 0.9f;
	}

	if(WasPressed(Keyboard->F10))
	{
		GameState->Camera.IsFree = !GameState->Camera.IsFree;
	}

	if(WasPressed(Keyboard->F9))
	{
		GameState->Camera.IsLocked = !GameState->Camera.IsLocked;
	}

	if(GameState->Camera.IsFree && !GameState->Camera.IsLocked)
	{
		v3 CameraddP = {};
		if(IsDown(Keyboard->MoveForward))
		{
			CameraddP += 1.0f*GameState->Camera.Direction;
		}
		if(IsDown(Keyboard->MoveLeft))
		{
			CameraddP += -1.0f*Cross(GameState->Camera.Direction, YAxis());
		}
		if(IsDown(Keyboard->MoveBack))
		{
			CameraddP += -1.0f*GameState->Camera.Direction;
		}
		if(IsDown(Keyboard->MoveRight))
		{
			CameraddP += 1.0f*Cross(GameState->Camera.Direction, YAxis());
		}

		if(!Equal(CameraddP, V3(0.0f)))
		{
			CameraddP = Normalize(CameraddP);
		}

		v3 P = Camera->P + GameState->Camera.Speed*dt*CameraddP;
		Camera->P = P;

		if(IsDown(GameInput->MouseButtons[MouseButton_Left]))
		{
			CameraDirectionUpdate(Camera, GameInput->dXMouse, GameInput->dYMouse, dt);
		}
	}

	entity *Entity = GameState->Entities + GameState->PlayerEntityIndex;

	DebugDrawFloat("fps: ", GameInput->FPS);
	DebugDrawFloat("TimeScale: ", GameState->TimeScale);

	if(ToggleButton("HandAndFoot", DebugDrawHandAndFoot))
	{
		DebugDrawHandAndFoot(Entity);
	}

	if(ToggleButton("DrawGroundArrow", DebugDrawGroundArrow))
	{
		DebugDrawGroundArrow(Entity, GameState->Quad);
	}

	if(ToggleButton("DrawMovementCollider", DebugDrawCapsule))
	{
		capsule Capsule = CapsuleGet(&Entity->MovementColliders);
		DebugDrawCapsule(Entity->P, CapsuleCenter(Capsule), QuaternionToMat4(Entity->Orientation), Mat4Scale(1.0f), Capsule);
	}

	if(ToggleButton("DrawCombatCollider", DebugDrawCollisionVolumes))
	{
		DebugDrawCollisionVolumes(Entity);
	}

	if(ToggleButton("DrawJoints", DebugDrawJoints))
	{
		DebugDrawJoints(Entity);
	}

	if(ToggleButton("DrawMeshTPoseAABB", DebugDrawMeshTPoseAABB))
	{
		DebugDrawMeshTPoseAABB(Entity);
	}

	if(ToggleButton("DrawWeaponCollider", DebugDrawWeaponCollider))
	{
		UiIndentAdd();
		if(ToggleButton("TPose", ModelTPose))
		{
			ModelTPose(Entity->AnimationPlayer->Model);
		}
		UiIndentRemove();

		DebugDrawWeaponCollider(Entity);
	}

	if(ToggleButton("+Player", DebugDrawEntity))
	{
		DebugDrawEntity(Entity);
	}

	if(ToggleButton("+AnimationPlayer", DebugDrawAnimationPlayer))
	{
		DebugDrawAnimationPlayer(Entity->AnimationPlayer);
		if(Entity->AnimationPlayer)
		{
			if(Entity->AnimationPlayer->SpawnAttackCollider)
			{
				DebugDrawWeaponCollider(Entity);
			}
		}
	}

	if(ToggleButton("+ShadowMap", DebugRenderToTexture))
	{
		DebugRenderToTexture(GameState);
	}

	if(ToggleButton("TextureView", DebugDrawTexture))
	{
		DebugDrawTexture("texture_01", 256.0f, 256.0f);
	}

	Platform.RenderToOpenGL(RenderBuffer, (u32)GameInput->BackBufferWidth, (u32)GameInput->BackBufferHeight);

	TemporaryMemoryEnd(UiMemory);
	TemporaryMemoryEnd(RenderMemory);
	TemporaryMemoryEnd(AnimationMemory);
}

extern "C" GAME_AUDIO_UPDATE(GameAudioUpdate)
{
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	temp_state *TempState = (temp_state *)GameMemory->TemporaryStorage;
	SoundToOutput(&GameState->AudioState, SoundBuffer, &GameState->AssetManager, &TempState->Arena);
}

