#if !defined(GAME_H)

// 11.24.2024 12:42 PM Animation player does not change states until switch to node is called.
// This allows to not switch nodes until certain conditions are true. For example, transitioning between
// running and sprinting states needs to happen at the proper times and until the animation time is within
// a valid interval the state remains the same.

#include <ft2build.h>
#include FT_FREETYPE_H

#include "platform.h"
#include "memory.h"

enum movement_state
{
	MovementState_Invalid,
	MovementState_Idle,
	MovementState_Run,
	MovementState_Sprint,
	MovementState_Jump,
	MovementState_TurningRight,
	MovementState_TurningLeft,
};

enum entity_type
{
	EntityType_Invalid,
	EntityType_Player,
	EntityType_Cube,
};

#include "intrinsics.h"
#include "math.h"
#include "strings.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"
#include "animation.h"
#include "asset.h"
#include "render.h"

struct entity
{
	entity_type Type;
	movement_state MovementState;

	// Gameplay
	v3 P;
	v3 dP;
	v3 ddP;
	quaternion Orientation;
	quaternion TargetOrientation;
	f32 Theta;
	f32 dTheta;

	// Animation
	animation_player *AnimationPlayer;
	animation_graph *AnimationGraph;
};

struct camera
{
	v3 P;
	v3 Direction;
};

struct game_state
{
	memory_arena Arena;

	u32 PlayerEntityIndex;
	u32 EntityCount;
	entity Entities[4096];

	quad Quad;

	camera Camera;
	v3 CameraOffsetFromPlayer;
	mat4 CameraTransform;

	f32 Angle;
	f32 TimeScale;
	f32 FOV;
	f32 Aspect;
	f32 ZNear;
	f32 ZFar;

	mat4 Perspective;

	u32 Shaders[4];

	u32 FBO;
	u32 RBO;
	u32 Texture;
	u32 TextureWidth;
	u32 TextureHeight;
	quad_2d Quad2d;

	asset_manager AssetManager;
};

struct temp_state
{
	b32 IsInitialized;
	memory_arena Arena;
};

global_varible platform_api Platform;

#define GAME_H
#endif
