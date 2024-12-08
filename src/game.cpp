
#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "font.cpp"
#include "mesh.cpp"
#include "animation.cpp"
#include "ui.cpp"
#include "asset.cpp"
#include "render.cpp"

internal entity * 
EntityAdd(game_state *GameState, entity_type Type)
{
	Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
	entity *Entity = GameState->Entities + GameState->EntityCount++;
	Entity->Type = Type;
	return(Entity);
}

internal void
PlayerAdd(game_state *GameState)
{
	entity *Entity = EntityAdd(GameState, EntityType_Player);
	GameState->PlayerEntityIndex = GameState->EntityCount - 1;
	Entity->P = V3(0.0f, 0.0f, -10.0f);
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);

	f32 Radians = DirectionToEuler(V3(0.0f, 0.0f, -1.0f)).yaw;
	Entity->Theta = RadToDegrees(Radians);
	Entity->dTheta = 0.0f;
	Entity->Orientation = Quaternion(V3(0.0f, 1.0f, 0.0f), Radians);
	Entity->MovementState = MovementState_Idle;

	// NOTE(Justin): The AABBDim is used for collision detection AND for AABB rendering. The visual
	// scale is used to scale the player model. Therefore the visual scale and the AABBDim are unrelated

	Entity->Height = 1.8f;
	Entity->AABBDim = V3(0.7f, Entity->Height, 0.7f);
	Entity->VolumeOffset = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);
	Entity->VisualScale = V3(0.01f);
}

internal void
CubeAdd(game_state *GameState, v3 P, v3 Dim)
{
	entity *Entity = EntityAdd(GameState, EntityType_Cube);
	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Orientation = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);

	Entity->Height = 1.0f;

	// NOTE(Justin): The cube mesh has dimensions 2x2x2. If we want the gameplay dimensions of
	// the cube to be 1x1x1, then the visual scale of the cube is 0.5. The AABBDim is used for collision
	// detection and the visual scale is used for rendering

	Entity->AABBDim = Dim;
	Entity->VisualScale = 0.5f*Entity->AABBDim;
}

inline mat4
EntityTransform(entity *Entity, f32 Scale = 1.0f)
{
	mat4 Result = Mat4Identity();

	mat4 R = QuaternionToMat4(Entity->Orientation);
	Result = Mat4(Scale * Mat4ColumnGet(R, 0),
				  Scale * Mat4ColumnGet(R, 1),
				  Scale * Mat4ColumnGet(R, 2),
				  Entity->P);

	return(Result);
}

inline mat4
EntityTransform(entity *Entity, v3 Scale = V3(1.0f))
{
	mat4 Result = Mat4Identity();

	mat4 R = QuaternionToMat4(Entity->Orientation);
	Result = Mat4(Scale.x * Mat4ColumnGet(R, 0),
				  Scale.y * Mat4ColumnGet(R, 1),
				  Scale.z * Mat4ColumnGet(R, 2),
				  Entity->P);

	return(Result);
}

inline void 
EntityMovementState(char *Buffer, entity *Entity)
{
	switch(Entity->MovementState)
	{
		case MovementState_Idle:
		{
			sprintf(Buffer, "%s", "MovementState: Idle");
		} break;
		case MovementState_Run:
		{
			sprintf(Buffer, "%s", "MovementState: Running");
		} break;
		case MovementState_Sprint:
		{
			sprintf(Buffer, "%s", "MovementState: Sprint");
		} break;
		case MovementState_Jump:
		{
			sprintf(Buffer, "%s", "MovementState: JumpForward");
		} break;
		case MovementState_Crouch:
		{
			sprintf(Buffer, "%s", "MovementState: Crouch");
		} break;
	}
}

inline void
EntityOrientationUpdate(entity *Entity, f32 dt, f32 AngularSpeed)
{
	f32 OldTheta = Entity->Theta;
	quaternion Orientation = Entity->Orientation;
	v3 FacingDirection = Entity->dP;
	f32 LengthSquared = Dot(FacingDirection, FacingDirection);
	if(LengthSquared != 0.0f)
	{
		// This angle is the angle we would rotate from the starting orientation (0 degress) to 
		// end up at the current orientation. IT IS NOT THE ANGLE WE ROTATE BY. E.g. if the player starts out
		// facing towards the camera with 0 degress of rotation and the player goes left. The final orientation is
		// This is confusing.. Need to think about why this works..

		f32 TargetAngleInRad = DirectionToEuler(FacingDirection).yaw;
		Entity->dTheta = RadToDegrees(TargetAngleInRad) - OldTheta;
		Entity->Theta = RadToDegrees(TargetAngleInRad);

		quaternion Target = Quaternion(V3(0.0f, 1.0f, 0.0f), TargetAngleInRad);
		Target = Conjugate(Target);
		Entity->Orientation = RotateTowards(Orientation, Target, dt, AngularSpeed);
	}
}

