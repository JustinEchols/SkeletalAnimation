#if !defined(GAME_H)

// TODO(Justin): Finish updating this comment.
// 12.10.2024 7:06 PM - Animation Driven Movement
// Added a bool that tells whether or not the animation player is driving the in game position.
// So a node has a controls position flag and the animation player has a controls position flag.
// The reason being is when the animation that controls position is almost done, the animation starts
// blending out with another animation AND HAS ALREADY SWITCHED TO A NEW NODE. We need information
// on whether or not there is a previously playing animation that controlled the in game position of the player.
// This bool tells us that. When updating each channel we look to see whether or not that channel controls
// the position of the player. If it does then the animation player is considered to be controlling player
// to.
//
// The animation delta accumulated overtime is the delta betwwen the current and next root position OF THE FINAL
// MIXED POSE.

// 12.10.2024 7:06 PM - Animation Driven Movement (Old Comments)
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


// 11.24.2024 12:42 PM - Animation Player
// Animation player does not change states until switch to node is called.
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
	EntityType_WalkableRegion,
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

	// Collision0
	f32 Height;
	v3 AABBDim;
	v3 VolumeOffset;

	// Collision1
	capsule Capsule;

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
	model *Circle;
	model *Capsule;
	model *Cube;
};

struct temp_state
{
	b32 IsInitialized;
	memory_arena Arena;
};

global_variable platform_api Platform;

#define GAME_H
#endif
