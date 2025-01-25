#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "font.cpp"
#include "mesh.cpp"
#include "entity.cpp"
#include "animation.cpp"
#include "ui.cpp"
#include "asset.cpp"
#include "render.cpp"

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

	Entity->Height = 1.8f;

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

	// Visuals
	Entity->VisualScale = V3(0.01f);

	if(GameState->PlayerEntityIndex == 0)
	{
		GameState->PlayerEntityIndex = Entity->ID;
	}

	return(Entity);
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

	// NOTE(Justin): The cube mesh has dimensions 1x1x1. The AABBDim is used for collision
	// detection and the visual scale used for rendering
	//
	// The volume offset depends on what convention is used as far as the entity's position. The convention
	// used is that the position is where on the ground the entity is located. So we offset the volume in the
	// +y direction.

	// ABB
	Entity->AABBDim = 0.99f*Dim;
	Entity->VolumeOffset = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);

	// OBB
	quaternion Q = Conjugate(Entity->Orientation);
	Entity->OBB.Center = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);
	Entity->OBB.X = Q * XAxis();
	Entity->OBB.Y = Q * YAxis();
	Entity->OBB.Z = Q * (-1.0f*ZAxis());
	Entity->OBB.Dim = 0.99f*Dim;

	Entity->VisualScale = 0.5f*Dim;
}

internal void
SphereAdd(game_state *GameState, v3 Center, f32 Radius)
{
	entity *Entity = EntityAdd(GameState, EntityType_Sphere);

	FlagAdd(Entity, EntityFlag_YSupported);
	FlagAdd(Entity, EntityFlag_Collides);

	Entity->P = Center;
	Entity->dP = {};
	Entity->ddP = {};
	Entity->Orientation = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);

	Entity->VisualScale = V3(1.0f);
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

	// ABB
	Entity->AABBDim = 0.99f*Dim;
	Entity->VolumeOffset = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);

	// OBB
	quaternion Q = Conjugate(Entity->Orientation);
	Entity->OBB.Center = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);
	Entity->OBB.X = Q * XAxis();
	Entity->OBB.Y = Q * YAxis();
	Entity->OBB.Z = Q * (-1.0f*ZAxis());
	Entity->OBB.Dim = 0.99f*Dim;

	Entity->VisualScale = 0.5f*Dim;
}

internal void
ElevatorAdd(game_state *GameState, v3 P, v3 Dim, quaternion Orientation)
{
	entity *Entity = EntityAdd(GameState, EntityType_Elevator);

	FlagAdd(Entity, EntityFlag_Moveable);
	FlagAdd(Entity, EntityFlag_Collides);

	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Orientation = Orientation;

	// ABB
	Entity->AABBDim = 0.99f*Dim;
	Entity->VolumeOffset = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);

	// OBB
	quaternion Q = Conjugate(Entity->Orientation);
	Entity->OBB.Center = 0.5f*V3(0.0f, Entity->AABBDim.y, 0.0f);
	Entity->OBB.X = Q * XAxis();
	Entity->OBB.Y = Q * YAxis();
	Entity->OBB.Z = Q * (-1.0f*ZAxis());
	Entity->OBB.Dim = 0.99f*Dim;

	Entity->VisualScale = 0.5f*Dim;
}

inline void 
MovementStateToString(char *Buffer, movement_state State)
{
	switch(State)
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
		case MovementState_Sliding:
		{
			sprintf(Buffer, "%s", "MovementState: Sliding");
		} break;
		case MovementState_Attack:
		{
			sprintf(Buffer, "%s", "MovementState: Attack");
		} break;
	}
}

