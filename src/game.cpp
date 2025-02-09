#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "font.cpp"
#include "mesh.cpp"
#include "entity.cpp"
#include "animation.cpp"
#include "asset.cpp"
#include "render.cpp"
#include "debug.cpp"
#include "ui.cpp"
#include "collision.cpp"

internal entity * 
PlayerAdd(game_state *GameState, v3 P)
{
	entity *Entity = EntityAdd(GameState, EntityType_Player);

	FlagAdd(Entity, EntityFlag_YSupported);
	FlagAdd(Entity, EntityFlag_Collides);
	FlagAdd(Entity, EntityFlag_Moveable);

	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);

	f32 Radians = DirectionToEuler(V3(0.0f, 0.0f, -1.0f)).yaw;
	Entity->Theta = RadToDegrees(Radians);
	Entity->ThetaTarget = Entity->Theta; 
	Entity->dTheta = 0.0f;
	Entity->Orientation = Quaternion(V3(0.0f, -1.0f, 0.0f), Radians);
	Entity->MovementState = MovementState_Idle;

	// TODO(Justin): These fields need to be set differently depending on
	// the player type.
	Entity->Height = 1.8f;

	Entity->MovementColliders.Type = CollisionVolumeType_Capsule;
	Entity->MovementColliders.VolumeCount = 1;
	Entity->MovementColliders.Volumes = PushArray(&GameState->Arena, 1, collision_volume);

	collision_group *MovementColliders = &Entity->MovementColliders;
	collision_volume *Volume = MovementColliders->Volumes;

	capsule C;
	C.Radius = 0.4f;
	C.Min = V3(0.0f, C.Radius, 0.0f);
	C.Max = V3(0.0f, Entity->Height - C.Radius, 0.0f);
	Volume->Capsule = C;
#if 0
	// AABB
	Entity->AABBDim = V3(0.7f, Entity->Height, 0.7f);
	Entity->VolumeOffset = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);

	// OBB
	quaternion Q = Conjugate(Entity->Orientation);
	Entity->OBB.Center = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);
	Entity->OBB.X = Q * XAxis();
	Entity->OBB.Y = Q * YAxis();
	Entity->OBB.Z = Q * (-1.0f*ZAxis());
	Entity->OBB.Dim = Entity->AABBDim;

	// Sphere
	Entity->Radius = 0.4f;

	// Capsule
	capsule C;
	C.Radius = Entity->Radius;
	C.Min = V3(0.0f, C.Radius, 0.0f);
	C.Max = V3(0.0f, Entity->Height - C.Radius, 0.0f);
	Entity->Capsule = C;
#endif

	// Visuals
	//Entity->VisualScale = V3(0.01f);

	if(GameState->PlayerEntityIndex == 0)
	{
		GameState->PlayerEntityIndex = Entity->ID;
	}

	return(Entity);
}

internal entity * 
XBotAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P);
	Player->Acceleration = 50.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 15.0f;
	Player->VisualScale = V3(0.01f);

	model *Model		= LookupModel(&GameState->AssetManager, "XBot").Model;
	animation_graph *G  = LookupGraph(&GameState->AssetManager, "XBot_AnimationGraph");
	asset_entry Entry	= LookupSampledAnimation(&GameState->AssetManager, "XBot_IdleLeft");

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlags_Looping, 0.2f);

	return(Player);
}

internal entity *
YBotAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P);
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

#if 0
	Player->Attacks[AttackType_Neutral].Type = AttackType_Neutral;
	Player->Attacks[AttackType_Neutral].Duration = 0.2f;
	Player->Attacks[AttackType_Neutral].Power = 0.07f;

	Player->Attacks[AttackType_Forward].Type = AttackType_Forward;
	Player->Attacks[AttackType_Forward].Duration = 0.5f;
	Player->Attacks[AttackType_Forward].Power = 0.12f;