inline quaternion 
OrientationUpdate(quaternion Orientation, f32 TargetAngleInDegrees, f32 dt, f32 AngularSpeed)
{
	quaternion Result = Orientation;
	quaternion Target = Quaternion(V3(0.0f, 1.0f, 0.0f), DegreeToRad(TargetAngleInDegrees));
	Target = Conjugate(Target);
	Result = RotateTowards(Orientation, Target, dt, AngularSpeed);

	return(Result);
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

internal b32
PointAndPlaneIntersect(v3 RelP, v3 DeltaP, v3 PointOnPlane, v3 PlaneNormal, aabb MKSumAABB, f32 *tMin)
{
	b32 Collided = false;

	f32 tEpsilon = 0.001f;
	f32 D = Dot(PlaneNormal, PointOnPlane);
	if(!Equal(DeltaP, V3(0.0f)))
	{
		f32 tResult = (D - Dot(PlaneNormal, RelP)) / Dot(PlaneNormal, DeltaP);
		if((tResult >= 0.0f) && tResult < *tMin)
		{
			v3 PointOfIntersection = RelP + tResult * DeltaP;
			if(InAABB(MKSumAABB, PointOfIntersection))
			{
				*tMin = Max(0.0f, tResult - tEpsilon);
				Collided = true;
			}
		}
	}

	return(Collided);
}

internal void
EntityMove(game_state *GameState, entity *Entity, v3 ddP, f32 dt)
{
	v3 DeltaP = 0.5f * dt * dt * ddP + dt * Entity->dP;
	Entity->dP = dt * ddP + Entity->dP;

	v3 OldP = Entity->P;
	f32 DeltaLength = Length(DeltaP);

	v3 DesiredP = OldP + DeltaP;
	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		f32 tMin = 1.0f;
		b32 Collided = false;
		v3 Normal = V3(0.0f);
		for(u32 TestEntityIndex = 0; TestEntityIndex < GameState->EntityCount; ++TestEntityIndex)
		{
			entity *TestEntity = GameState->Entities + TestEntityIndex;
			if(Entity != TestEntity)
			{
				v3 TestP = TestEntity->P;
				v3 CurrentP = Entity->P; 
				v3 RelP = (CurrentP + Entity->VolumeOffset) - (TestP + Entity->VolumeOffset);

				v3 AABBDim = Entity->AABBDim + TestEntity->AABBDim;
				aabb MKSumAABB = AABBCenterDim(V3(0.0f), AABBDim);

				// NOTE(Jusitn): Left face
				v3 PlaneNormal = {-1.0f, 0.0f, 0.0f};
				v3 PointOnPlane = MKSumAABB.Min;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Right face
				PlaneNormal = {1.0f, 0.0f, 0.0f};
				PointOnPlane = MKSumAABB.Max;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Back face
				PlaneNormal = {0.0f, 0.0f, -1.0f};
				PointOnPlane = MKSumAABB.Max;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Front face
				PlaneNormal = {0.0f, 0.0f, 1.0f};
				PointOnPlane = MKSumAABB.Min;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Top face
				PlaneNormal = {0.0f, 1.0f, 0.0f};
				PointOnPlane = MKSumAABB.Max;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}
			}
		}

		Entity->P += tMin * DeltaP; 
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
}

