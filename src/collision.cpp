
internal void
CollisionRuleAdd(game_state *GameState, u32 A, u32 B, b32 ShouldCollide, collision_type CollisionType)
{
	if(A > B)
	{
		u32 Temp = A;
		A = B;
		B = Temp;
	}

	u32 HashIndex = A & (ArrayCount(GameState->CollisionRuleHash) - 1);
	Assert((HashIndex >= 0) && (HashIndex < ArrayCount(GameState->CollisionRuleHash)));

	pairwise_collision_rule *Found = 0;
	for(pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashIndex];
			Rule;
			Rule = Rule->NextInHash)
	{
		if((Rule->IDA == A) && (Rule->IDB == B))
		{
			Found = Rule;
			break;
		}
	}

	if(!Found)
	{
		Found = GameState->CollisionRuleFirstFree;
		if(Found)
		{
			GameState->CollisionRuleFirstFree = Found->NextInHash;
		}
		else
		{
			Found = PushStruct(&GameState->Arena, pairwise_collision_rule);
		}

		Found->NextInHash = GameState->CollisionRuleHash[HashIndex];
		GameState->CollisionRuleHash[HashIndex] = Found;
	}

	if(Found)
	{
		Found->IDA = A;
		Found->IDB = B;
		Found->ShouldCollide = ShouldCollide;
		Found->CollisionType = CollisionType;
	}
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

	// Check whether or not we started inside and adjust the MK dimensions and 
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

internal b32
SpheresIntersect(v3 CenterA, f32 RadiusA, v3 CenterB, f32 RadiusB)
{
	b32 Result = false;

	f32 RadiiSq = Square(RadiusA) + Square(RadiusB);

	v3 Rel = CenterA - CenterB;
	f32 DistSq = Dot(Rel, Rel);

	Result = DistSq <= RadiiSq;

	return(Result);
}