#endif

	model *Model = LookupModel(&GameState->AssetManager, "YBot").Model;
	animation_graph *G  = LookupGraph(&GameState->AssetManager, "YBot_AnimationGraph");
	asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "YBot_FightIdleRight");

	Assert(Model);
	Assert(G);
	Assert(Entry.SampledAnimation);

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlags_Looping, 0.2f);

	return(Player);
}

internal entity * 
KnightAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P);
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

	Player->Attacks[AttackType_Strong].Type = AttackType_Strong;
	Player->Attacks[AttackType_Strong].Duration = 1.0f;
	Player->Attacks[AttackType_Strong].Power = 0.18f;

	model *Model = LookupModel(&GameState->AssetManager, "PaladinWithProp").Model;
	animation_graph *G  = LookupGraph(&GameState->AssetManager, "Paladin_AnimationGraph");
	asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "Paladin_SwordAndShieldIdle_00");

	Assert(Model);
	Assert(G);
	Assert(Entry.SampledAnimation);

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlags_Looping, 0.2f);

	return(Player);
}

internal entity * 
VampireAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P);
	Player->Acceleration = 50.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 15.0f;

	model *Model = LookupModel(&GameState->AssetManager, "VampireALusth").Model;
	animation_graph *G  = LookupGraph(&GameState->AssetManager, "Vampire_AnimationGraph");
	asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "Vampire_IdleLeft");

	Assert(Model);
	Assert(G);
	Assert(Entry.SampledAnimation);

	Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
	Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);

	AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
	Player->AnimationGraph = G;
	AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlags_Looping, 0.2f);

	return(Player);
}

internal void
CubeAdd(game_state *GameState, v3 P, v3 Dim, quaternion Orientation)
{
	entity *Entity = EntityAdd(GameState, EntityType_Cube);

	FlagAdd(Entity, EntityFlag_YSupported);
	FlagAdd(Entity, EntityFlag_Collides);

	Entity->P = P;
	Entity->dP = {};
	Entity->ddP = {};
	Entity->Orientation = Orientation;

	collision_group *MovementColliders = &Entity->MovementColliders;
	MovementColliders->Type = CollisionVolumeType_OBB;
	MovementColliders->VolumeCount = 1;
	MovementColliders->Volumes = PushArray(&GameState->Arena, 1, collision_volume);

	collision_volume *Volume = MovementColliders->Volumes;
	quaternion Q = Conjugate(Entity->Orientation);
	Volume->OBB.Dim = 0.99f*Dim;
	Volume->OBB.Center  = 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
	Volume->Offset		= 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
	Volume->OBB.X = Q * XAxis();
	Volume->OBB.Y = Q * YAxis();
	Volume->OBB.Z = Q * (-1.0f*ZAxis());

	Entity->VisualScale = 0.5f*Dim;
}

internal void
WalkableRegionAdd(game_state *GameState, v3 P, v3 Dim, quaternion Orientation)
{
	entity *Entity = EntityAdd(GameState, EntityType_WalkableRegion);

	FlagAdd(Entity, EntityFlag_YSupported);
	FlagAdd(Entity, EntityFlag_Collides);

	Entity->P = P;
	Entity->dP = {};
	Entity->ddP = {};
	Entity->Orientation = Orientation;

	collision_group *MovementColliders = &Entity->MovementColliders;
	MovementColliders->Type = CollisionVolumeType_OBB;
	MovementColliders->VolumeCount = 1;
	MovementColliders->Volumes = PushArray(&GameState->Arena, 1, collision_volume);

	collision_volume *Volume = MovementColliders->Volumes;
	quaternion Q = Conjugate(Entity->Orientation);
	Volume->OBB.Dim = 0.99f*Dim;
	Volume->OBB.Center  = 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
	Volume->Offset		= 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
	Volume->OBB.X = Q * XAxis();
	Volume->OBB.Y = Q * YAxis();
	Volume->OBB.Z = Q * (-1.0f*ZAxis());

	Entity->VisualScale = 0.5f*Dim;
}