internal void
EntityMoveByAnimation(game_state *GameState, entity *Entity, v3 DeltaP, f32 dt)
{
	v3 OldP = Entity->P;
	v3 DesiredP = OldP + DeltaP;
	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
	f32 tMin = 1.0f;
		b32 Collided = false;
		v3 Normal = V3(0.0f);
		for(u32 TestEntityIndex = 0; TestEntityIndex < GameState->EntityCount; ++TestEntityIndex)
		{
			entity *TestEntity = GameState->Entities + TestEntityIndex;
			if(Entity != TestEntity)
			{
				v3 TestP = TestEntity->P;
				v3 CurrentP = Entity->P; 
				v3 RelP = (CurrentP + Entity->VolumeOffset) - (TestP + Entity->VolumeOffset);

				v3 AABBDim = Entity->AABBDim + TestEntity->AABBDim;
				aabb MKSumAABB = AABBCenterDim(V3(0.0f), AABBDim);

				// NOTE(Jusitn): Left face
				v3 PlaneNormal = {-1.0f, 0.0f, 0.0f};
				v3 PointOnPlane = MKSumAABB.Min;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Right face
				PlaneNormal = {1.0f, 0.0f, 0.0f};
				PointOnPlane = MKSumAABB.Max;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Back face
				PlaneNormal = {0.0f, 0.0f, -1.0f};
				PointOnPlane = MKSumAABB.Max;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Front face
				PlaneNormal = {0.0f, 0.0f, 1.0f};
				PointOnPlane = MKSumAABB.Min;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}

				// NOTE(Jusitn): Top face
				PlaneNormal = {0.0f, 1.0f, 0.0f};
				PointOnPlane = MKSumAABB.Max;
				if(PointAndPlaneIntersect(RelP, DeltaP, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
				{
					Normal = PlaneNormal;
					Collided = true;
				}
			}
		}

		Entity->P += tMin * DeltaP; 
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

		ArenaSubset(&GameState->Arena, &GameState->AssetManager.Arena, Kilobyte(512));
		AssetManagerInit(&GameState->AssetManager);
		asset_manager *Assets = &GameState->AssetManager;

		GameState->Quad = QuadDefault();
		GameState->Texture.Width = 256;
		GameState->Texture.Height = 256;

		// Player
		PlayerAdd(GameState);
		entity *Player			= GameState->Entities + GameState->PlayerEntityIndex;
		Player->AnimationPlayer = PushStruct(Arena, animation_player);
		Player->AnimationGraph	= PushStruct(Arena, animation_graph);
		model *XBot				= LookupModel(Assets, "XBot");
		AnimationPlayerInitialize(Player->AnimationPlayer, XBot, Arena);
		Player->AnimationGraph	= LookupGraph(Assets, "XBot_AnimationGraph");

		// Cubes 
		v3 StartP = V3(-3.0f, 0.0f, -10.0f);
		v3 Dim = V3(2.0f, 1.0f, 10.0f);
		CubeAdd(GameState, StartP, Dim);

		Dim = V3(2.0f, 10.0f, 2.0f);
		StartP += V3(6.0f, 0.0f, -0.0f);
		CubeAdd(GameState, StartP, Dim);

		Dim = V3(1.f, 1.0f, 2.0f);
		StartP += V3(6.5f, 0.0f, 0.0f);
		CubeAdd(GameState, StartP, Dim);

		//LevelSave((entity *)GameState->Entities, GameState->EntityCount);

		GameState->CameraOffsetFromPlayer = V3(0.0f, 2.0f, 5.0f);
		CameraSet(&GameState->Camera, Player->P + GameState->CameraOffsetFromPlayer, -90.0f, -10.0f);
		GameState->Camera.RotationAboutY = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);
		CameraTransformUpdate(GameState);

		GameState->TimeScale = 1.0f;
		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)GameInput->BackBufferWidth / (f32)GameInput->BackBufferHeight;
		GameState->ZNear = 0.2f;
		GameState->ZFar = 200.0f;
		GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);
		GameState->Gravity = 9.8f;

		GameState->Cylinder = ModelCylinderInititalize(Arena);

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

	v3 ddP = {};
	game_keyboard *Keyboard = &GameInput->Keyboard;
	if(IsDown(Keyboard->W))
	{
		ddP += V3(0.0f, 0.0f, -1.0f);
	}
	if(IsDown(Keyboard->A))
	{
		ddP += V3(-1.0f, 0.0f, 0.0f);
	}
	if(IsDown(Keyboard->S))
	{
		ddP += V3(0.0f, 0.0f, 1.0f);
	}
	if(IsDown(Keyboard->D))
	{
		ddP += V3(1.0f, 0.0f, 0.0f);
	}

	b32 Sprinting = false;
	if(IsDown(Keyboard->Shift))
	{
		Sprinting = true;
	}
	
	b32 Jumping = false;
	if(IsDown(Keyboard->Space))
	{
		Jumping = true;
	}

	b32 Crouching = false;
	if(IsDown(Keyboard->Ctrl))
	{
		Crouching = true;
	}

	if(IsDown(Keyboard->Add))
	{
		GameState->TimeScale *= 1.1f;
	}

	if(IsDown(Keyboard->Subtract))
	{
		GameState->TimeScale *= 0.9f;
	}

	if(!Equal(ddP, V3(0.0f)))
	{
		ddP = Normalize(ddP);
	}
	else
	{
	}

	f32 dt = GameInput->DtForFrame;
	dt *= GameState->TimeScale;
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				f32 a = 40.0f;

				v3 OldPlayerddP = Entity->ddP;
				if(!Entity->AnimationGraph->CurrentNode.ControlsPosition)
				{
					Entity->ddP = ddP;

					if(Sprinting)
					{
						a *= 1.5f;
					}

					f32 PlayerDrag = -10.0f;
					f32 AngularSpeed = 10.0f;
					f32 Speed = Length(Entity->dP);
					ddP = a * ddP;
					ddP += PlayerDrag * Entity->dP;

					EntityMove(GameState, Entity, ddP, dt);
					EntityOrientationUpdate(Entity, dt, AngularSpeed);
				}

				switch(Entity->MovementState)
				{
					case MovementState_Idle:
					{
						if(Equal(OldPlayerddP, V3(0.0f)) &&
						  !Equal(Entity->ddP, OldPlayerddP))
						{
							if(Sprinting)
							{
								Entity->MovementState = MovementState_Sprint;
							}
							else
							{
								Entity->MovementState = MovementState_Run;
							}
						}

						if(Crouching)
						{
							Entity->MovementState = MovementState_Crouch;
						}
					} break;
					case MovementState_Crouch:
					{
						if(!Crouching)
						{
							Entity->MovementState = MovementState_Idle;
						}
					} break;
					case MovementState_Run:
					{
						if(Equal(Entity->ddP, V3(0.0f)))
						{
							Entity->MovementState = MovementState_Idle;
						}
						else
						{
							if(Sprinting)
							{
								Entity->MovementState = MovementState_Sprint;
							}

							if(Jumping)
							{
								Entity->MovementState = MovementState_Jump;
							}
						}

					} break;
					case MovementState_Sprint:
					{
						if(Equal(Entity->ddP, V3(0.0f)))
						{
							Entity->MovementState = MovementState_Idle;
						}
						else
						{
							if(!Sprinting)
							{
								Entity->MovementState = MovementState_Run;
							}
						}
					} break;
					case MovementState_Jump:
					{
						if(!Jumping)
						{
							if(Equal(Entity->ddP, V3(0.0f)))
							{
								Entity->MovementState = MovementState_Idle;
							}
							else if(Sprinting)
							{
								Entity->MovementState = MovementState_Sprint;
							}
							else
							{
								Entity->MovementState = MovementState_Run;
							}
						}
					} break;
					case MovementState_Invalid:
					{
						Entity->MovementState = MovementState_Invalid;
					} break;

				};
			} break;
			case EntityType_Cube:
			{
			} break;
		}
	}

	asset_manager *Assets = &GameState->AssetManager;

	//
	// NOTE(Justin): Animation update, will have to be rolled into physics work per entity....
	//

	temporary_memory AnimationMemory = TemporaryMemoryBegin(&TempState->Arena);

	entity *Player = GameState->Entities + GameState->PlayerEntityIndex;
	Animate(Player, Assets);
	if(Player->AnimationGraph->CurrentNode.ControlsPosition)
	{
		// How this hackiness works. The player's position accumulated a delta vector
		// for each frame that the animation is playing. The delta vector is the root position
		// before and after the animation player updates. We then move the player by this delta amount.
		// Now the rig itself will also be updating each frame the animation is playing. So we need to offset
		// the rigs position by a delta vector too. Otherwise the gameplay position and visual position both 
		// accumulate a delta for each frame. The offset needed is going to be the total delta from the start
		// of the animation to the current time because the player position has already accumulated it. This vecotr
		// is the rigs tpose root position minus the current position in the animation.

		// This method has a problem. We are still playing the animation. So when we go to blend it with another
		// animation, the visual position has been updated the entire time. So when we start blending in another animation
		// the player teleports to the blended position.

		// TODO(Justin): Robustness
		// TODO(Justin): Update velocity..
		
		v3 OldP = Player->AnimationPlayer->FinalPose->Positions[0];
		AnimationPlayerUpdate(Player->AnimationPlayer, &TempState->Arena, dt);
		v3 NewP = Player->AnimationPlayer->FinalPose->Positions[0];

		v3 AnimationDelta = NewP - OldP;
		v3 GameDelta = 2.0f*V3(Player->VisualScale.x * AnimationDelta.x,
							   Player->VisualScale.y * AnimationDelta.y,
							   Player->VisualScale.z * AnimationDelta.z);

		GameDelta = Conjugate(Player->Orientation)*GameDelta;

		EntityMoveByAnimation(GameState, Player, GameDelta, dt);

		if(Player->P.y <= 0.0f)
		{
			Player->P.y = 0.0f;
		}

		v3 RootP = JointPositionGet(Player->AnimationPlayer->Model, 0);
		v3 OffsetToRoot = RootP - OldP;
		ModelJointsUpdate(Player->AnimationPlayer, OffsetToRoot);
	}
	else
	{
		AnimationPlayerUpdate(Player->AnimationPlayer, &TempState->Arena, dt);
		if(!Equal(Player->AnimationPlayer->AnimationDelta, V3(0.0f)))
		{
			// TODO(Justin): Robustness
			v3 RootP = JointPositionGet(Player->AnimationPlayer->Model, 0);
			Player->AnimationPlayer->AnimationDelta = V3(0.0f);

			Player->AnimationPlayer->FinalPose[0].Positions[0] = RootP;
			Player->AnimationPlayer->FinalPose[1].Positions[0] = RootP;
			ModelJointsUpdate(Player->AnimationPlayer);
		}
		else
		{
			ModelJointsUpdate(Player->AnimationPlayer);
		}
	}

	AnimationGraphPerFrameUpdate(Assets, Player->AnimationPlayer, Player->AnimationGraph);

	camera *Camera = &GameState->Camera;
