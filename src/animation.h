#if !defined(ANIMATION_H)

enum animation_name
{
	Animation_Idle,
	Animation_Walk,
	Animation_Run,
	Animation_Sprint,
	Animation_Jump,
	Animation_Wave,
	Animation_IdleToSprint,
	Animation_JumpForward,
};

char *AnimationFiles[] =
{
	"..\\data\\XBot_ActionIdle.animation",
	"..\\data\\XBot_Walking.animation",
	"..\\data\\XBot_Running.animation",
	"..\\data\\XBot_FastRun.animation",
	"..\\data\\XBot_Jumping.animation",
	"..\\data\\XBot_Waving.animation",
	"..\\data\\XBot_IdleToSprint.animation",
	"..\\data\\XBot_JumpForward.animation",
	"..\\data\\XBot_IdleLookAround.animation",
	"..\\data\\XBot_FemaleWalk.animation",
	"..\\data\\XBot_RightTurn.animation",
	"..\\data\\XBot_LeftTurn.animation",
	"..\\data\\XBot_PushingStart.animation",
	"..\\data\\XBot_Pushing.animation",
	"..\\data\\XBot_PushingStop.animation",
	"..\\data\\XBot_ActionIdleToStandingIdle.animation",
	"..\\data\\XBot_RunningToTurn.animation",
	"..\\data\\XBot_RunningChangeDirection.animation",
	"..\\data\\XBot_RunToStop.animation",
};

enum animation_state
{
	AnimationState_Invalid = 0x0,
	AnimationState_Idle = 0x1,
	AnimationState_IdleToRun = 0x2,
	AnimationState_IdleToSprint = 0x3,
	AnimationState_Running = 0x4,
	AnimationState_RunningToIdle = 0x5,
	AnimationState_RunningToSprint = 0x6,
	AnimationState_Sprint = 0x7,
	AnimationState_SprintToIdle = 0x8,
	AnimationState_SprintToRun = 0x9,
	AnimationState_JumpForward = 0x10,
	AnimationState_IdleWalkRun = 0x11,

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

struct animation_player
{
	b32 IsInitialized;
	animation_state State;

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