internal void
ElevatorAdd(game_state *GameState, v3 P, v3 Dim, quaternion Orientation)
{
	entity *Entity = EntityAdd(GameState, EntityType_Elevator);

	FlagAdd(Entity, EntityFlag_Moveable);
	FlagAdd(Entity, EntityFlag_Moving);
	FlagAdd(Entity, EntityFlag_Collides);

	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Orientation = Orientation;

	collision_group *MovementColliders = &Entity->MovementColliders;
	MovementColliders->Type = CollisionVolumeType_OBB;
	MovementColliders->VolumeCount = 1;
	MovementColliders->Volumes = PushArray(&GameState->Arena, 1, collision_volume);

	collision_volume *Volume = MovementColliders->Volumes;

	quaternion Q = Conjugate(Entity->Orientation);
	Volume->OBB.Dim = 0.99f*Dim;
	Volume->OBB.Center = 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
	Volume->OBB.X = Q * XAxis();
	Volume->OBB.Y = Q * YAxis();
	Volume->OBB.Z = Q * (-1.0f*ZAxis());

	Entity->VisualScale = 0.5f*Dim;
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
	GameState->CameraTransform = Mat4Camera(GameState->Camera.P,
											GameState->Camera.P + GameState->Camera.Direction);
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

internal void
CollisionGroupUpdate(entity *Entity)
{
	v3 P = Entity->P;

	collision_group *Group = &Entity->CombatColliders;
	Group->Volumes[0].Offset = Entity->LeftHandP - P;
	Group->Volumes[1].Offset = Entity->RightHandP - P;
	Group->Volumes[2].Offset = Entity->LeftFootP - P;
	Group->Volumes[3].Offset = Entity->RightFootP - P;
}

#if 1
internal void
CheckForAttackCollision(game_state *GameState, entity *Entity)
{
	if(!IsAttacking(Entity) || Entity->Type != EntityType_Player)
	{
		return;
	}

	for(u32 TestEntityIndex = 0; TestEntityIndex < GameState->EntityCount; ++TestEntityIndex)
	{
		b32 AttackCollided = false;
		entity *AttackedEntity = 0;

		entity *TestEntity = GameState->Entities + TestEntityIndex;
		if(Entity->ID != TestEntity->ID)
		{
			if(TestEntity->Type == EntityType_Player)
			{
				if(Entity->AttackType != AttackType_None)
				{
					CollisionGroupUpdate(Entity);
					collision_group *Group = &Entity->CombatColliders;
					for(u32 VolumeIndex = 0; VolumeIndex < Group->VolumeCount; ++VolumeIndex)
					{
						collision_volume *Volume = Group->Volumes + VolumeIndex;
						v3 A = Entity->P + Volume->Offset;
						v3 B = TestEntity->P;
						if(SpheresIntersect(A, 0.5f, B, 0.5f))
						{
							AttackCollided = true;
							AttackedEntity = TestEntity;
						}
					}
				}
			}
		}

		if(AttackCollided)
		{
			FlagAdd(AttackedEntity, EntityFlag_Attacked);
			AttackedEntity->P += Normalize(Entity->dP);
			break;
		}
	}
}
#endif

internal void
EntityMove(game_state *GameState, entity *Entity, v3 ddP, f32 dt)
{
	if(!FlagIsSet(Entity, EntityFlag_YSupported))
	{
		ddP += V3(0.0f, -GameState->Gravity, 0.0f);
	}

	v3 DeltaP = 0.5f * dt * dt * ddP + dt * Entity->dP;
	Entity->dP = dt * ddP + Entity->dP;

	v3 OldP = Entity->P;
	v3 DesiredP = OldP + DeltaP;

	animation_player *AnimationPlayer = Entity->AnimationPlayer;
	if(AnimationPlayer && AnimationPlayer->ControlsPosition)
	{
		// TODO(Justin): Robustness
		// TODO(Justin): Update velocity 
		v3 AnimationDelta = AnimationPlayer->RootMotionAccumulator;
		v3 GameDelta = 2.0f*V3(Entity->VisualScale.x * AnimationDelta.x,
				Entity->VisualScale.y * AnimationDelta.y,
				Entity->VisualScale.z * AnimationDelta.z);

		DeltaP = Conjugate(Entity->Orientation)*GameDelta;
		DesiredP = OldP + DeltaP;
		AnimationPlayer->RootMotionAccumulator = {};
	}

	// TODO(Justin): Figure out the "correct" length here. Or have delta normalized instead of t normalized.
	v3 GroundP = {};
	v3 GroundDelta = V3(0.0f, -20.0f,0.0f);
	entity *EntityBelow = 0;
	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		b32 Collided = false;
		b32 AttackCollided = false;

		f32 tMin = 1.0f;
		f32 tGround = 1.0f;

		v3 Normal = {};
		v3 GroundNormal = {};

		entity *HitEntity = 0;
		entity *AttackedEntity = 0;
		entity *Region = 0;

		DesiredP = Entity->P + DeltaP;
		for(u32 TestEntityIndex = 0; TestEntityIndex < GameState->EntityCount; ++TestEntityIndex)
		{
			entity *TestEntity = GameState->Entities + TestEntityIndex;
			collision_type CollisionType = CollisionType_None;

			// TODO(Justin): Early out sphere intersection test.
			if(EntitiesCanCollide(GameState, Entity, TestEntity, &CollisionType))
			{
				v3 TestP = TestEntity->P;
				v3 CurrentP = Entity->P; 

				collision_volume Volume		= Entity->MovementColliders.Volumes[0];
				collision_volume TestVolume = TestEntity->MovementColliders.Volumes[0];

				// TODO(Justin): Make sure the chosen sphere center is correct.
				v3 SphereCenter = CapsuleSphereCenterVsOBB(CurrentP, Volume.Capsule, TestEntity->P, TestVolume.OBB);
				v3 SphereRel = SphereCenter - (TestP + TestVolume.Offset);
				if(MovingSphereHitOBB(SphereRel, Volume.Capsule.Radius, DeltaP, TestVolume.OBB, &Normal, &tMin))
				{
					Collided = true;
					HitEntity = TestEntity;
				}

				// GroundCheck
				v3 RelP		= (CurrentP + V3(0.0f, Volume.Capsule.Radius, 0.0f)) - (TestP + TestVolume.Offset);
				if(MovingSphereHitOBB(RelP, Volume.Capsule.Radius, GroundDelta, TestVolume.OBB, &GroundNormal, &tGround))
				{
					EntityBelow = TestEntity;
				}
			}

			// Combat collisions
			if(Entity->ID != TestEntity->ID)
			{
				if(Entity->Type == EntityType_Player && TestEntity->Type == EntityType_Player)
				{
					if(Entity->AttackType != AttackType_None)
					{
						CollisionGroupUpdate(Entity);
						collision_group *Group = &Entity->CombatColliders;
						for(u32 VolumeIndex = 0; VolumeIndex < Group->VolumeCount; ++VolumeIndex)
						{
							collision_volume *Volume = Group->Volumes + VolumeIndex;
							v3 A = Entity->P + Volume->Offset;
							v3 B = TestEntity->P;
							if(SpheresIntersect(A, 0.5f, B, 0.5f))
							{
								AttackCollided = true;
								AttackedEntity = TestEntity;
							}
						}
					}
				}
			}

			if(TestEntity->Type == EntityType_WalkableRegion)
			{
				if(InAABB(AABBCenterDim(TestEntity->P, TestEntity->MovementColliders.Volumes[0].OBB.Dim), Entity->P))
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
			Entity->dP = {};
			break;
		}

		if(AttackCollided)
		{
			FlagAdd(AttackedEntity, EntityFlag_Attacked);
			AttackedEntity->P += Normalize(Entity->dP);
		}

		Entity->P += tMin * DeltaP; 
		GroundP = Entity->P;
		GroundP += tGround * GroundDelta;

		if(Collided)
		{
			Entity->dP = Entity->dP - Dot(Normal, Entity->dP) * Normal;
			DeltaP = DesiredP - Entity->P;
			DeltaP = DeltaP - Dot(Normal, DeltaP) * Normal;
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
		if(Entity->Type != EntityType_Elevator)
		{
			FlagClear(Entity, EntityFlag_YSupported);
		}
	}
	else
	{
		FlagAdd(Entity, EntityFlag_YSupported);
	}
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

		memory_arena *Arena = &GameState->Arena;

		//
		// NOTE(Justin): Assets.
		//

		ArenaSubset(&GameState->Arena, &GameState->AssetManager.Arena, Kilobyte(1024));
		AssetManagerInitialize(&GameState->AssetManager);
		asset_manager *Assets = &GameState->AssetManager;

		GameState->Quad = QuadDefault();
		GameState->Texture.Width = 256;
		GameState->Texture.Height = 256;

		EntityAdd(GameState, EntityType_Null);

		quaternion CubeOrientation = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);

		// Region
		WalkableRegionAdd(GameState, V3(0.0f, 0.0f, -20.0f), V3(40.0f), CubeOrientation);

		// Cubes 
		v3 StartP = V3(-3.0f, 0.0f, -10.0f);
		v3 Dim = V3(2.0f, 1.0f, 10.0f);

		CubeAdd(GameState, StartP, Dim, CubeOrientation);

		Dim = V3(2.0f, 10.0f, 2.0f);
		StartP += V3(6.0f, 0.0f, -0.0f);
		CubeAdd(GameState, StartP, Dim, CubeOrientation);

		Dim = V3(2.0f, 0.5f, 2.0f);
		v3 ElevatorP = StartP + V3(0.0f, 1.0f, 2.0f);
		//ElevatorAdd(GameState, ElevatorP, Dim, CubeOrientation);

		Dim = V3(1.f, 1.0f, 5.0f);
		StartP += V3(3.0f, 0.0f, -5.0f);
		CubeAdd(GameState, StartP, Dim, Quaternion(V3(0.0f, 1.0f, 0.0f), DegreeToRad(-30.0f)));

		StartP = V3(-7.0f, 1.0f, -10.0f);
		Dim = V3(2.0f, 1.0f, 15.0f);
		CubeOrientation = Quaternion(V3(1.0f, 0.0f, 0.0f), DegreeToRad(-30.0f));
		CubeAdd(GameState, StartP, Dim, CubeOrientation);

		StartP = V3(-7.0f, 0.0f, -18.0f);
		Dim = V3(2.0f, 5.0f, 4.0f);
		CubeAdd(GameState, StartP, Dim, Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f));

		GameState->CameraSpeed = 10.0f;
		GameState->DefaultYaw = -90.0f;
		GameState->DefaultPitch = -10.0f;
		GameState->CameraOffsetFromPlayer = V3(0.0f, 2.0f, 5.0f);
		CameraSet(&GameState->Camera, V3(0.0f, 0.0f, -10.0f) + GameState->CameraOffsetFromPlayer, -90.0f, -10.0f);
		CameraTransformUpdate(GameState);

		GameState->TimeScale = 1.0f;
		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)GameInput->BackBufferWidth / (f32)GameInput->BackBufferHeight;
		GameState->ZNear = 0.2f;
		GameState->ZFar = 200.0f;
		GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);
		GameState->Gravity = 80.0f;

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

