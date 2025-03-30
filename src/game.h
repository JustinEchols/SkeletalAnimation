#if !defined(GAME_H)

/*
 * Animation
 *	- Remove duplicated joints in multiple meshes 
 *		[x] Skeleton now at model level and shared between all meshes of a model
 *	- Multiple controls turning animations
 *	- How to relate an attack state duration to an animations duration/time scale?
 *	- Fix stutter when root motion animation is blending out
 *
 * Physics
 *	- Moving Capsule vs Moving OBB
 *	- Moving Capsule vs Moving Capsule
 * Entity
 *	- Define/determine what entity flags last only during the frame and clear them as a group
 * UI
 *	- Generate IDs without having to refer to a function pointer
 *
// Weapons
//	- Weapons are often times part of a skeleton. The position of the collision 
//	  geometry can have its position at the weapon joint and then apply an 
//	  offset. The orientation of the weapon joint is also used as the orientation
//	  of the collision geometry
//
//	- Do we need to compute an obb instead of an abb for the mesh?
//  [] Need to introduce weapon concepts/data structures into mesh
//	[] data structure containing
//		- joint index
//			- will be the joint that the collision volume is relative to
//		- axis system or quaternion
//			- use the joint?
//	[x] aabb of mesh
//		- this is done in model space tpose
//			- should be able to construct a capsule from the aabb
*/

// TODO(Justin): Clean this up by processing the font data into an asset file
#if !RELEASE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "platform.h"
#include "memory.h"
#include "intrinsics.h"
#include "math.h"

enum movement_state
{
	MovementState_Idle,
	MovementState_Run,
	MovementState_Sprint,
	MovementState_Jump,
	MovementState_InAir,
	MovementState_Land,
	MovementState_Crouch,
	MovementState_Sliding,
	MovementState_Attack,
};

#include "strings.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"
#include "animation.h"
#include "ui.h"
#include "asset.h"
#include "render.h"
#include "collision.h"

struct move_info
{
	b32 AnyAction;
	b32 StandingStill;
	b32 CanSprint;
	b32 CanJump;
	b32 JumpPressed;
	b32 Crouching;
	b32 Attacking;
	b32 CanStrongAttack;
	b32 NoVelocity;
	b32 Accelerating;

	f32 Speed;

	v3 ddP;
	v2 StickDelta;
};

enum attack_type
{
	AttackType_None,
	AttackType_Neutral1,
	AttackType_Neutral2,
	AttackType_Neutral3,
	AttackType_Forward,
	AttackType_Strong,
	AttackType_Sprint,
	AttackType_Air,

	AttackType_Count
};

struct attack
{
	attack_type Type;
	f32 CurrentTime;
	f32 Duration;
	f32 Power;
};

enum entity_type
{
	EntityType_Null,
	EntityType_Player,
	EntityType_Cube,
	EntityType_Sphere,
	EntityType_WalkableRegion,
	EntityType_Elevator,
	EntityType_Light,
};

// NOTE(Justin) Determine a convention for when/where certain flags get updated. For example the animation system
// will spawn an attack collider at a certain time during an an attack animation and when that happens we know
// we need to check for an attack collision the animation system is responsible for the notification and then
// we turn on the attack collision check flag.
enum entity_flag
{
	EntityFlag_Collides = (1 << 1),
	EntityFlag_YSupported = (1 << 2),
	EntityFlag_Moveable = (1 << 3),
	EntityFlag_Collided = (1 << 4),
	EntityFlag_Moving = (1 << 5),
	EntityFlag_ShouldAttack = (1 << 6), // Anytime an attack key is pressed this is set
	EntityFlag_Attacking = (1 << 7),
	EntityFlag_Visible = (1 << 8),
	EntityFlag_ShouldJump = (1 << 9),
	EntityFlag_Jumping = (1 << 10),
	EntityFlag_AnimationControlling = (1 << 11),
	EntityFlag_AttackCollisionCheck = (1 << 12),
	EntityFlag_Attacked = (1 << 13),
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

