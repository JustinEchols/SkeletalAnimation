#if !defined(ANIMATION_H)

char *AnimationFiles[] =
{
	"..\\data\\XBot_ActionIdle.animation",
	"..\\data\\XBot_Walking.animation",
	"..\\data\\XBot_Running.animation",
	"..\\data\\XBot_FastRun.animation",
	"..\\data\\XBot_Jumping.animation",
	"..\\data\\XBot_IdleToSprint.animation",
	"..\\data\\XBot_FemaleWalk.animation",
	"..\\data\\XBot_IdleLookAround.animation",
	"..\\data\\XBot_RightTurn.animation",
	"..\\data\\XBot_LeftTurn.animation",
	"..\\data\\XBot_PushingStart.animation",
	"..\\data\\XBot_Pushing.animation",
	"..\\data\\XBot_PushingStop.animation",
	"..\\data\\XBot_ActionIdleToStandingIdle.animation",
	"..\\data\\XBot_RunningToTurn.animation",
	"..\\data\\XBot_RunningChangeDirection.animation",

};

enum animation_state
{
	AnimationState_Idle = 0x0,
	AnimationState_Running = 0x1,
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
	AnimationFlags_Looping = (1 << 1),
	AnimationFlags_RemoveLocomotion = (1 << 2),
	AnimationFlags_Finished = (1 << 3),
};

struct animation
{
	u32 Flags;

	f32 Duration;
	f32 CurrentTime; 
	f32 OldTime; 
	f32 TimeScale; 

	f32 BlendFactorLast;
	f32 BlendDuration;
	f32 BlendCurrentTime;
	b32 BlendingIn;
	b32 BlendingOut;

	animation_id ID;
	animation_info *Info;
	animation *Next;
};

struct animation_player
{
	b32 IsInitialized;
	animation_state State;

	memory_arena *PermArena;
	animation *AnimationPreviouslyAdded;
	animation *AnimationPreviouslyFreed;

	u32 PlayingCount;
	u32 RetiredCount;

	f32 CurrentTime;
	f32 dt;

	key_frame *BlendedAnimations;
	model *Model; 
};

#define ANIMATION_H
#endif
