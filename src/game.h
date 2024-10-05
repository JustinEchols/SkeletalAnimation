#if !defined(GAME_H)

struct memory_arena
{
	u8 *Base;
	memory_index Size;
	memory_index Used;

	s32 TempCount;
};

struct temporary_memory
{
	memory_arena *Arena;
	memory_index Used;
};

internal void
ArenaInitialize(memory_arena *Arena, u8 *Base, memory_index Size)
{
	Arena->Base = Base;
	Arena->Size = Size;
	Arena->Used = 0;
	Arena->TempCount = 0;
}

internal void *
PushSize_(memory_arena *Arena, memory_index Size)
{
	Assert((Arena->Used + Size) <= Arena->Size);

	void *Result = Arena->Base + Arena->Used;
	Arena->Used += Size;

	return(Result);
}

internal void *
MemoryCopy(memory_index Size, void *SrcInit, void *DestInit)
{
	u8 *Src = (u8 *)SrcInit;
	u8 *Dest = (u8 *)DestInit;
	while(Size--) {*Dest++ = *Src++;}

	return(DestInit);
}

internal void
MemoryZero(memory_index Size, void *Src)
{
	u8 *P = (u8 *)Src;
	while(Size--)
	{
		*P++ = 0;
	}
}

inline temporary_memory
TemporaryMemoryBegin(memory_arena *Arena)
{
	temporary_memory Result;

	Result.Arena = Arena;
	Result.Used = Arena->Used;

	Arena->TempCount++;

	return(Result);
}

inline void
TemporaryMemoryEnd(temporary_memory TempMemory)
{
	memory_arena *Arena = TempMemory.Arena;
	Assert(Arena->Used >= TempMemory.Used);
	Arena->Used = TempMemory.Used;
	Assert(Arena->TempCount > 0);
	Arena->TempCount--;
}


internal u32
U32ArraySum(u32 *A, u32 Count)
{
	u32 Result = 0;
	for(u32 Index = 0; Index < Count; ++Index)
	{
		Result += A[Index];
	}
	return(Result);
}

#include "intrinsics.h"
#include "math_util.h"
#include "strings.h"
#include "mesh.h"
#include "animation.h"
#include "asset.h"

enum entity_type
{
	EntityType_Invalid,
	EntityType_Player,
	EntityType_Cube,
};

struct entity
{
	entity_type Type;

	v3 P;
	v3 dP;
	v3 ddP;
	quaternion Orientation;
};

struct game_state
{
	memory_arena Arena;
	memory_arena TempArena;

	u32 EntityCount;
	entity Entities[4096];

	u32 PlayerEntityIndex;
	model *XBot;
	model *Cube;

	animation_player AnimationPlayer;
	animation_info *AnimationInfos;
	animation *Animations;

	v3 CameraP;
	v3 Direction;
	mat4 CameraTransform;

	f32 FOV;
	f32 Aspect;
	f32 ZNear;
	f32 ZFar;
	mat4 PerspectiveTransform;

	f32 Angle;

	u32 Shaders[2];
};

#define GAME_H
#endif