#if DEVELOPER
	if(!GameInput->ReloadingGame && Platform.DebugFileIsDirty(GameState->AssetManager.XBotGraphFileInfo.Path, &GameState->AssetManager.XBotGraphFileInfo.FileDate))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		animation_graph *G = LookupGraph(&GameState->AssetManager, "XBot_AnimationGraph");
		ArenaClear(&G->Arena);
		G->NodeCount = 0;
		G->Index = 0;
		G->CurrentNode = {};
		MemoryZero(&G->Nodes, sizeof(G->Nodes));
		AnimationGraphInitialize(G, "../src/XBot_AnimationGraph.animation_graph");
		Entity->AnimationGraph = G;
	}

	if(!GameInput->ReloadingGame && Platform.DebugFileIsDirty(GameState->AssetManager.YBotGraphFileInfo.Path, &GameState->AssetManager.YBotGraphFileInfo.FileDate))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		animation_graph *G = LookupGraph(&GameState->AssetManager, "YBot_AnimationGraph");
		ArenaClear(&G->Arena);
		G->NodeCount = 0;
		G->Index = 0;
		G->CurrentNode = {};
		MemoryZero(&G->Nodes, sizeof(G->Nodes));
		AnimationGraphInitialize(G, "../src/YBot_AnimationGraph.animation_graph");
		Entity->AnimationGraph = G;
	}

	if(!GameInput->ReloadingGame && Platform.DebugFileIsDirty(GameState->AssetManager.PaladinGraphFileInfo.Path, &GameState->AssetManager.PaladinGraphFileInfo.FileDate))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		animation_graph *G = LookupGraph(&GameState->AssetManager, "Paladin_AnimationGraph");
		ArenaClear(&G->Arena);
		G->NodeCount = 0;
		G->Index = 0;
		G->CurrentNode = {};
		MemoryZero(&G->Nodes, sizeof(G->Nodes));
		AnimationGraphInitialize(G, "../src/Paladin_AnimationGraph.animation_graph");
		Entity->AnimationGraph = G;
	}
