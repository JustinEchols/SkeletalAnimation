#if !defined(ANIMATION_H)

struct key_frame
{
	v3 *Positions;
	quaternion *Orientations;
	v3 *Scales;
};

// NOTE(Justin): Sampled animation
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

enum animation_flags
{
	AnimationFlags_Playing = (1 << 1),
	AnimationFlags_Finished = (1 << 2),
	AnimationFlags_Looping = (1 << 3),
	AnimationFlags_RemoveLocomotion = (1 << 4),
	AnimationFlags_JointMask = (1 << 5),
	AnimationFlags_ControlsPosition = (1 << 6),
	AnimationFlags_ControlsTurning = (1 << 7),
};

// NOTE(Justin): Animation channel 
struct animation
{
	string Name;

	u32 Flags;
	f32 Duration;
	f32 CurrentTime; 
	f32 OldTime; 
	f32 TimeScale; 
	f32 StartTime; // Does this need to be stored?

	f32 BlendFactor;
	f32 BlendDuration;
	f32 BlendCurrentTime;
	b32 BlendingIn;
	b32 BlendingOut;

	animation_id ID;
	animation_info *Info;
	animation *Next;

	key_frame *BlendedPose;

	v3 MotionDeltaPerFrame;
	f32 TurningDeltaPerFrame;
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
// TODO(Justin): Right now Tag is the actual name of the animation...
struct animation_graph_node
{
	string Name;
	string Tag; 
	u32 AnimationFlags;
	f32 TimeScale;
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
};

struct animation_player
{
	b32 IsInitialized;
	movement_state MovementState;
	movement_state NewState;

	memory_arena *Arena;
	animation *Channels;
	animation *FreeChannels;

	b32 ControlsPosition;
	b32 ControlsTurning;
	u32 PlayingCount;
	u32 RetiredCount;

	f32 CurrentTime;
	f32 dt;

	v3 RootMotionAccumulator;
	v3 RootPLocked;

	f32 RootTurningAccumulator;
	quaternion OrientationLockedAt;

	key_frame *FinalPose;
	model *Model; 
};

#define ANIMATION_H
#endif
