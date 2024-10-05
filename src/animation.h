#if !defined(ANIMATION_H)

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

struct animation
{
	b32 Looping;
	b32 Finished;
	b32 RemoveLocomotion;

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

	memory_arena *PermArena;
	animation *AnimationPreviouslyAdded;
	animation *AnimationPreviouslyFreed;

	u32 ActiveCount;
	u32 RetiredCount;

	f32 CurrentTime;
	f32 dt;

	key_frame *BlendedAnimations;
	model *Model; 
};

#define ANIMATION_H
#endif
