#if !defined(GAME_H)

/*
 * Animation
 *	- Jumping
 *	- Landing
 *	- Combat
 *	- Multiple controls turning animations
 *
 * Physics
 *	- Moving Capsule vs Moving OBB
 *	- Moving Capsule vs Moving Capsule
 *
*/

#if !RELEASE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "platform.h"
#include "memory.h"

enum attack_type
{
	AttackType_None,
	AttackType_Neutral,
	AttackType_Forward,
	AttackType_Dash,
	AttackType_Air,
};

struct attack
{
	attack_type Type;
	f32 Timer;
	f32 CurrentTime;
	f32 Duration;
	f32 Power;
};

enum movement_state
{
	MovementState_Idle,
	MovementState_Run,
	MovementState_Sprint,
	MovementState_Jump,
	MovementState_Crouch,
	MovementState_Sliding,
	MovementState_Attack,
	MovementState_InAir,
};

enum entity_type
{
	EntityType_Null,
	EntityType_Player,
	EntityType_Cube,
	EntityType_Sphere,
	EntityType_WalkableRegion,
	EntityType_Elevator,
};

enum entity_flag
{
	EntityFlag_Collides = (1 << 1),
	EntityFlag_YSupported = (1 << 2),
	EntityFlag_Moveable = (1 << 3),
	EntityFlag_Collided = (1 << 4),
	EntityFlag_Moving = (1 << 5),
	EntityFlag_Attacking = (1 << 6),
};


#include "intrinsics.h"
#include "math.h"
#include "strings.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"
#include "animation.h"
#include "ui.h"
#include "asset.h"
#include "render.h"


struct move_info
{
	b32 AnyAction;
	b32 StandingStill;
	b32 CanSprint;
	b32 CanJump;
	b32 Crouching;
	b32 Attacking;
	b32 NoVelocity;
	b32 Accelerating;
	f32 Speed;

	v3 ddP;
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

enum collision_type
{
	CollisionType_None,
	CollisionType_MovingCapsuleOBB,
	CollisionType_MovingCapsuleMovingCapsule,
};

struct pairwise_collision_rule
{
	u32 IDA;
	u32 IDB;
	b32 ShouldCollide;
	collision_type CollisionType;
	pairwise_collision_rule *NextInHash;
};

struct entity
{
	entity_type Type;
	u32 Flags;
	u32 ID; // For now this is just an index into the entity array.

	// Gameplay
	v3 P;
	v3 dP;
	v3 ddP;
	f32 Theta;
	f32 ThetaTarget;
	f32 dTheta;
	quaternion Orientation;
	movement_state MovementState;
	move_info MoveInfo;

	f32 Drag;
	f32 Acceleration;
	f32 AngularSpeed;
	attack Attack;

	// Collision0
	f32 Height;
	v3 AABBDim;
	v3 VolumeOffset;

	// Collision1
	obb OBB;

	// Collision2
	f32 Radius;

	// Collision3
	capsule Capsule;

	// Animation
	animation_player *AnimationPlayer;
	animation_graph *AnimationGraph;

	// Rendering
	// Should this be part of the game asset?
	v3 VisualScale;

	v3 LeftFootP;
	v3 RightFootP;
	v3 LeftHandP;
	v3 RightHandP;
};

struct camera
{
	v3 P;
	v3 Direction;
	f32 Yaw;
	f32 Pitch;
	quaternion RotationAboutY;
};

struct game_state
{
	memory_arena Arena;

	u32 PlayerEntityIndex;
	u32 EntityCount;
	entity Entities[4096];

	pairwise_collision_rule *CollisionRuleHash[256];
	pairwise_collision_rule *CollisionRuleFirstFree;

	quad Quad;

	b32 CameraIsFree;
	b32 CameraIsLocked;
	f32 CameraSpeed;
	f32 DefaultYaw;
	f32 DefaultPitch;

	camera Camera;
	v3 CameraddP;
	v3 CameraOffsetFromPlayer;

	mat4 CameraTransform;
	mat4 Perspective;

	f32 Angle;
	f32 TimeScale;
	f32 FOV;
	f32 Aspect;
	f32 ZNear;
	f32 ZFar;
	f32 Gravity;

	u32 CurrentCharacter;
	u32 PlayerIDForController[ArrayCount(((game_input *)0)->Controllers)];

	asset_manager AssetManager;

	texture Texture;
	model Cylinder;
	model *Capsule;
	model *Cube;
	model *Sphere;
};

struct temp_state
{
	b32 IsInitialized;
	memory_arena Arena;
};

global_variable platform_api Platform;
global_variable ui Ui;

#define GAME_H
#endif
