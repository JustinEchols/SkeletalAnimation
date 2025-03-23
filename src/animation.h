#if !defined(ANIMATION_H)

// NOTE(Justin): There are JointCount number of positions, quaternions, and scales
// where JointCount is the # of joints that the animation affects.
struct key_frame
{
	v3 *Positions;
	quaternion *Orientations;
	v3 *Scales;
};

// NOTE(Justin): Sampled animation
// TODO(Justin): Remove current time 
struct animation_info
{
	string Name;

	u32 JointCount;
	u32 KeyFrameCount;
	f32 CurrentTime;
	f32 Duration;
	f32 FrameRate;

	string *JointNames; // Names of all the joints that this sampled animation affects
	key_frame *KeyFrames; // Actual samples of the animation
	key_frame *ReservedForChannel; // Memory reserved for blended pose (output) of animation channel 
};

struct animation_id
{
	u32 Value;
};

enum animation_flags
{
	AnimationFlags_Playing = (1 << 1),
	AnimationFlags_Finished = (1 << 2), // Non-looping animation finished
	AnimationFlags_Looping = (1 << 3),
	AnimationFlags_RemoveLocomotion = (1 << 4),
	AnimationFlags_JointMask = (1 << 5),
	AnimationFlags_ControlsPosition = (1 << 6),
	AnimationFlags_ControlsTurning = (1 << 7),
	AnimationFlags_CompletedCycle = (1 << 8), // Notifies animation player that animation controlling movement has completed a single playback
	AnimationFlags_IgnoreYMotion = (1 << 9),
};

// NOTE(Justin): Animation channel 
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

	key_frame *BlendedPose;

	v3 RootMotionDeltaPerFrame;
	v3 RootVelocityDeltaPerFrame;
};

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
	string Path;
	string Name;
	memory_arena Arena;
	u32 NodeCount;
	u32 Index;
	animation_graph_node CurrentNode;
	animation_graph_node Nodes[64];
};

#define ANIMATION_H
#endif
