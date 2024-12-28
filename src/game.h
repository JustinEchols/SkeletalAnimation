#if !defined(GAME_H)

// NOTE(Justin): JUMP ANIMATION. 
// The issue at hand is that when the jump is finished
// two animations start bledning together. The jump animation
// is near the end and the position is far away while the other animation
// blending in is near the model space origin. The result is that
// the position interpolates towards the model origin resulting in
// the player sliding back world space during the blend.
//
// SOlUTION. If an animation controls the position of the player, it only
// controls it while it is NOT BLENDING OUT. During playback the previous
// updates are the same. However when the animation starts blending out
// we keep checking to see if the current aniomation is blending in.
// While the current animation is blending in we set the root position
// of the FinalPose to be the root position of the currently blending
// in animation. After the current it done blending in we set the 
// total accumulated delta computed during the controls position animation
// to 0. The next iteration we skip setting the root position and continue
// BAU.

// NOTE(Justin): COLLISION. 
// The vector from the center of the OBB to the center of the sphere, that is computed in world space,
// is RelP and is the position vector of the center of the sphere in
// relative space/OBB space/configuration space/minkowski space.
//
// We compute the coordinates of RelP and DeltaP in this space and are then able to do an AABB test with
// the center of the AABB at the origin, 0. Then, each face of the AABB is a plane
// and we can determine each normal and signed distance D of the plane. The normal is just pre-populated
// data and the signed distance of the plane is computed by taking the dot product with the normal
// and either the min or max of the AABB depending on which normal is currently being tested. The
// reason why the dot product is done with the min or max is due to the fact that these are actual
// points on the respective planes. I.e. D = n.X, where X is either AABB.Min or AABB.max.

// The radius is the HALF DIM of the bounding box constructed from the
// sphere. So, the correct MK sum needs to use TWICE the radius, per the implementation of
// AABBCenterDim(-).


// NOTE(Justin): COLLISION. 
// NOTE(Justin): If the ground normal is way to big compared to the rest of the units
// in the game then the tGround value will be 0 a majority of the time. Which
// is a bug. 

// NOTE(Justin): ANIMATION. Calling animation play everytime is what allows
// the animation system to "work" currently. If we only allow an animation to play during a 
// state change then if a sudden state change happens such as idle -> run -> idle
// what ends up happening is that the idle and run animation do not complete the cross fade.
// Since the cross fade is not complete both animations are still active. Since both are still active
// the idle animation is active. if the idle animation is still active and we try and play another idle 
// animation then it will return immedialtey. The result is that the original blend between idle and run
// will complete the cross fade. When this happens the idle animation drops and we are left with a running animation
// that keeps looping even though from the game perspective the player is not moving.
//
// Q: How do we fix this without having to call animation play everytime?
// Or is calling animation play everytime an ok solution?
//
// Q: Do we force the blend to complete before moving to another animation?
// If we play animation that is currently blending with another then we already force the blend to complete before
// playing the animation 

//
// There seem to be a lot of problems in different places in the animation system that
// arise from the case when two animations are blending and something else happens..
// What is the best way to handle this?
//

// TODO(Justin): ANIMTATION - Finish updating this comment.
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

//////////////////////////////////////////

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
	MovementState_Falling,
};

enum entity_type
{
	EntityType_Invalid,
	EntityType_Player,
	EntityType_Cube,
	EntityType_Sphere,
	EntityType_WalkableRegion,
};

enum entity_flag
{
	EntityFlag_YSupported = (1 << 2),
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
	u32 Flags;

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
	b32 CameraIsFree;

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

#define GAME_H
#endif
