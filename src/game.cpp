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
	FlagAdd(Entity, EntityFlag_Visible);

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

	Player->Attacks[AttackType_Neutral1].Type = AttackType_Neutral1;
	Player->Attacks[AttackType_Neutral1].Duration = 0.2f;
	Player->Attacks[AttackType_Neutral1].Power = 0.07f;

	Player->Attacks[AttackType_Neutral2].Type = AttackType_Neutral2;
	Player->Attacks[AttackType_Neutral2].Duration = 0.3f;
	Player->Attacks[AttackType_Neutral2].Power = 0.07f;

	model *Model = LookupModel(&GameState->AssetManager, "YBot").Model;
	animation_graph *G  = LookupGraph(&GameState->AssetManager, "YBot_AnimationGraph");
	asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "YBot_FightIdleLeft");

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

	Player->Attacks[AttackType_Sprint].Type = AttackType_Sprint;
	Player->Attacks[AttackType_Sprint].Duration = 0.7f;
	Player->Attacks[AttackType_Sprint].Power = 12.0f;

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
BruteAdd(game_state *GameState, v3 P)
{
	entity *Player = PlayerAdd(GameState, P);
	Player->Acceleration = 40.0f;
	Player->Drag = 10.0f;
	Player->AngularSpeed = 10.0f;
	Player->VisualScale = V3(0.01f);

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
	Player->Attacks[AttackType_Neutral2].Duration = 0.3f;
	Player->Attacks[AttackType_Neutral2].Power = 5.0f;

	Player->Attacks[AttackType_Neutral3].Type = AttackType_Neutral3;
	Player->Attacks[AttackType_Neutral3].Duration = 0.3f;
	Player->Attacks[AttackType_Neutral3].Power = 7.0f;

	Player->Attacks[AttackType_Strong].Type = AttackType_Strong;
	Player->Attacks[AttackType_Strong].Duration = 0.5f;
	Player->Attacks[AttackType_Strong].Power = 18.0f;

	Player->Attacks[AttackType_Sprint].Type = AttackType_Sprint;
	Player->Attacks[AttackType_Sprint].Duration = 1.0f;
	Player->Attacks[AttackType_Sprint].Power = 3.0f;

	model *Model = LookupModel(&GameState->AssetManager, "Brute").Model;
	animation_graph *G  = LookupGraph(&GameState->AssetManager, "Brute_AnimationGraph");
	asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "Brute_Idle");

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

		v3 GameDelta = V3(Entity->VisualScale.x * AnimationDelta.x,
						  Entity->VisualScale.y * AnimationDelta.y,
						  Entity->VisualScale.z * AnimationDelta.z);

		DeltaP = Conjugate(Entity->Orientation)*GameDelta;

		//ddP = Normalize(DeltaP);

		AnimationPlayer->RootMotionAccumulator = {};

		Entity->ddP = ddP;
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