#if 0
	Camera->RotationAboutY = OrientationUpdate(Camera->RotationAboutY, 180.0f - Player->Theta, dt, 10.0f);
	Camera->P = Player->P + Camera->RotationAboutY*GameState->CameraOffsetFromPlayer;

	v3 NewDirection = Camera->RotationAboutY * Camera->Direction;
	f32 D = Dot(NewDirection, Camera->Direction);
	if(D < 0.99f)
	{
		Camera->Direction = Normalize(Player->P - Camera->P + Camera->Direction);
	}
#else
	Camera->P = Player->P + GameState->CameraOffsetFromPlayer;
#endif

	//
	// NOTE(Justin): Render.
	//

	// NOTE(Justin): The viewing volume and transformations of the light are orientated with respect to 
	// the light's viewing transformation. E.g. This directionatl light is to the right,down, and forward. We
	// place the light up, to the left, and at the front of the scene. The orthographic clip volume is
	// orientated the same was as the light. Meaning the left face of the volume is am xy plane in world space
	// that is further in the -Z DIRECTION. The right face is also an xy plane in world space
	// that is closer in the +Z DIRECTOIN. Changing the left and right values of the volume will
	// change what objects cast shadows based on their Z POSITION. If a z coordinate of an object
	// is more negative than the left face the object WILL NOT CAST A SHADOW. If the light source is
	// directional, then this is a visual artifact since the object SHOULD cast a shadow because the light
	// source is directional.
	// 

	v3 LightDir = V3(1.0f, -1.0f, -1.0f);
	v3 LightP = V3(-10.0f, 10.0f, 0.0f);
	f32 Left = -20.0f;
	f32 Right = 20.0f;
	f32 Bottom = -10.0f;
	f32 Top = 10.0f;
	f32 Near = GameState->ZNear;
	f32 Far = GameState->ZFar;

	mat4 LightOrtho = Mat4OrthographicProjection(Left, Right, Bottom, Top, Near, Far);
	mat4 LightView = Mat4Camera(LightP, LightDir);
	mat4 LightTransform = LightOrtho * LightView;
	PerspectiveTransformUpdate(GameState, (f32)GameInput->BackBufferWidth, (f32)GameInput->BackBufferHeight);
	CameraTransformUpdate(GameState);

	temporary_memory RenderMemory = TemporaryMemoryBegin(&TempState->Arena);
	render_buffer *RenderBuffer = RenderBufferAllocate(&TempState->Arena, Megabyte(512),
														GameState->CameraTransform,
														GameState->Perspective,
														LightTransform,
														Assets,
														Camera->P,
														LightDir);

	PushClear(RenderBuffer, V4(0.3f, 0.4f, 0.4f, 1.0f));

	//
	// NOTE(Justin): Ground quad.
	//

	quad GroundQuad = GameState->Quad;
	for(u32 Index = 0; Index < ArrayCount(GroundQuad.Vertices); ++Index)
	{
		GroundQuad.Vertices[Index].UV *= 250.0f;
	}

	mat4 T = Mat4Translate(V3(0.0f, 0.0f, -250.0f));
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(500.0f);
	s32 TextureIndex = StringHashLookup(&Assets->TextureNames, "texture_01");
	PushTexture(RenderBuffer, LookupTexture(Assets, "texture_01"), TextureIndex);
	PushQuad3D(RenderBuffer, GroundQuad.Vertices, T*R*S, TextureIndex);

	//
	// NOTE(Justin): Entities.
	//

	// NOTE(Justin): Convention is that the ground position is y=0. So, to render correctly we need a visual
	// offset.

	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				mat4 Transform = EntityTransform(Entity, Entity->VisualScale);
				model *Model = LookupModel(Assets, "XBot");
				PushModel(RenderBuffer, Model, Transform);

				//
				// NOTE(Justin): Debug
				//

				// AABB
				T = Mat4Translate(Entity->P + Entity->VolumeOffset);
				R = QuaternionToMat4(Entity->Orientation);

				// The visual scale of the player and the AABBDim are unrelated to we have to scale the dim
				// by 0.5.

				// TODO(Justin): Debug this.
				S = Mat4Scale(0.5f*Entity->AABBDim);
				PushAABB(RenderBuffer, LookupModel(Assets, "Cube"), T*R*S, V3(1.0f));

				// Ground arrow 
				v3 P = Entity->P;
				P.y += 0.1f;
				T = Mat4Translate(P);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(V3(0.5f));

				TextureIndex = StringHashLookup(&Assets->TextureNames, "left_arrow");
				PushTexture(RenderBuffer, LookupTexture(Assets, "left_arrow"), TextureIndex);
				//PushQuad3D(RenderBuffer, GameState->Quad.Vertices, T*R*S, TextureIndex);
			} break;
			case EntityType_Cube:
			{
				model *Cube = LookupModel(Assets, "Cube");
				v3 EntityP = Entity->P;
				EntityP.y += Entity->VisualScale.y;
				T = Mat4Translate(EntityP);
				S = Mat4Scale(Entity->VisualScale);

				PushTexture(RenderBuffer, Cube->Meshes[0].Texture, StringHashLookup(&Assets->TextureNames, (char *)Cube->Meshes[0].Texture->Name.Data));
				PushModel(RenderBuffer, Cube, T*S);
				//PushAABB(RenderBuffer, LookupModel(Assets, "Cube"), T*S, V3(1.0f));
			} break;
		};
	}

	//
	// NOTE(Justin): Test font/ui.
	//

	entity *Entity = GameState->Entities + GameState->PlayerEntityIndex;

	font *FontInfo =  &Assets->Font;
	f32 Scale = 0.35f;
	f32 Gap = Scale * (f32)FontInfo->LineHeight / 64.0f;
	f32 X = 0.0f;
	f32 Y = (f32)GameInput->BackBufferHeight - Gap;
	f32 dY = 5.0f;
	s32 WindowWidth = GameInput->BackBufferWidth;
	s32 WindowHeight = GameInput->BackBufferHeight;
	v2 MouseP = V2(GameInput->MouseX, GameInput->MouseY);
	v2 P = V2(X, Y);
	v3 HoverColor = V3(1.0f, 1.0f, 0.0f);
	v3 DefaultColor = V3(1.0f);

	ui *UI = &GameState->UI;
	UI->MouseP = MouseP;
	UI->LeftClick = WasPressed(GameInput->MouseButtons[MouseButton_Left]);
	UI->HotID = 0;

	char Buff[256];
	sprintf(Buff, "%s", "Controls: wasd to move, shift to sprint, +- to scale time");
	string Text = StringCopy(&TempState->Arena, Buff);
	PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
	P.y -= (Gap + dY);

	sprintf(Buff, "%s %.2f", "time scale: ", GameState->TimeScale);
	Text = StringCopy(&TempState->Arena, Buff);
	PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
	P.y -= (Gap + dY);

	sprintf(Buff, "+Player");
	Text = StringCopy(&TempState->Arena, Buff);
	rect Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));
	ui_button EntityButton;
	EntityButton.ID = 1;
	EntityButton.P = P;
	EntityButton.Rect = Rect;

	if(InRect(EntityButton.Rect, UI->MouseP))
	{
		UI->HotID = EntityButton.ID;
		if(UI->ActiveID == 0 && UI->LeftClick)
		{
			UI->ActiveID = EntityButton.ID;
		}
		else if(UI->ActiveID == EntityButton.ID && UI->LeftClick)
		{
			UI->ActiveID = 0;
		}
		else if((UI->ActiveID != EntityButton.ID) && UI->LeftClick)
		{
			UI->ActiveID = EntityButton.ID;
		}
	}

	if(UI->ActiveID == EntityButton.ID)
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, EntityButton.P, Scale, HoverColor);
		P.y -= (Gap + dY);

		P.x += 20.0f;
		sprintf(Buff, "p: %.1f %.1f %.1f", Entity->P.x, Entity->P.y, Entity->P.z);
		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);

		sprintf(Buff, "dP: %.1f %.1f %.1f", Entity->dP.x, Entity->dP.y, Entity->dP.z);
		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);

		sprintf(Buff, "ddP: %.1f %.1f %.1f", Entity->ddP.x, Entity->ddP.y, Entity->ddP.z);
		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);

		sprintf(Buff, "Theta: %.2f", Entity->Theta);
		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);

		sprintf(Buff, "dTheta: %.2f", Entity->dTheta);
		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);

		// NOTE(Justin): 
		// dTheta > 0 -> CCW turning left
		// dTheta < 0 -> CW turning right 
		b32 TurningRight = (Entity->dTheta < 0.0f);
		b32 TurningLeft = (Entity->dTheta > 0.0f);
		if(TurningRight)
		{
			sprintf(Buff, "turning: Right");
		}
		else if(TurningLeft)
		{
			sprintf(Buff, "turning: Left");
		}
		else
		{
			sprintf(Buff, "turning: Still");
		}

		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);

		P.x -= 20.0f;
	}
	else
	{
		PushText(RenderBuffer, Text, FontInfo, EntityButton.P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}

	//
	// NOTE(Jusitn): Animation information.
	//

	sprintf(Buff, "%s", "+Animation Control");
	Text = StringCopy(&TempState->Arena, Buff);
	Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));

	ui_button Button;
	Button.ID = 2;
	Button.P = P;
	Button.Rect = Rect;

	if(InRect(Button.Rect, UI->MouseP))
	{
		UI->HotID = Button.ID;
		if(UI->ActiveID == 0 && UI->LeftClick)
		{
			UI->ActiveID = Button.ID;
		}
		else if(UI->ActiveID == Button.ID && UI->LeftClick)
		{
			UI->ActiveID = 0;
		}
		else if((UI->ActiveID != Button.ID) && UI->LeftClick)
		{
			UI->ActiveID = Button.ID;
		}
	}

	if(UI->ActiveID == Button.ID)
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, Button.P, Scale, HoverColor);
		P.y -= (Gap + dY);

		P.x += 20.0f;

		EntityMovementState(Buff, Entity);
		Text = StringCopy(&TempState->Arena, Buff);
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);
		for(animation *Animation = Player->AnimationPlayer->Channels; Animation; Animation = Animation->Next)
		{
			if(Animation)
			{
				sprintf(Buff, "Name: %s", Animation->Name.Data);
				Text = StringCopy(&TempState->Arena, Buff);
				PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
				P.y -= (Gap + dY);

				sprintf(Buff, "%s %.2f", "Duration: ", Animation->Duration);
				Text = StringCopy(&TempState->Arena, Buff);
				PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
				P.y -= (Gap + dY);

				sprintf(Buff, "%s %.2f", "t: ", Animation->CurrentTime);
				Text = StringCopy(&TempState->Arena, Buff);
				PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
				P.y -= (Gap + dY);

				sprintf(Buff, "%s %.2f", "blend duration: ", Animation->BlendDuration);
				Text = StringCopy(&TempState->Arena, Buff);
				PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
				P.y -= (Gap + dY);

				sprintf(Buff, "%s %.2f", "blend_t: ", Animation->BlendCurrentTime);
				Text = StringCopy(&TempState->Arena, Buff);
				PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
				P.y -= (Gap + dY);

				sprintf(Buff, "%s %.2f", "blend: ", Animation->BlendFactor);
				Text = StringCopy(&TempState->Arena, Buff);
				PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
				P.y -= (Gap + dY);
			}
			P.y -= (Gap + dY);
		}
		P.x -= 20.0f;
	}
	else
	{
		PushText(RenderBuffer, Text, FontInfo, Button.P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}

	sprintf(Buff, "%s", "+Texture");
	Text = StringCopy(&TempState->Arena, Buff);
	Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));

	ui_button AnimationButton;
	AnimationButton.ID = 3;
	AnimationButton.P = P;
	AnimationButton.Rect = Rect;

	if(InRect(AnimationButton.Rect, UI->MouseP))
	{
		UI->HotID = AnimationButton.ID;
		if(UI->ActiveID == 0 && UI->LeftClick)
		{
			UI->ActiveID = AnimationButton.ID;
		}
		else if(UI->ActiveID == AnimationButton.ID && UI->LeftClick)
		{
			UI->ActiveID = 0;
		}
		else if(UI->ActiveID != AnimationButton.ID && UI->LeftClick)
		{
			UI->ActiveID = AnimationButton.ID;
		}
	}

	if(UI->ActiveID != AnimationButton.ID)
	{
		PushText(RenderBuffer, Text, FontInfo, AnimationButton.P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}
	else
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, AnimationButton.P, Scale, HoverColor);
		P.y -= (Gap + dY);

		//
		// NOTE(Justin): Render to texture
		//

		// TODO(justin); Cleanup
		mat4 Persp = Mat4Perspective(GameState->FOV, 256.0f/256.0f, GameState->ZNear, GameState->ZFar);
		render_buffer *RenderToTextureBuffer = RenderBufferAllocate(&TempState->Arena, Megabyte(64),
				GameState->CameraTransform,
				Persp,
				Mat4Identity(),
				Assets,
				Camera->P,
				V3(0.0f),
				2);

		PushClear(RenderToTextureBuffer, V4(1.0f));
		mat4 Transform = EntityTransform(Entity, Entity->VisualScale);
		model *Model = LookupModel(Assets, "XBot");
		PushModel(RenderToTextureBuffer, Model, Transform);

		Platform.RenderToOpenGL(RenderToTextureBuffer, GameState->Texture.Width, GameState->Texture.Height);

		//
		// NOTE(Justin): Render to default frame buffer
		//

		f32 Width = 256.0f;
		f32 Height = 256.0f;
		P = V2(0.0f, P.y - Height);

		Rect = RectMinDim(P, V2(Width, Height));
		f32 Vertices[6][4] =
		{
			{Rect.Min.x, Rect.Min.y, 0.0f, 0.0f},
			{Rect.Max.x, Rect.Min.y, 1.0f, 0.0f},
			{Rect.Max.x, Rect.Max.y, 1.0f, 1.0f},

			{Rect.Max.x, Rect.Max.y, 1.0f, 1.0f},
			{Rect.Min.x, Rect.Max.y, 0.0f, 1.0f},
			{Rect.Min.x, Rect.Min.y, 0.0f, 0.0f},
		};

		PushRenderToTexture(RenderBuffer, (f32 *)Vertices);
	}

	Platform.RenderToOpenGL(RenderBuffer, (u32)GameInput->BackBufferWidth, (u32)GameInput->BackBufferHeight);

	// NOTE(Justin): Need to begin/end in correct order OW assert will fire. Is this desired?
	TemporaryMemoryEnd(RenderMemory);
	TemporaryMemoryEnd(AnimationMemory);
}
