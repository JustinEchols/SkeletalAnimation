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

internal void
PlayerAdd(game_state *GameState, v3 P)
{
	entity *Entity = EntityAdd(GameState, EntityType_Player);
	GameState->PlayerEntityIndex = GameState->EntityCount - 1;

	FlagAdd(Entity, EntityFlag_YSupported);
	FlagAdd(Entity, EntityFlag_Collides);

	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);

	f32 Radians = DirectionToEuler(V3(0.0f, 0.0f, -1.0f)).yaw;
	Entity->Theta = RadToDegrees(Radians);
	Entity->ThetaTarget = Entity->Theta; 
	Entity->dTheta = 0.0f;
	Entity->Orientation = Quaternion(V3(0.0f, -1.0f, 0.0f), Radians);
	Entity->MovementState = MovementState_Idle;

	//
	// AABB 
	//

	// The visual scale of the player and the visual scale of the AABBDim are unrelated so 
	// we have to scale the dim by 0.5.

	// NOTE(Justin): The AABBDim is used for collision detection AND for AABB rendering. The visual
	// scale is used to scale the player model. Therefore the visual scale and the AABBDim are unrelated
	//
	// The volume offset depends on what convention is used as far as the entity's position. The convention
	// used is that the position is where on the ground the entity is located. So we offset the volume in the
	// +y direction.

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

	// TODO(Justin): Capsule
	capsule C;
	C.Radius = Entity->Radius;
	C.Min = V3(0.0f, C.Radius, 0.0f);
	C.Max = V3(0.0f, Entity->Height - C.Radius, 0.0f);
	Entity->Capsule = C;

	// Visuals
	Entity->VisualScale = V3(0.01f);
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
		animation_player *AnimationPlayer = Entity->AnimationPlayer;
		if(AnimationPlayer && AnimationPlayer->ControlsTurning)
		{
		}
		else
		{
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
			v3 Current = Entity->Orientation *V3(0.0f, 0.0f, 1.0f);
			Entity->Theta = -1.0f*RadToDegrees(DirectionToEuler(Current).yaw);
		}
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

// NOTE(Justin): The epsilon bug was a bug in EntityMove(). The desired position was not being
// updated.
// TODO(Justin): Figure out an approach to epsilons.
// NOTE(Justin): There might be an epsilon bug when epsilon = 0.001f when colliding with an OBB. 
// Some of the time the player will tunnel through the OBB. After chaning epsilon to 0.01f;
// the player does not tunnel. It is possible that the position the player is moved to when
// using 0.001f for epsilon, ends up being behind the non-aligned plane of the OBB. If the player
// gets moved there then they tunnel through the OBB.

//f32 tEpsilon = 0.001f;
//f32 tEpsilon = 0.01f;