internal void
LevelLoad(game_state *GameState, char *FileName)
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

	b32 ParsingEntity = false;
	entity *Current = GameState->Entities + GameState->EntityCount;
	while(*Content)
	{
		BufferLine(&Content, LineBuffer);
		u8 *Line = LineBuffer;
		BufferNextWord(&Line, Word);

		if(StringsAreSame(Word, "entity"))
		{
			if(ParsingEntity)
			{
				Current->ID = GameState->EntityCount;
				GameState->EntityCount++;
				Current = GameState->Entities + GameState->EntityCount;
			}
			else
			{
				ParsingEntity = true;
			}
		}

		if(!ParsingEntity)
		{
			if(StringsAreSame(Word, "camera_speed"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Camera.Speed = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "camera_yaw"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Camera.DefaultYaw = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "camera_pitch"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Camera.DefaultPitch = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "camera_position"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Camera.P.x = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Camera.P.y = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Camera.P.z = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "camera_offset_from_player"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->CameraOffsetFromPlayer.x = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->CameraOffsetFromPlayer.y = F32FromASCII(Word);
				
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->CameraOffsetFromPlayer.z = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "camera_yaw"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->FOV = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "fov"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->FOV = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "z_near"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->ZNear = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "z_far"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->ZFar = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "gravity"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				GameState->Gravity = F32FromASCII(Word);
			}
		}
		else
		{
			if(StringsAreSame(Word, "type"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);

				if(StringsAreSame(Word, "walkable_region"))
				{
					Current->Type = EntityType_WalkableRegion;
				}
				else if(StringsAreSame(Word, "cube"))
				{
					Current->Type = EntityType_Cube;
				}
			}
			else if(StringsAreSame(Word, "flags"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Current->Flags = U32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "position"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Current->P.x = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Current->P.y = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Current->P.z = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "axis_angle"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				f32 X = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				f32 Y = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				f32 Z = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				f32 Angle = F32FromASCII(Word);

				Current->Orientation = Quaternion(V3(X, Y , Z), DegreeToRad(Angle));
			}
			else if(StringsAreSame(Word, "movement_collider_count"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Current->MovementColliders.VolumeCount = U32FromASCII(Word);
				Current->MovementColliders.Volumes = PushArray(&GameState->Arena, Current->MovementColliders.VolumeCount, collision_volume);
			}
			else if(StringsAreSame(Word, "movement_collider_type"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);

				if(StringsAreSame(Word, "obb"))
				{
					Current->MovementColliders.Type = CollisionVolumeType_OBB; 
				}
			}
			else if(StringsAreSame(Word, "dim"))
			{
				v3 Dim = {};

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Dim.x = F32FromASCII(Word); 

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Dim.y = F32FromASCII(Word); 

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Dim.z = F32FromASCII(Word); 

				collision_volume *Volume = Current->MovementColliders.Volumes;
				quaternion Q = Conjugate(Current->Orientation);
				Volume->OBB.Dim = 0.99f*Dim;
				Volume->OBB.Center  = 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
				Volume->Offset		= 0.5f*V3(0.0f, Volume->OBB.Dim.y, 0.0f);
				Volume->OBB.X = Q * XAxis();
				Volume->OBB.Y = Q * YAxis();
				Volume->OBB.Z = Q * (-1.0f*ZAxis());
			}
			else if(StringsAreSame(Word, "visual_scale"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				f32 Scale = F32FromASCII(Word); 
				Current->VisualScale = Scale * Current->MovementColliders.Volumes[0].OBB.Dim;
			}
		}

		AdvanceLine(&Content);
	}

	Platform.DebugFileFree(File.Content);
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

#if DEVELOPER
	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.LevelFileInfo))
	{
#if 1
		GameState->EntityCount = 0;
		EntityAdd(GameState, EntityType_Null);
		LevelLoad(GameState, "../src/test.level");
		GameState->EntityCount += 4;
#else
		GameState->EntityCount = 0;
		GameState->PlayerEntityIndex = 0;
		MemoryZero(GameState->Entities, ArrayCount(GameState->Entities) * sizeof(entity));
		EntityAdd(GameState, EntityType_Null);
		LevelLoad(GameState, "../src/test.level");

		entity *Player = XBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
		GameState->PlayerIDForController[1] = Player->ID;

		entity *YBot = YBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
		entity *Knight = KnightAdd(GameState, V3(0.0f, 0.01f, -5.0f));
		entity *Brute = BruteAdd(GameState, V3(0.0f, 0.01f, -5.0f));

		FlagClear(YBot, EntityFlag_Visible);
		FlagClear(Knight, EntityFlag_Visible);
		FlagClear(Brute, EntityFlag_Visible);

		GameState->CharacterIDs[GameState->CurrentCharacter] = Player->ID;
		GameState->CharacterIDs[GameState->CurrentCharacter + 1] = YBot->ID;
		GameState->CharacterIDs[GameState->CurrentCharacter + 2] = Knight->ID;
		GameState->CharacterIDs[GameState->CurrentCharacter + 3] = Brute->ID;

#endif
	}

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.XBotGraphFileInfo))
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

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.YBotGraphFileInfo))
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

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.PaladinGraphFileInfo))
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

	if(!GameInput->ReloadingGame && ShouldReload(&GameState->AssetManager.BruteGraphFileInfo))
	{
		entity *Entity = GameState->Entities + GameState->PlayerIDForController[0];
		animation_graph *G = LookupGraph(&GameState->AssetManager, "Brute_AnimationGraph");
		ArenaClear(&G->Arena);
		G->NodeCount = 0;
		G->Index = 0;
		G->CurrentNode = {};
		MemoryZero(&G->Nodes, sizeof(G->Nodes));
		AnimationGraphInitialize(G, "../src/Brute_AnimationGraph.animation_graph");
		Entity->AnimationGraph = G;
	}