	f32 Height;
	f32 DistanceFromGround;
	f32 Drag;
	f32 Acceleration;
	f32 AngularSpeed;
	f32 JumpDelay;

	collision_group MovementColliders;
	collision_group CombatColliders;

	v3 LeftFootP;
	v3 RightFootP;
	v3 LeftHandP;
	v3 RightHandP;

	attack_type AttackType;
	attack Attacks[AttackType_Count];

	// Animation
	animation_player *AnimationPlayer;
	animation_graph *AnimationGraph;

	// Rendering
	// Should this be part of the game asset?
	v3 VisualScale;
};

enum light_type
{
	LightType_None,
	LightType_Directional,

	LightType_Count,
};

struct light : public entity
{
	light_type Type;

	f32 Left;
	f32 Right;
	f32 Bottom;
	f32 Top;
	f32 Near;
	f32 Far;
	f32 Time;

	v3 Dir;
	mat4 Ortho;
	mat4 View;
};

struct camera
{
	b32 IsFree;
	b32 IsLocked;
	f32 Speed;
	f32 DefaultYaw;
	f32 DefaultPitch;
	f32 Yaw;
	f32 Pitch;

	v3 P;
	v3 Direction;
};

struct sound_id
{
	u32 Value;
};

struct sound
{
	v2 Volume;
	v2 VolumeTarget;
	v2 VolumeSpeed;
	f32 SamplesPlayed;
	f32 dSample;
	sound_id ID;
	sound *Next;
};

struct audio_state
{
	memory_arena *Arena;
	sound *Channels;
	sound *FreeChannels;
};

enum game_variable_type
{
	game_state_variable_type_none,
	game_state_variable_type_u32,
	game_state_variable_type_s32,
	game_state_variable_type_f32,
	game_state_variable_type_v3,
	game_state_variable_type_quaternion,
	game_state_variable_type_struct,
	game_state_variable_type_enum,
};

struct game_variable_data
{
	game_variable_type Type;
	b32 Initialize;
	memory_index Size;
	umm Offset;
	umm ArrayOffset;
	//u32 EnumValue;
};

struct game_state
{
	memory_arena Arena;

	u32 PlayerEntityIndex;
	u32 EntityCount;
	entity Entities[4096];
	light Lights[8];

	pairwise_collision_rule *CollisionRuleHash[256];
	pairwise_collision_rule *CollisionRuleFirstFree;

	quad Quad;

	camera Camera;
	v3 CameraddP;
	v3 CameraOffsetFromPlayer;

	mat4 CameraTransform;
	mat4 Perspective;

	f32 TimeScale;
	f32 FOV;
	f32 Aspect;
	f32 ZNear;
	f32 ZFar;
	f32 Gravity;

	u32 CurrentCharacter;
	u32 CharacterIDs[6];
	u32 PlayerIDForController[ArrayCount(((game_input *)0)->Controllers)];

	texture Texture;

	audio_state AudioState;
};

struct temp_state
{
	b32 IsInitialized;
	memory_arena Arena;
	asset_manager AssetManager;
};

global_variable platform_api Platform;
global_variable ui Ui;

inline void
UiAdvanceLine(f32 Amount = 0.0f)
{
	if(Amount != 0.0f)
	{
		Ui.P.y += Amount;
	}
	else
	{
		Ui.P.y -= Ui.LineGap;
	}
}

inline void
UiIndentAdd(f32 Amount = 0.0f)
{
	if(Amount != 0.0f)
	{
		Ui.P.x += Amount;
	}
	else
	{
		Ui.P.x += Ui.DefaultIndent;
	}
}

inline void
UiIndentRemove(f32 Amount = 0.0f)
{
	if(Amount != 0.0f)
	{
		Ui.P.x -= Amount;
	}
	else
	{
		Ui.P.x -= Ui.DefaultIndent;
	}
}

#define GAME_H
#endif