#endif

	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(GameInput->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = ControllerGet(GameInput, ControllerIndex);
		u32 ID = GameState->PlayerIDForController[ControllerIndex];
		if(ID == 0)
		{
			entity *Player = 0;

			if(ControllerIndex == 0 && Controller->Space.EndedDown)
			{
				//Player = YBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
				Player = XBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
			}

			if(ControllerIndex == 1 && Controller->Start.EndedDown)
			{
				Player = KnightAdd(GameState, V3(0.0f, 0.01f, -5.0f));
				//Player = YBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
			}


			if(Player)
			{
				GameState->PlayerIDForController[ControllerIndex] = Player->ID;
			}
		}
		else
		{
			entity *Entity = GameState->Entities + GameState->PlayerIDForController[ControllerIndex];
			move_info *MoveInfo = &Entity->MoveInfo;
			*MoveInfo = {};
			f32 AccelerationSq = 0.0f;
			b32 Sprinting = false;

			// NOTE(Justin): For debug camera
			if(GameState->CameraIsFree && !GameState->CameraIsLocked) continue;

			if(Controller->IsAnalog)
			{
				MoveInfo->ddP = V3(Controller->StickAverageX, 0.0f, -1.0f*Controller->StickAverageY);

				if(IsDown(Controller->ActionRight) && IsGrounded(Entity))
				{
					MoveInfo->CanJump = true;
					MoveInfo->AnyAction = true;
				}

				if(IsDown(Controller->ActionDown))
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

			if(MoveInfo->NoVelocity && !MoveInfo->AnyAction)
			{
				MoveInfo->StandingStill = true;
			}
		}
	}

	f32 dt = GameInput->DtForFrame;
#if DEVELOPER
	dt *= GameState->TimeScale;
#endif
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		v3 ddP = {};
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				move_info MoveInfo = Entity->MoveInfo;
				EvaluatePlayerMove(Entity, MoveInfo, &ddP);
				switch(Entity->MovementState)
				{
					case MovementState_Idle:	EvaluateIdle(Entity, MoveInfo); break;
					case MovementState_Crouch:	EvaluateCrouch(Entity, MoveInfo); break;
					case MovementState_Run:		EvaluateRun(Entity, MoveInfo); break;
					case MovementState_Sprint:	EvaluateSprint(Entity, MoveInfo); break;
					case MovementState_Jump:	EvaluateJump(Entity, MoveInfo); break;
					case MovementState_InAir:	EvaluateInAir(Entity, MoveInfo); break;
					case MovementState_Land:	EvaluateLand(Entity, MoveInfo); break;
					case MovementState_Sliding:	EvaluateSliding(Entity, MoveInfo); break;
					case MovementState_Attack:	EvaluateAttack(Entity, MoveInfo, dt); break;
				};
			} break;
			case EntityType_Elevator:
			{
				f32 MaxY = 4.0f;
				v3 ElevatorddP = {};

				if(Entity->P.y >= 1.0f && Entity->P.y < MaxY)
				{
					ElevatorddP = {0.0f, 1.0f, 0.0f};
				}
				if(Entity->P.y > MaxY)
				{
					ElevatorddP = {0.0f, -1.0f, 0.0f};
				}
				if(Entity->P.y >= 0.0f && Entity->P.y < 1.0f)
				{
					ElevatorddP = {0.0f, 1.0f, 0.0f};
				}
				ddP = ElevatorddP;
			} break;
		}

		if(!Equal(ddP, V3(0.0f)) && CanMove(Entity))
		{
			if(IsMoving(Entity))
			{
				EntityMove(GameState, Entity, ddP, dt);
				EntityOrientationUpdate(Entity, dt, Entity->AngularSpeed);
			}
		}
		else
		{
			CheckForAttackCollision(GameState, Entity);
		}
	}

	asset_manager *Assets = &GameState->AssetManager;
	entity *Player = GameState->Entities + GameState->PlayerEntityIndex;
	camera *Camera = &GameState->Camera;

	if(!GameState->CameraIsFree)
	{
		v3 CameraP = Player->P + GameState->CameraOffsetFromPlayer;
		CameraSet(Camera, CameraP, GameState->DefaultYaw, GameState->DefaultPitch);
	}

	//
	// NOTE(Justin): Render.
	//

	v3 LightDir = V3(1.0f, -1.0f, -1.0f);
	v3 LightP = V3(-10.0f, 10.0f, 0.0f);

	f32 Left = -30.0f;
	f32 Right = 30.0f;
	f32 Bottom = -20.0f;
	f32 Top = 20.0f;
	f32 Near = GameState->ZNear;
	f32 Far = GameState->ZFar;

	mat4 LightOrtho = Mat4OrthographicProjection(Left, Right, Bottom, Top, Near, Far);
	mat4 LightView = Mat4Camera(LightP, LightDir);
	mat4 LightTransform = LightOrtho * LightView;

	PerspectiveTransformUpdate(GameState, (f32)GameInput->BackBufferWidth, (f32)GameInput->BackBufferHeight);
	CameraTransformUpdate(GameState);

	temporary_memory AnimationMemory = TemporaryMemoryBegin(&TempState->Arena);
	temporary_memory RenderMemory = TemporaryMemoryBegin(&TempState->Arena);
	render_buffer *RenderBuffer = RenderBufferAllocate(&TempState->Arena, Megabyte(512),
														GameState->CameraTransform,
														GameState->Perspective,
														LightTransform,
														Assets,
														Camera->P,
														LightDir);

	UiBegin(RenderBuffer, &TempState->Arena, GameInput, Assets);

	PushClear(RenderBuffer, V4(0.3f, 0.4f, 0.4f, 1.0f));

	mat4 T;
	mat4 R;
	mat4 S;
	asset_entry Entry = {};
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
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

				f32 TurnFactor = AngleBetween(AnimationPlayer->OrientationLockedAt, Entity->Orientation);

				if(!AnimationPlayer->ControlsTurning && (AnimationPlayer->RootTurningAccumulator == 0.0f))
				{
					// Record gameplay orientation until animation player controls turning 
					AnimationPlayer->OrientationLockedAt = Entity->Orientation;
				}

				if(AnimationPlayer->ControlsTurning)
				{
					// Controls turning animation is playing and not blending out
					R = QuaternionToMat4(AnimationPlayer->OrientationLockedAt);

				}
				else
				{
					// TODO(Justin): Find a way to choose correct turning speed.

					// Catch up the locked orientation to current orientation.


					if(AnimationPlayer->RootTurningAccumulator != 0.0f)
					{
						f32 TurnSpeed = 4.5f;

						AnimationPlayer->OrientationLockedAt = RotateTowards(AnimationPlayer->OrientationLockedAt, Entity->Orientation, dt, TurnSpeed);
						R = QuaternionToMat4(AnimationPlayer->OrientationLockedAt);
						AnimationPlayer->RootTurningAccumulator = 0.0f;
					}
				}

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

				//
				// Axes.
				//