inline void
EntityOrientationUpdate(entity *Entity, f32 dt, f32 AngularSpeed)
{
	v3 FacingDirection = Entity->dP;

	f32 TargetAngleInRad = DirectionToEuler(FacingDirection).yaw;
	Entity->ThetaTarget = RadToDegrees(TargetAngleInRad);
	if(Entity->Theta != Entity->ThetaTarget)
	{
		//animation_player *AnimationPlayer = Entity->AnimationPlayer;
		//if(!AnimationPlayer->ControlsTurning && (AnimationPlayer->RootTurningAccumulator == 0.0f))
		//{
		//	// NOTE(Justin): If the animation player does not control turning keep recording
		//	// the player's orientation. When the animation player decides to control turning
		//	// the orientation that we start playing the animation at is the last recorded orientation
		//	// of the player!
		//	AnimationPlayer->OrientationLockedAt = Entity->Orientation;
		//}

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

internal v3
ClosestPointOnOBB(obb OBB, v3 WorldPosition, v3 P)
{
	v3 ClosestPoint;

	v3 Center = WorldPosition + OBB.Center;
	v3 CenterToP = P - Center;

	f32 X = Dot(CenterToP, OBB.X);
	f32 Y = Dot(CenterToP, OBB.Y);
	f32 Z = Dot(CenterToP, OBB.Z);

	f32 HalfDimX = 0.5f*OBB.Dim.E[0];
	f32 HalfDimY = 0.5f*OBB.Dim.E[1];
	f32 HalfDimZ = 0.5f*OBB.Dim.E[2];

	X = Clamp(-HalfDimX, X, HalfDimX);
	Y = Clamp(-HalfDimY, Y, HalfDimY);
	Z = Clamp(-HalfDimZ, Z, HalfDimZ);

	// NOTE(Justin): If P is already inside the OBB,
	// then we have to adjust either X, Y, or Z. Otherwise
	// the closest point returned will be the original point
	// itself.

	if(InAABB(AABBCenterDim(Center, OBB.Dim), P))
	{
		f32 DistanceX = HalfDimX - AbsVal(X);
		f32 DistanceY = HalfDimY - AbsVal(Y);
		f32 DistanceZ = HalfDimZ - AbsVal(Z);

		f32 Min = Min3(DistanceX, DistanceY, DistanceZ);
		if(Min == DistanceX)
		{
			X = (X >= 0.0f) ? HalfDimX : -HalfDimX;
		}
		else if(Min == DistanceY)
		{
			Y = (Y >= 0.0f) ? HalfDimY : -HalfDimY;
		}
		else
		{
			Z = (Z >= 0.0f) ? HalfDimZ : -HalfDimZ;
		}
	}

	ClosestPoint = Center + X*OBB.X + Y*OBB.Y + Z*OBB.Z;

	return(ClosestPoint);
}

internal v3
ClosestPointOnLineSegment(v3 A, v3 B, v3 P)
{
	v3 ClosestPoint;

	v3 Delta = B - A;
	f32 t = Dot(P - A, Delta) / Dot(Delta, Delta);
	t = Clamp01(t);

	ClosestPoint = A + t*Delta;

	return(ClosestPoint);
}

internal v3
ClosestPointOnPlane(v3 PlaneNormal, v3 PointOnPlane, v3 P)
{
	v3 ClosestPoint;

	ClosestPoint = P - Dot(PlaneNormal, P - PointOnPlane) * PlaneNormal;

	return(ClosestPoint);
}

internal collision_result
AABBCollisionInfo(aabb AABB)
{
	collision_result Result;

	v3 ZeroVector = V3(0.0f);

	// NOTE(Jusitn): Left face
	Result.Info[0].PlaneNormal = {-1.0f, 0.0f, 0.0f};
	Result.Info[0].PointOnPlane = AABB.Min;
	Result.Info[0].PointOfIntersection = ZeroVector;
	Result.Info[0].tResult = F32Max;

	// NOTE(Jusitn): Right face
	Result.Info[1].PlaneNormal = {1.0f, 0.0f, 0.0f};
	Result.Info[1].PointOnPlane = AABB.Max;
	Result.Info[1].PointOfIntersection = ZeroVector;
	Result.Info[1].tResult = F32Max;

	// NOTE(Jusitn): Back face
	Result.Info[2].PlaneNormal = {0.0f, 0.0f, -1.0f};
	Result.Info[2].PointOnPlane = AABB.Max;
	Result.Info[2].PointOfIntersection = ZeroVector;
	Result.Info[2].tResult = F32Max;

	// NOTE(Jusitn): Front face
	Result.Info[3].PlaneNormal = {0.0f, 0.0f, 1.0f};
	Result.Info[3].PointOnPlane = AABB.Min;
	Result.Info[3].PointOfIntersection = ZeroVector;
	Result.Info[3].tResult = F32Max;

	// NOTE(Jusitn): Top face
	Result.Info[4].PlaneNormal = {0.0f, 1.0f, 0.0f};
	Result.Info[4].PointOnPlane = AABB.Max;
	Result.Info[4].PointOfIntersection = ZeroVector;
	Result.Info[4].tResult = F32Max;

	// NOTE(Jusitn): Bottom face
	Result.Info[5].PlaneNormal = {0.0f, -1.0f, 0.0f};
	Result.Info[5].PointOnPlane = AABB.Min;
	Result.Info[5].PointOfIntersection = ZeroVector;
	Result.Info[5].tResult = F32Max;

	return(Result);
}

// TODO(Justin): Figure out an approach to epsilons.
internal b32
PointAndPlaneIntersect(v3 RelP, v3 DeltaP, v3 PlaneNormal, f32 D, aabb MKSumAABB, f32 *tMin, f32 tEpsilon = 0.001f)
{
	b32 Collided = false;
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

internal b32
RayAndPlaneIntersect(v3 RelP, v3 DeltaP, v3 PlaneNormal, f32 D, f32 *tMin, f32 tEpsilon = 0.01f)
{
	b32 Collided = false;
	if(!Equal(DeltaP, V3(0.0f)))
	{
		f32 tResult = (D - Dot(PlaneNormal, RelP)) / Dot(PlaneNormal, DeltaP);
		if((tResult >= 0.0f) && tResult < *tMin)
		{
			*tMin = Max(0.0f, tResult - tEpsilon);
			Collided = true;
		}
	}

	return(Collided);
}

// TODO(Justin): Correct this..
inline v3 
CapsuleSphereCenterVsOBB(v3 CapsuleP, capsule Capsule, v3 OBBP, obb OBB)
{
	v3 Result = {};

	v3 A = CapsuleP + Capsule.Min;
	v3 B = CapsuleP + Capsule.Max;

	v3 OBBPointClosestToA = ClosestPointOnOBB(OBB, OBBP, A);
	v3 OBBPointClosestToB = ClosestPointOnOBB(OBB, OBBP, B);

	v3 DeltaA = OBBPointClosestToA - A;
	v3 DeltaB = OBBPointClosestToB - B;

	f32 dA = Dot(DeltaA, DeltaA);
	f32 dB = Dot(DeltaB, DeltaB);

	v3 Closest;
	if(dA < dB)
	{
		Closest = OBBPointClosestToA;
	}
	else
	{
		Closest = OBBPointClosestToB;
	}

	Result = ClosestPointOnLineSegment(A, B, Closest);
	
	return(Result);
}

internal b32
MovingSphereHitOBB(v3 RelP, f32 Radius, v3 DeltaP, obb OBB, v3 *DestNormal, f32 *tMin, f32 tEpsilon = 0.01f)
{
	b32 Collided = false;

	// Write sphere center in OBB space. C = O + aX + bY + cZ. Note that the origin is 0.
	v3 SphereCenter;
	SphereCenter.x = Dot(RelP, OBB.X);
	SphereCenter.y = Dot(RelP, OBB.Y);
	SphereCenter.z = Dot(RelP, OBB.Z);

	// Write delta vector in OBB space. D = aX + bY + cZ
	v3 Delta;
	Delta.x = Dot(DeltaP, OBB.X);
	Delta.y = Dot(DeltaP, OBB.Y);
	Delta.z = Dot(DeltaP, OBB.Z);

	b32 StartedInside = false;

	// Check whether or not we started inside and adjust the MK dimensions. Also
	// flip the plane normal whenever there is a collision (see below).
	v3 MKDim;
	if(InAABB(AABBCenterDim(V3(0.0f), OBB.Dim), SphereCenter))
	{
		StartedInside = true;
		MKDim = -2.0f*V3(Radius) + OBB.Dim;
	}
	else
	{
		MKDim = 2.0f*V3(Radius) + OBB.Dim;
	}

	// Construct the MK sum. It is an AABB with center 0 that is expanded by the radius r.
	aabb MKSumAABB = AABBCenterDim(V3(0.0f), MKDim);
	collision_result CollisionResult = AABBCollisionInfo(MKSumAABB);
	for(u32 InfoIndex = 0; InfoIndex < ArrayCount(CollisionResult.Info); ++InfoIndex)
	{
		collision_info Info = CollisionResult.Info[InfoIndex];
		v3 PlaneNormal = Info.PlaneNormal;
		v3 PointOnPlane = Info.PointOnPlane;
		f32 D = Dot(PlaneNormal, PointOnPlane);
		if(PointAndPlaneIntersect(SphereCenter, Delta, PlaneNormal, D, MKSumAABB, tMin, tEpsilon))
		{
			// TODO(Justin): Voronoi region check for full sphere swept vs OBB collision detection.

			if(PlaneNormal.x == 1.0f)	*DestNormal =	    OBB.X;
			if(PlaneNormal.x == -1.0f)	*DestNormal = -1.0f*OBB.X; 
			if(PlaneNormal.y == 1.0f)	*DestNormal =	    OBB.Y;
			if(PlaneNormal.y == -1.0f)	*DestNormal = -1.0f*OBB.Y;
			if(PlaneNormal.z == 1.0f)	*DestNormal =	    OBB.Z;
			if(PlaneNormal.z == -1.0f)	*DestNormal = -1.0f*OBB.Z;

			Collided = true;
		}
	}

	if(StartedInside && Collided)
	{
		*DestNormal *= -1.0f;
	}

	return(Collided);
}


inline b32
EntitiesCanCollide(entity *Mover, entity *A)
{
	b32 Result = false;
	if(FlagIsSet(Mover, EntityFlag_Collides) && FlagIsSet(A, EntityFlag_Collides))
	{
		// TODO(Justin): Additional testing to see if they can collide. E.g.
		// Both entities may be able to collide with each other but based
		// on the positions and velocities they may not. In this case
		// we should early out.
		Result = true;
	}

	return(Result);
}

#define PLAYER_COLLIDER_AABB 0
#define PLAYER_COLLIDER_OBB 0
#define PLAYER_COLLIDER_SPHERE 0
#define PLAYER_COLLIDER_CAPSULE 1
#define PLAYER_COLLIDER_OFF 0

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
	b32 HitSomethingBesidesRegion = false;
	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		b32 Collided = false;

		f32 tMin = 1.0f;
		f32 tGround = 1.0f;

		v3 Normal = {};
		v3 GroundNormal = {};

		entity *HitEntity = 0;
		entity *Region = 0;

		DesiredP = Entity->P + DeltaP;
		for(u32 TestEntityIndex = 0; TestEntityIndex < GameState->EntityCount; ++TestEntityIndex)
		{
			entity *TestEntity = GameState->Entities + TestEntityIndex;
			if(Entity->ID != TestEntity->ID)
			{
				if(EntitiesCanCollide(Entity, TestEntity))
				{
					v3 TestP = TestEntity->P;
					v3 CurrentP = Entity->P; 
#if PLAYER_COLLIDER_AABB
					v3 RelP		= (CurrentP + Entity->VolumeOffset) - (TestP + TestEntity->VolumeOffset);
					v3 AABBDim	= Entity->AABBDim + TestEntity->AABBDim;

					aabb MKSumAABB = AABBCenterDim(V3(0.0f), AABBDim);
					collision_result CollisionResult = CollisionInfoDefault(MKSumAABB);
					for(u32 InfoIndex = 0; InfoIndex < ArrayCount(CollisionResult.Info); ++InfoIndex)
					{
						collision_info Info = CollisionResult.Info[InfoIndex];
						v3 PlaneNormal = Info.PlaneNormal;
						v3 PointOnPlane = Info.PointOnPlane;
						f32 D = Dot(PlaneNormal, PointOnPlane);
						if(PointAndPlaneIntersect(RelP, DeltaP, PlaneNormal, D, MKSumAABB, &tMin))
						{
							Normal = Info.PlaneNormal;
							Collided = true;
							HitEntity = TestEntity;
						}
					}
#elif PLAYER_COLLIDER_SPHERE 
					// Compute the delta from the OBB's center to the Sphere's center in XYZ space.
					v3 RelP		= (CurrentP + V3(0.0f, Entity->Radius, 0.0f)) - (TestP + TestEntity->VolumeOffset);
					if(MovingSphereHitOBB(RelP, Entity->Radius, DeltaP, TestEntity->OBB, &Normal, &tMin))
					{
						Collided = true;
						HitEntity = TestEntity;
					}

					// GroundCheck
					if(MovingSphereHitOBB(RelP, 0.8f*Entity->Radius, GroundDelta, TestEntity->OBB, &GroundNormal, &tGround))
					{
						EntityBelow = TestEntity;
					}
#elif PLAYER_COLLIDER_CAPSULE
					// Compute the center of the sphere along the capsule axis and then convert it to OBB space.
					// TODO(Justin): Make sure the chosen sphere center is correct. It seems ok..
					v3 SphereCenter = CapsuleSphereCenterVsOBB(CurrentP, Entity->Capsule, TestEntity->P, TestEntity->OBB);
					v3 SphereRel = SphereCenter - (TestP + TestEntity->VolumeOffset);
					if(MovingSphereHitOBB(SphereRel, Entity->Radius, DeltaP, TestEntity->OBB, &Normal, &tMin))
					{
						Collided = true;
						HitEntity = TestEntity;
					}

					// GroundCheck
					v3 RelP		= (CurrentP + V3(0.0f, Entity->Radius, 0.0f)) - (TestP + TestEntity->VolumeOffset);
					if(MovingSphereHitOBB(RelP, Entity->Radius, GroundDelta, TestEntity->OBB, &GroundNormal, &tGround))
					{
						EntityBelow = TestEntity;
					}
#elif PLAYER_COLLIDER_OFF
#endif

					if(TestEntity->Type == EntityType_WalkableRegion)
					{
						if(InAABB(AABBCenterDim(TestEntity->P, TestEntity->AABBDim), CurrentP))
						{
							Region = TestEntity;
						}
					}
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

inline void
DebugDrawOBB(render_buffer *RenderBuffer, model *DebugCube, obb OBB, v3 P, v3 Offset, v3 Color)
{
	v3 X = OBB.X;
	v3 Y = OBB.Y;
	v3 Z = OBB.Z;

	mat4 T = Mat4Translate(P + Offset);
	mat4 R = Mat4(X, Y, Z);
	mat4 S = Mat4Scale(OBB.Dim);

	PushAABB(RenderBuffer, DebugCube, T*R*S, Color);
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

		//Dim = V3(2.0f, 0.5f, 2.0f);
		//v3 ElevatorP = StartP + V3(0.0f, 0.01f, 2.0f);
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
		GameState->Camera.RotationAboutY = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);
		CameraTransformUpdate(GameState);

		GameState->TimeScale = 1.0f;
		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)GameInput->BackBufferWidth / (f32)GameInput->BackBufferHeight;
		GameState->ZNear = 0.2f;
		GameState->ZFar = 200.0f;
		GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);
		GameState->Gravity = 80.0f;

		// NOTE(Justin): Initialize a default capsule st it can be used for any entity.
		capsule Cap = CapsuleMinMaxRadius(V3(0.0f, 0.4f, 0.0f), V3(0.0f, 1.8f - 0.4f, 0.0f), 0.4f);

		GameState->Capsule = PushArray(Arena, 1, model);
		model *Capsule = GameState->Capsule;
		*Capsule	= DebugModelCapsuleInitialize(Arena, Cap.Min, Cap.Max, Cap.Radius);

		GameState->Cube = PushArray(Arena, 1, model);
		model *Cube = GameState->Cube;
		*Cube = DebugModelCubeInitialize(Arena);

		GameState->Sphere = PushArray(Arena, 1, model);
		model *Sphere = GameState->Sphere;
		*Sphere = DebugModelSphereInitialize(Arena, 0.5f);

		// Player
#if 0
		model *Model = LookupModel(&GameState->AssetManager, "YBot").Model;
		animation_graph *G  = LookupGraph(&GameState->AssetManager, "YBot_AnimationGraph");
		asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "YBot_FightIdleRight");
#else
		model *Model = LookupModel(&GameState->AssetManager, "XBot").Model;
		animation_graph *G  = LookupGraph(&GameState->AssetManager, "XBot_AnimationGraph");
		asset_entry Entry = LookupSampledAnimation(&GameState->AssetManager, "XBot_IdleRight");
#endif

		entity *Player = PlayerAdd(GameState, V3(0.0f, 0.01f, -5.0f));
		Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
		Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);
		Player->Acceleration = 50.0f;
		Player->Drag = 10.0f;
		Player->AngularSpeed = 15.0f;

		AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
		Player->AnimationGraph = G;
		AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlags_Looping, 0.2f);
		GameState->PlayerIDForController[0] = Player->ID;

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
		entity *Entity = GameState->Entities + GameState->PlayerEntityIndex;

		animation_graph *G = LookupGraph(&GameState->AssetManager, "XBot_AnimationGraph");
		ArenaClear(&G->Arena);
		G->NodeCount = 0;
		G->Index = 0;
		G->CurrentNode = {};
		MemoryZero(&G->Nodes, sizeof(G->Nodes));

		AnimationGraphInitialize(G, "../src/XBot_AnimationGraph.animation_graph");
		Entity->AnimationGraph = G;
	}
