#if !defined(ANIMATION_H)

// NOTE(Justin): There are JointCount number of positions, quaternions, and scales
// where JointCount is the # of joints that the animation affects _not_ the number of joints
// in the skeleton.

struct key_frame
{
	v3 *Positions;
	quaternion *Orientations;
	v3 *Scales;
};

// NOTE(Justin): animation_info is a sampled/keyframed animation.
// JointNames an array of all the names of the joints that this sampled animation affects.
// KeyFrames an array of the actual samples of the animation.
// ReservedForChannel is memory reserved for the interpolated pose (output) of a channel. This is allocated at animation load time.

struct animation_info
{
	string Name;

	u32 JointCount;
	u32 KeyFrameCount;
	f32 CurrentTime;
	f32 Duration;
	f32 FrameRate;

	string *JointNames;
	key_frame *KeyFrames;
	key_frame *ReservedForChannel;
};

struct animation_id
{
	u32 Value;
};

enum animation_flag
{
	AnimationFlag_Playing = (1 << 1),
	AnimationFlag_Finished = (1 << 2), // Notifies that a non-looping animation has completed.
	AnimationFlag_Looping = (1 << 3),
	AnimationFlag_RemoveLocomotion = (1 << 4),
	AnimationFlag_JointMask = (1 << 5),
	AnimationFlag_ControlsPosition = (1 << 6),
	AnimationFlag_ControlsTurning = (1 << 7),
	AnimationFlag_CompletedCycle = (1 << 8),
	AnimationFlag_IgnoreYMotion = (1 << 9),
};

// NOTE(Justin): Animation channel/sampler
// InterpolatedPose is the output of a channel computed by interpolating between two key frames.
// It is used in the n-way blend calculation of the final pose

struct animation
{
	string Name;

	u32 Flags;
	f32 Duration;
	f32 CurrentTime; 
	f32 TimeScale; 
	f32 BlendFactor;
	f32 BlendDuration;
	f32 BlendCurrentTime;
	b32 BlendingIn;
	b32 BlendingOut;

	animation_id ID;
	animation_info *Info;
	animation *Next;

	key_frame *InterpolatedPose;

	v3 RootMotionDeltaPerFrame;
	v3 RootVelocityDeltaPerFrame;
};

// NOTE(Justin): animation_player 
// FinalPose is the output of the animation player used in the calculation of a model's current pose
// The input is an animation and each animation playing (>= 1) is responsible for computing an interpolated pose
// which is then used to compute FinalPose as a weighted blend.

struct animation_player
{
	b32 IsInitialized;
	movement_state MovementState;
	movement_state NewState;

	memory_arena *Arena;
	animation *Channels;
	animation *FreeChannels;

	f32 CurrentTime;
	f32 dt;

	b32 SpawnAttackCollider;
	b32 ControlsPosition;
	b32 ControlsTurning;
	b32 UpdateLockedP;
	u32 PlayingCount;
	u32 RetiredCount;

	key_frame *FinalPose;
	model *Model; 

	v3 EntityPLockedAt;
	v3 RootMotionAccumulator;
	v3 RootVelocityAccumulator;
};

enum arc_type 
{
	ArcType_None,
	ArcType_TimeInterval,
	ArcType_WhenDone,
};

struct animation_graph_arc
{
	string Message;
	string Destination;
	f32 RemainingTimeBeforeCrossFade;
	arc_type Type;
	f32 t0, t1;
	b32 BlendDurationSet;
	f32 BlendDuration;
	f32 StartTime;
};

// TODO(Justin): Node type? Additive, nblend, composite...
// TODO(Justin): Spawn/Do mesh collider for attack? Spawnt0 Spawnt1. Performed once per frame.
// TODO(Justin): Right now Tag is the actual name of the animation...
struct animation_graph_node
{
	string Name;
	string Tag; 
	u32 AnimationFlags;
	f32 TimeScale;
	f32 Collidert0;
	f32 Collidert1;
	u32 Index;
	u32 ArcCount;
	animation_graph_arc Arcs[16];
	animation_graph_arc WhenDone;
};

struct animation_graph
{
	memory_arena Arena;
	u32 NodeCount;
	u32 Index;
	animation_graph_node CurrentNode;
	animation_graph_node Nodes[64];

	// Members needed for hot reloading
	string Path;
	string Name;
};

#define ANIMATION_H
#endif