#endif

	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(GameInput->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = ControllerGet(GameInput, ControllerIndex);
		u32 ID = GameState->PlayerIDForController[ControllerIndex];
		if(ID == 0)
		{
			if((ControllerIndex == 1) && Controller->Start.EndedDown)
			{
				entity *Player = XBotAdd(GameState, V3(0.0f, 0.01f, -5.0f));
				GameState->PlayerIDForController[ControllerIndex] = Player->ID;

				entity *YBot = YBotAdd(GameState, V3(-10.0f, 0.01f, -2.0f));
				entity *Knight = KnightAdd(GameState, V3(-5.0f, 0.01f, -2.0f));
				entity *Brute = BruteAdd(GameState, V3(5.0f, 0.01f, -2.0f));

				//FlagClear(YBot, EntityFlag_Visible);
				//FlagClear(Knight, EntityFlag_Visible);
				//FlagClear(Brute, EntityFlag_Visible);

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

#if 0
			if((ControllerIndex == 1) && WasPressed(Controller->Start))
			{

				u32 CurrentID = GameState->PlayerEntityIndex;
				entity *Current = GameState->Entities + CurrentID;
				FlagClear(Current, EntityFlag_Visible);

				GameState->CurrentCharacter++;
				if(GameState->CurrentCharacter > 3)
				{
					GameState->CurrentCharacter = 0;
				}

				CurrentID = GameState->CharacterIDs[GameState->CurrentCharacter];
				GameState->PlayerIDForController[ControllerIndex] = CurrentID;
				GameState->PlayerEntityIndex = CurrentID;
				Current = GameState->Entities + GameState->PlayerEntityIndex;
				FlagAdd(Current, EntityFlag_Visible);
				Swapped = true;

			}
#else
#endif

			// Handle input 

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
		}

		if(!Equal(Entity->MoveInfo.ddP, V3(0.0f)) && CanMove(Entity))
		{
			if(IsMoving(Entity))
			{
				EntityMove(GameState, Entity, Entity->MoveInfo.ddP, dt);
				EntityOrientationUpdate(Entity, dt, Entity->AngularSpeed);
			}
		}

		if(Equal(Entity->MoveInfo.ddP, V3(0.0f)))
		{
			if(Entity->AnimationPlayer && Entity->AnimationPlayer->ControlsPosition)
			{
				EntityMove(GameState, Entity, Entity->MoveInfo.ddP, dt);
				EntityOrientationUpdate(Entity, dt, Entity->AngularSpeed);
			}
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

				if(AnimationPlayer->ControlsPosition)
				{
					T = Mat4Translate(AnimationPlayer->EntityPLockedAt);
					FlagAdd(Entity, EntityFlag_AnimationControlling);
				}
				else
				{
					FlagClear(Entity, EntityFlag_AnimationControlling);
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

	if(ToggleButton("+ShadowMap", DebugRenderToTexture))
	{
		DebugRenderToTexture(GameState);
	}

	if(ToggleButton("TextureView", DebugDrawTexture))
	{
		DebugDrawTexture("texture_01", 256.0f, 256.0f);
	}

	Platform.RenderToOpenGL(RenderBuffer, (u32)GameInput->BackBufferWidth, (u32)GameInput->BackBufferHeight);

	TemporaryMemoryEnd(RenderMemory);
	TemporaryMemoryEnd(AnimationMemory);
}
