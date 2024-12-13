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
	MovementState_Crouch,
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
#include "ui.h"
#include "asset.h"
#include "render.h"

struct entity
{
	entity_type Type;

	// Gameplay
	v3 P;
	v3 dP;
	v3 ddP;
	f32 Theta;
	f32 ThetaTarget;
	f32 dTheta;
	quaternion Orientation;
	movement_state MovementState;

	// Collision
	f32 Height;
	v3 AABBDim;
	v3 VolumeOffset;

	// Animation
	animation_player *AnimationPlayer;
	animation_graph *AnimationGraph;

	// Rendering
	// Should this be part of the game asset?
	v3 VisualScale;
};

struct camera
{
	v3 P;
	v3 Direction;
	quaternion RotationAboutY;
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
	mat4 Perspective;

	f32 Angle;
	f32 TimeScale;
	f32 FOV;
	f32 Aspect;
	f32 ZNear;
	f32 ZFar;
	f32 Gravity;

	asset_manager AssetManager;

	ui UI;

	texture Texture;
	model Cylinder;
};

struct temp_state
{
	b32 IsInitialized;
	memory_arena Arena;
};

global_variable platform_api Platform;

#define GAME_H
#endif
