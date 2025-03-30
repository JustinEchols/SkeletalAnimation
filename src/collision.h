#if !defined(COLLISION_H)

enum collision_type
{
	CollisionType_None,
	CollisionType_MovingCapsuleOBB,
	CollisionType_MovingCapsuleMovingCapsule,
};

enum collision_volume_type
{
	CollisionVolumeType_AABB,
	CollisionVolumeType_OBB,
	CollisionVolumeType_Sphere,
	CollisionVolumeType_Capsule,
};

struct collision_info
{
	v3 PlaneNormal;
	v3 PointOnPlane;
	v3 PointOfIntersection;
	f32 tResult;
};

struct collision_result
{
	collision_info Info[6];
};

struct pairwise_collision_rule
{
	u32 IDA;
	u32 IDB;
	b32 ShouldCollide;
	collision_type CollisionType;
	pairwise_collision_rule *NextInHash;
};

struct collision_volume
{
	collision_volume_type Type;
	v3 Offset;
	v3 Center;
	v3 Dim;
	v3 X;
	v3 Y;
	v3 Z;
	v3 Min;
	v3 Max;
	f32 Radius;
};

struct collision_group
{
	u32 VolumeCount;
	collision_volume *Volumes;
};

#define COLLISION_H
#endif