#endif

	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(GameInput->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = ControllerGet(GameInput, ControllerIndex);
		u32 ID = GameState->PlayerIDForController[ControllerIndex];
		if(ID == 0)
		{
			if(Controller->Start.EndedDown)
			{
				model *Model = 0;
				animation_graph *G = 0;
				asset_entry Entry = {};

				Model = LookupModel(&GameState->AssetManager, "XBot").Model;
				G  = LookupGraph(&GameState->AssetManager, "XBot_AnimationGraph");
				Entry = LookupSampledAnimation(&GameState->AssetManager, "XBot_IdleRight");

				entity *Player = PlayerAdd(GameState, V3(0.0f, 0.01f, -10.0f));
				Player->AnimationPlayer = PushStruct(&GameState->Arena, animation_player);
				Player->AnimationGraph	= PushStruct(&GameState->Arena, animation_graph);
				Player->Acceleration = 50.0f;
				Player->Drag = 10.0f;
				Player->AngularSpeed = 15.0f;

				AnimationPlayerInitialize(Player->AnimationPlayer, Model, &GameState->Arena);
				Player->AnimationGraph = G;
				AnimationPlay(Player->AnimationPlayer, Entry.SampledAnimation, Entry.Index, AnimationFlags_Looping, 0.2f);
				GameState->PlayerIDForController[ControllerIndex] = Player->ID;
			}
		}
		else
		{
			entity *Entity = GameState->Entities + GameState->PlayerIDForController[ControllerIndex];
			move_info *MoveInfo = &Entity->MoveInfo;
			*MoveInfo = {};
			f32 Acceleration = 0.0f;

			// NOTE(Justin): For debug camera
			if(GameState->CameraIsFree) continue;

			b32 Sprinting = false;
			if(Controller->IsAnalog)
			{
				MoveInfo->ddP = V3(Controller->StickAverageX, 0.0f, -1.0f*Controller->StickAverageY);

				if(IsDown(Controller->ActionDown) && IsGrounded(Entity))
				{
					MoveInfo->CanJump = true;
					MoveInfo->AnyAction = true;
				}

				if(IsDown(Controller->ActionLeft))
				{
					MoveInfo->Attacking = true;
					MoveInfo->AnyAction = true;
				}
				
				Acceleration = Dot(MoveInfo->ddP, MoveInfo->ddP);
				if(Acceleration >= 1.0f)
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

				Acceleration = Dot(MoveInfo->ddP, MoveInfo->ddP);

			}

			if(Acceleration != 0.0f)
			{
				MoveInfo->ddP = Normalize(MoveInfo->ddP);
				MoveInfo->AnyAction = true;
				MoveInfo->Accelerating = true;
			}

			if(Sprinting && MoveInfo->Accelerating)
			{
				MoveInfo->CanSprint = true;
			}

			if(!Sprinting && MoveInfo->Accelerating)
			{
				//MoveInfo.CanRun = true;
			}

			if(Equal(Entity->dP, V3(0.0f)))
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
	dt *= GameState->TimeScale;
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
					case MovementState_Sliding:	EvaluateSliding(Entity, MoveInfo); break;
					case MovementState_Attack:	EvaluateAttack(Entity, MoveInfo, dt); break;
					case MovementState_InAir:	EvaluateInAir(Entity, MoveInfo); break;
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

		if(!Equal(ddP, V3(0.0f)) && FlagIsSet(Entity, EntityFlag_Moving))
		{
			if(CanMove(Entity))
			{
				EntityMove(GameState, Entity, ddP, dt);
				EntityOrientationUpdate(Entity, dt, Entity->AngularSpeed);
			}
		}
	}

	asset_manager *Assets = &GameState->AssetManager;



	temporary_memory AnimationMemory = TemporaryMemoryBegin(&TempState->Arena);
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

	temporary_memory RenderMemory = TemporaryMemoryBegin(&TempState->Arena);
	render_buffer *RenderBuffer = RenderBufferAllocate(&TempState->Arena, Megabyte(512),
														GameState->CameraTransform,
														GameState->Perspective,
														LightTransform,
														Assets,
														Camera->P,
														LightDir);

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

				if(!AnimationPlayer->ControlsTurning && (AnimationPlayer->RootTurningAccumulator == 0.0f))
				{
					// NOTE(Justin): If the animation player does not control turning keep recording
					// the player's orientation. When the animation player decides to control turning
					// the orientation that we start playing the animation at is the last recorded orientation
					// of the player!
					AnimationPlayer->OrientationLockedAt = Entity->Orientation;
				}

				if(AnimationPlayer->ControlsTurning)
				{
					R = QuaternionToMat4(AnimationPlayer->OrientationLockedAt);
				}
				else
				{
					if(AnimationPlayer->RootTurningAccumulator != 0.0f)
					{
						//Target = RotateTowards(AnimationPlayer->OrientationLockedAt, Entity->Orientation, dt, 1.0f);
						AnimationPlayer->OrientationLockedAt = RotateTowards(AnimationPlayer->OrientationLockedAt, Entity->Orientation, dt, 5.0f);
						R = QuaternionToMat4(AnimationPlayer->OrientationLockedAt);
						AnimationPlayer->RootTurningAccumulator = 0.0f;
					}
				}

				PushModel(RenderBuffer, CString(AnimationPlayer->Model->Name),T*R*S);


				// TODO(Justin): Move debug diagrams to ui.

				//
				// Debug volume
				//

#if PLAYER_COLLIDER_AABB
				T = Mat4Translate(Entity->P + Entity->VolumeOffset);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(Entity->AABBDim);
				PushAABB(RenderBuffer, GameState->Cube, T*R*S, V3(1.0f));
#elif PLAYER_COLLIDER_OBB
				T = Mat4Translate(Entity->P + Entity->VolumeOffset);
				R = Mat4(Entity->OBB.X, Entity->OBB.Y, Entity->OBB.Z);
				S = Mat4Scale(Entity->OBB.Dim);
				PushAABB(RenderBuffer, GameState->Cube, T*R*S, V3(1.0f));
#elif PLAYER_COLLIDER_SPHERE
				T = Mat4Translate(Entity->P + V3(0.0f, Entity->Radius, 0.0f));
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(1.0f);
				PushAABB(RenderBuffer, GameState->Sphere, T*R*S, V3(1.0f));
#elif PLAYER_COLLIDER_CAPSULE
				capsule Capsule = Player->Capsule;
				T = Mat4Translate(Entity->P + CapsuleCenter(Capsule));
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(1.0f);
				//PushCapsule(RenderBuffer, GameState->Capsule, T*R*S, V3(1.0f));
#endif

				//
				// Ground arrow 
				//

				v3 P = Entity->P;
				P.y += 0.01f;
				T = Mat4Translate(P);
				//R = QuaternionToMat4(Entity->Orientation);
				R = QuaternionToMat4(AnimationPlayer->OrientationLockedAt);
				S = Mat4Scale(V3(0.5f));

				Entry = LookupTexture(Assets, "left_arrow");
				PushTexture(RenderBuffer, Entry.Texture, Entry.Index);
				PushQuad3D(RenderBuffer, GameState->Quad.Vertices, T*R*S, Entry.Index);
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
				// OBB
				//

				DebugDrawOBB(RenderBuffer, GameState->Cube, Entity->OBB, Entity->P, Entity->VolumeOffset, V3(1.0f));

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
				v3 Dim = Entity->AABBDim;
				T = Mat4Translate(P);
				R = Mat4Identity();
				S = Mat4Scale(Dim.x);

				for(u32 Index = 0; Index < ArrayCount(GroundQuad.Vertices); ++Index)
				{
					GroundQuad.Vertices[Index].UV *= 0.5f*Dim.x;
				}

				PushTexture(RenderBuffer, Entry.Texture, Entry.Index);
				PushQuad3D(RenderBuffer, GroundQuad.Vertices, T*R*S, Entry.Index);

				T = Mat4Translate(Entity->P + Entity->VolumeOffset);
				S = Mat4Scale(Entity->AABBDim);
				PushAABB(RenderBuffer, GameState->Cube, T*S, V3(1.0f));
			} break;
			case EntityType_Sphere:
			{
				v3 EntityP = Entity->P;
				EntityP.y += Entity->Radius;
				T = Mat4Translate(EntityP);
				S = Mat4Scale(0.5f);

				model *Sphere = LookupModel(Assets, "Sphere").Model;
				PushTexture(RenderBuffer, Sphere->Meshes[0].Texture, StringHashLookup(&Assets->TextureNames, (char *)Sphere->Meshes[0].Texture->Name.Data));
				//PushModel(RenderBuffer, Sphere, T*S);

				T = Mat4Translate(EntityP);
				S = Mat4Scale(1.0f);
				PushAABB(RenderBuffer, GameState->Sphere, T*S, V3(1.0f));
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
	// NOTE(Justin): Test font/ui.
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

	if(GameState->CameraIsFree)
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

		// TODO(justin) FPS camera 
		if(IsDown(GameInput->MouseButtons[MouseButton_Left]))
		{
			CameraDirectionUpdate(Camera, GameInput->dXMouse, GameInput->dYMouse, dt);
		}
	}

	entity *Entity = GameState->Entities + GameState->PlayerEntityIndex;

	font *FontInfo =  &Assets->Font;
	f32 Scale = FontInfo->Scale;
	f32 Gap = FontInfo->LineGap;
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
	UiBegin(UI, GameInput, Assets);

	char Buff[256];
	string Text;

	sprintf(Buff, "%s %.2f", "fps: ", GameInput->FPS);
	Text = StringCopy(&TempState->Arena, Buff);
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

	if(Button(UI, &EntityButton))
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

		sprintf(Buff, "ThetaTarget: %.2f", Entity->ThetaTarget);
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

		b32 TurningLeft = (Entity->dTheta > 0.0f);
		b32 TurningRight = (Entity->dTheta < 0.0f);
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

		
		if(FlagIsSet(Player, EntityFlag_YSupported))
		{
			sprintf(Buff, "y supported: true");
		}
		else
		{
			sprintf(Buff, "y supported: false");
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

	ui_button AnimationButton;
	AnimationButton.ID = 2;
	AnimationButton.P = P;
	AnimationButton.Rect = Rect;
	if(Button(UI, &AnimationButton))
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, AnimationButton.P, Scale, HoverColor);
		P.y -= (Gap + dY);

		P.x += 20.0f;

		sprintf(Buff, "RootMotionAccumulator: %.1f %.1f %.1f",	   Player->AnimationPlayer->RootMotionAccumulator.x,
														   Player->AnimationPlayer->RootMotionAccumulator.y,
														   Player->AnimationPlayer->RootMotionAccumulator.z);
		Text = StringCopy(&TempState->Arena, Buff);
		if(Player->AnimationPlayer->ControlsPosition)
		{
			PushText(RenderBuffer, Text, FontInfo, P, Scale, HoverColor);
			P.y -= (Gap + dY);
		}
		else
		{
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
			P.y -= (Gap + dY);
		}

		sprintf(Buff, "RootTurningAccumulator: %f", Player->AnimationPlayer->RootTurningAccumulator);
		Text = StringCopy(&TempState->Arena, Buff);
		if(Player->AnimationPlayer->ControlsTurning)
		{
			PushText(RenderBuffer, Text, FontInfo, P, Scale, HoverColor);
			P.y -= (Gap + dY);
		}
		else
		{
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
			P.y -= (Gap + dY);
		}

		MovementStateToString(Buff, Entity->AnimationPlayer->MovementState);
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
		PushText(RenderBuffer, Text, FontInfo, AnimationButton.P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}

	sprintf(Buff, "+DrawHandAndFootPositions");
	Text = StringCopy(&TempState->Arena, Buff);
	Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));
	ui_button HandAndFoot;
	HandAndFoot.ID = __LINE__;
	HandAndFoot.P = P;
	HandAndFoot.Rect = Rect;

	if(Button(UI, &HandAndFoot))
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, P, Scale, HoverColor);

		T = EntityTransform(Entity, Entity->VisualScale);
		R = Mat4Identity();
		S = Mat4Scale(0.2f);

		T = Mat4Translate(Entity->LeftFootP);
		PushAABB(RenderBuffer, GameState->Sphere, T*R*S, V3(1.0f));

		T = Mat4Translate(Entity->RightFootP);
		PushAABB(RenderBuffer, GameState->Sphere, T*R*S, V3(1.0f));

		T = Mat4Translate(Entity->LeftHandP);
		PushAABB(RenderBuffer, GameState->Sphere, T*R*S, V3(1.0f));

		T = Mat4Translate(Entity->RightHandP);
		PushAABB(RenderBuffer, GameState->Sphere, T*R*S, V3(1.0f));
		P.y -= (Gap + dY);
	}
	else
	{
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}

	sprintf(Buff, "%s", "+ShadowMapTexture");
	Text = StringCopy(&TempState->Arena, Buff);
	Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));

	ui_button TextureButton;
	TextureButton.ID = 4;
	TextureButton.P = P;
	TextureButton.Rect = Rect;
	if(!Button(UI, &TextureButton))
	{
		PushText(RenderBuffer, Text, FontInfo, TextureButton.P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}
	else
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, TextureButton.P, Scale, HoverColor);
		P.y -= (Gap + dY);

		//
		// NOTE(Justin): Render to texture
		//

		// TODO(Justin); Cleanup
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
		model *Model = LookupModel(Assets, "XBot").Model;
		//PushModel(RenderToTextureBuffer, Model, Transform);

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