#if 0
				S = Mat4Scale(0.5f);
				model *ZArrow = LookupModel(Assets, "Arrow");
				model *XArrow = LookupModel(Assets, "XArrow");
				model *YArrow = LookupModel(Assets, "YArrow");

				T = Mat4Translate(Entity->P + Entity->VolumeOffset + 0.5f*X);
				PushAABB(RenderBuffer, XArrow, T*R*S, V3(1.0f, 0.0f, 0.0f));
				T = Mat4Translate(Entity->P + Entity->VolumeOffset + 0.5f*Y);
				PushAABB(RenderBuffer, YArrow, T*R*S, V3(0.0f, 1.0f, 0.0f));
				T = Mat4Translate(Entity->P + Entity->VolumeOffset + 0.5f*Z);
				PushAABB(RenderBuffer, ZArrow, T*R*S, V3(0.0f, 0.0f, 1.0f));
#endif
			} break;
			case EntityType_WalkableRegion:
			{
				quad GroundQuad = GameState->Quad;
				Entry = LookupTexture(Assets, "texture_01");

				v3 P = Entity->P;
				v3 Dim = Entity->MovementColliders.Volumes[0].OBB.Dim;
				T = Mat4Translate(P);
				R = Mat4Identity();
				S = Mat4Scale(Dim.x);

				for(u32 Index = 0; Index < ArrayCount(GroundQuad.Vertices); ++Index)
				{
					GroundQuad.Vertices[Index].UV *= 0.5f*Dim.x;
				}

				PushTexture(RenderBuffer, Entry.Texture, Entry.Index);
				PushQuad3D(RenderBuffer, GroundQuad.Vertices, T*R*S, Entry.Index);

				T = Mat4Translate(Entity->P + Entity->MovementColliders.Volumes[0].OBB.Center);
				S = Mat4Scale(Dim);
				PushDebugVolume(RenderBuffer, &Assets->Cube, T*S, V3(1.0f));
			} break;
			case EntityType_Elevator:
			{
				v3 EntityP = Entity->P;
				EntityP.y += Entity->VisualScale.y;
				T = Mat4Translate(EntityP);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(Entity->VisualScale);

				model *Cube = LookupModel(Assets, "Cube").Model;
				Entry = LookupTexture(Assets, "red_texture_02");
				PushTexture(RenderBuffer, Entry.Texture, Entry.Index);
				PushMesh(RenderBuffer, &Cube->Meshes[0], T*R*S, Entry.Index);
			} break;
		};
	}

	//
	// NOTE(Justin): Debug ui.
	//

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
		GameState->CameraIsFree = !GameState->CameraIsFree;
	}

	if(WasPressed(Keyboard->F9))
	{
		GameState->CameraIsLocked = !GameState->CameraIsLocked;
	}

	if(GameState->CameraIsFree && !GameState->CameraIsLocked)
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

		v3 P = Camera->P + GameState->CameraSpeed*dt*CameraddP;
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
		DebugDrawCapsule(Entity);
	}

	if(ToggleButton("DrawCombatCollider", DebugDrawCollisionVolumes))
	{
		DebugDrawCollisionVolumes(Entity);
	}

	if(ToggleButton("DrawJoints", DebugDrawJoints))
	{
		DebugDrawJoints(Entity);
	}

	if(ToggleButton("+Player", DebugDrawEntity))
	{
		DebugDrawEntity(Entity);
	}

	if(ToggleButton("+AnimationPlayer", DebugDrawAnimationPlayer))
	{
		DebugDrawAnimationPlayer(Entity->AnimationPlayer);
	}

	if(ToggleButton("+Texture", DebugDrawTexture))
	{
		DebugDrawTexture(GameState);
	}

	Platform.RenderToOpenGL(RenderBuffer, (u32)GameInput->BackBufferWidth, (u32)GameInput->BackBufferHeight);

	TemporaryMemoryEnd(RenderMemory);
	TemporaryMemoryEnd(AnimationMemory);
}