// NOTE(Justin): This is really PointIntersectsPlaneAndInABB.....
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
			//v3 PointOfIntersection = RelP + tResult * DeltaP;
			*tMin = Max(0.0f, tResult - tEpsilon);
			Collided = true;
		}
	}

	return(Collided);
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

		///Entity->dP = dt*DeltaP + Entity->dP;

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
			if(Entity != TestEntity)
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
		if(TestP.y < GroundY && EntityIsGrounded(Entity) && (Normal.y < 0.0f))
		{
			// NOTE(Justin): This is supposed to handle the situation when the player collides with an OBB and is underneath the OBB and on the ground and running toward
			// the corner.
			//
			//  /
			// /___   <-- O
			//
			// The collide and slide solution does not work because the new position of the player will be behind the ground plane (tunnel through the ground).
			// Setting the velocity to 0 and then breaking, we effectively are not accepting the move.
			// TODO(Justin): Robustness

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

		ArenaSubset(&GameState->Arena, &GameState->AssetManager.Arena, Kilobyte(512));
		AssetManagerInitialize(&GameState->AssetManager);
		asset_manager *Assets = &GameState->AssetManager;

		GameState->Quad = QuadDefault();
		GameState->Texture.Width = 256;
		GameState->Texture.Height = 256;

		quaternion CubeOrientation = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);

		// Region
		WalkableRegionAdd(GameState, V3(0.0f, 0.0f, -20.0f), V3(40.0f), CubeOrientation);
		WalkableRegionAdd(GameState, V3(-30.0f, 0.0f, -10.0f), V3(10.0f), CubeOrientation);

		// Player
		PlayerAdd(GameState, V3(0.0f, 0.01f, -10.0f));
		entity *Player			= GameState->Entities + GameState->PlayerEntityIndex;
		Player->AnimationPlayer = PushStruct(Arena, animation_player);
		Player->AnimationGraph	= PushStruct(Arena, animation_graph);
		model *XBot				= LookupModel(Assets, "XBot").Model;
		AnimationPlayerInitialize(Player->AnimationPlayer, XBot, Arena);
		Player->AnimationGraph	= LookupGraph(Assets, "XBot_AnimationGraph");

		// Cubes 
		v3 StartP = V3(-3.0f, 0.0f, -10.0f);
		v3 Dim = V3(2.0f, 1.0f, 10.0f);

		CubeAdd(GameState, StartP, Dim, CubeOrientation);

		Dim = V3(2.0f, 10.0f, 2.0f);
		StartP += V3(6.0f, 0.0f, -0.0f);
		CubeAdd(GameState, StartP, Dim, CubeOrientation);

		Dim = V3(2.0f, 0.5f, 2.0f);
		v3 ElevatorP = StartP + V3(0.0f, 0.01f, 2.0f);
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
		CameraSet(&GameState->Camera, Player->P + GameState->CameraOffsetFromPlayer, -90.0f, -10.0f);
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
	v3 CameraddP = {};
	game_keyboard *Keyboard = &GameInput->Keyboard;
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

	b32 Sprinting = false;
	b32 Jumping = false;
	b32 Crouching = false;
	if(!GameState->CameraIsFree)
	{
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
		if(IsDown(Keyboard->Shift))
		{
			Sprinting = true;
		}
		if(IsDown(Keyboard->Space))
		{
			Jumping = true;
		}
		if(IsDown(Keyboard->Ctrl))
		{
			Crouching = true;
		}
	}
	else
	{
		if(IsDown(Keyboard->W))
		{
			CameraddP += 1.0f*GameState->Camera.Direction;
		}
		if(IsDown(Keyboard->A))
		{
			CameraddP += -1.0f*Cross(GameState->Camera.Direction, YAxis());
		}
		if(IsDown(Keyboard->S))
		{
			CameraddP += -1.0f*GameState->Camera.Direction;
		}
		if(IsDown(Keyboard->D))
		{
			CameraddP += 1.0f*Cross(GameState->Camera.Direction, YAxis());
		}

		CameraddP = Normalize(CameraddP);
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
				f32 a = 50.0f;

				v3 OldPlayerddP = Entity->ddP;
				Entity->ddP = ddP;

				if(Sprinting)
				{
					a *= 1.5f;
				}

				if(Equal(OldPlayerddP, V3(0.0f)) &&
				  !Equal(Entity->ddP, OldPlayerddP))
				{
					FlagAdd(Entity, EntityFlag_Moving);
				}
				else
				{
					FlagClear(Entity, EntityFlag_Moving);
				}

				if(Jumping && EntityIsGrounded(Entity))
				{
					f32 X = AbsVal(Entity->dP.x);
					f32 Z = AbsVal(Entity->dP.z);
					f32 MaxComponent = Max(X, Z);

					// TODO(Justin): Handle diagonal
					if(MaxComponent == X)
					{
						Entity->dP.x += SignOf(Entity->dP.x)*10.0f;
					}
					else
					{
						Entity->dP.z += SignOf(Entity->dP.z)*10.0f;
					}


					Entity->dP.y = 30.0f;

				}

				ddP = a * ddP;
				ddP += -10.0f * Entity->dP;

				if(!Equal(ddP, V3(0.0f)))
				{
					EntityMove(GameState, Entity, ddP, dt);
					EntityOrientationUpdate(Entity, dt, 15.0f);
				}

				switch(Entity->MovementState)
				{
					case MovementState_Idle:
					{
						if(EntityIsMoving(Entity))
						{
							if(Jumping)
							{
								Entity->MovementState = MovementState_Jump;
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
						else
						{
							if(Crouching)
							{
								Entity->MovementState = MovementState_Crouch;
							}
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

							if(Crouching)
							{
								Entity->MovementState = MovementState_Sliding;
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
						FlagClear(Entity, EntityFlag_YSupported);

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
					case MovementState_Sliding:
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

				Entity->ddP = ElevatorddP;
				EntityMove(GameState, Entity, ElevatorddP, dt);
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
	AnimationPlayerUpdate(Player->AnimationPlayer, &TempState->Arena, dt);
	ModelJointsUpdate(Player);
	AnimationGraphPerFrameUpdate(Assets, Player->AnimationPlayer, Player->AnimationGraph);

	camera *Camera = &GameState->Camera;
	if(!GameState->CameraIsFree)
	{
		v3 CameraP = Player->P + GameState->CameraOffsetFromPlayer;
		CameraSet(Camera, CameraP, GameState->DefaultYaw, GameState->DefaultPitch);
	}
	else
	{
		v3 P = Camera->P + GameState->CameraSpeed*dt*CameraddP;
		Camera->P = P;

		// TODO(justin) FPS camera 
		if(IsDown(GameInput->MouseButtons[MouseButton_Left]))
		{
			CameraDirectionUpdate(Camera, GameInput->dXMouse, GameInput->dYMouse, dt);
		}
	}

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

	//
	// NOTE(Justin): Render ground quads of walkable regions.
	//

	mat4 T;
	mat4 R;
	mat4 S;
	quad GroundQuad = GameState->Quad;
	asset_entry Entry = LookupTexture(Assets, "texture_01");
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		if(Entity->Type == EntityType_WalkableRegion)
		{
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

			GroundQuad = QuadDefault();
		}
	}

	//
	// NOTE(Justin): Entities.
	//

	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				mat4 Transform = EntityTransform(Entity, Entity->VisualScale);
				model *Model = LookupModel(Assets, "XBot").Model;
				PushModel(RenderBuffer, Model, Transform);

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
				P.y += 0.1f;
				T = Mat4Translate(P);
				R = QuaternionToMat4(Entity->Orientation);
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

				model *Cube = LookupModel(Assets, "Cube").Model;
				PushTexture(RenderBuffer, Cube->Meshes[0].Texture, StringHashLookup(&Assets->TextureNames, (char *)Cube->Meshes[0].Texture->Name.Data));
				PushModel(RenderBuffer, Cube, T*R*S);

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
				v3 EntityP = Entity->P;
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
				PushModel(RenderBuffer, Sphere, T*S);

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

	sprintf(Buff, "%s", "+Joints");
	Text = StringCopy(&TempState->Arena, Buff);
	Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));

	ui_button JointButton;
	JointButton.ID = 3;
	JointButton.P = P;
	JointButton.Rect = Rect;
	if(!Button(UI, &JointButton))
	{
		PushText(RenderBuffer, Text, FontInfo, JointButton.P, Scale, DefaultColor);
		P.y -= (Gap + dY);
	}
	else
	{
		Text.Data[0] = '-';
		PushText(RenderBuffer, Text, FontInfo, JointButton.P, Scale, HoverColor);
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
