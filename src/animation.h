#if !defined(ANIMATION_H)

#if 0
struct animation_names
{
	string Name;
	string FullPath;
	b32 Loaded;
	b32 Dirty;
	table Table;
};
#endif

enum animation_name
{
	Animation_IdleRight,
	Animation_IdleLeft,
	Animation_Run,
	Animation_Sprint,
	Animation_JumpForward,
	Animation_RunMirror,
	Animation_SprintMirror,
};

char *AnimationFiles[] =
{
	"..\\data\\XBot_IdleRight.animation", // can transition to running and sprint
	"..\\data\\XBot_IdleLeft.animation", // can transition to mirrored running and sprint
	"..\\data\\XBot_Running.animation",
	"..\\data\\XBot_FastRun.animation",
	"..\\data\\XBot_JumpForward.animation",
	"..\\data\\XBot_RunningMirror.animation",
	"..\\data\\XBot_FastRunMirror.animation",
};

enum animation_state
{
	AnimationState_Invalid = 0x0,
	AnimationState_Idle = 0x1,
	AnimationState_Running = 0x3,
	AnimationState_Sprint = 0x4,
	AnimationState_JumpForward = 0x5,
};

struct key_frame
{
	v3 *Positions;
	quaternion *Orientations;
	v3 *Scales;
};

struct animation_info
{
	u32 JointCount;
	u32 KeyFrameCount;

	f32 CurrentTime;
	f32 Duration;
	f32 FrameRate;

	string *JointNames;
	key_frame *KeyFrames;
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
	AnimationFlags_MustFinish = (1 << 5),
};

struct animation
{
	char *Name;

	u32 DefaultFlags;
	u32 Flags;

	f32 Duration;
	f32 CurrentTime; 
	f32 OldTime; 
	f32 TimeScale; 

	f32 BlendFactor;
	f32 BlendDuration;
	f32 BlendCurrentTime;
	b32 BlendingIn;
	b32 BlendingOut;
	b32 BlendingComposite;

	animation_id ID;
	animation_info *Info;
	animation *Next;

	key_frame *BlendedPose;
};

enum arc_type 
{
	ArcType_None,
	ArcType_TimeInterval,
};

struct animation_graph_arc
{
	string Destination;
	string Message;
	f32 RemainingTimeBeforeCrossFade;
	arc_type Type;
	f32 t0, t1;
};

struct animation_graph_node
{
	string Name;
	string Tag;
	u32 Index;
	u32 ArcCount;
	animation_graph_arc Arcs[16];
	animation_graph_arc WhenDone;
};

struct animation_graph
{
	memory_arena *Arena;
	u32 NodeCount;
	u32 Index;
	animation_graph_node CurrentNode;
	animation_graph_node Nodes[64];
};

struct animation_player
{
	b32 IsInitialized;
	animation_state State;
	movement_state MovementState;

	memory_arena *Arena;
	animation *Channels;
	animation *FreeChannels;

	u32 PlayingCount;
	u32 RetiredCount;

	f32 CurrentTime;
	f32 dt;

	key_frame *FinalPose;
	model *Model; 
};


#define ANIMATION_H
#endif
