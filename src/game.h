#if !defined(GAME_H)

#include "platform.h"

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

internal void
ArenaClear(memory_arena *Arena)
{
	ArenaInitialize(Arena, Arena->Base, Arena->Size);
}

internal void
ArenaSubset(memory_arena *Parent, memory_arena *Child, memory_index Size)
{
	ArenaInitialize(Child, Parent->Base + Parent->Used, Size);
	Parent->Used += Size;
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
MemoryZero(void * Src, memory_index Size)
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



#if 0
internal void
PrintString(char *String)
{
	OutputDebugStringA(String);
	OutputDebugStringA("\n");
}
#endif

enum movement_state
{
	MovementState_Invalid,
	MovementState_Idle,
	MovementState_Run,
	MovementState_Sprint,
	MovementState_Jump,
	MovementState_TurningRight,
	MovementState_TurningLeft,
};

enum entity_type
{
	EntityType_Invalid,
	EntityType_Player,
	EntityType_Cube,
};

#include "intrinsics.h"
#include "math.h"
#include "strings.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"
#include "animation.h"
#include "asset.h"

// TODO(Justin): quad_3d_vertex?
struct quad_vertex
{
	v3 P;
	v3 N;
	v2 UV;
};

// TODO(Justin): quad_3d?
struct quad
{
	b32 UploadedToGPU;
	u32 VA;
	u32 VB;
	u32 Texture;

	quad_vertex Vertices[6];
};

struct quad_2d
{
	u32 VA;
	u32 VB;
	u32 Texture;
};

#include "render.h"



struct entity
{
	entity_type Type;
	movement_state MovementState;

	// Gameplay
	v3 P;
	v3 dP;
	v3 ddP;
	quaternion Orientation;
	quaternion TargetOrientation;
	f32 Theta;
	f32 dTheta;

	// Animation
	animation_player *AnimationPlayer;
	animation_graph *AnimationGraph;
};






struct camera
{
	v3 P;
	v3 Direction;
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

	f32 Angle;
	f32 TimeScale;
	f32 FOV;
	f32 Aspect;
	f32 ZNear;
	f32 ZFar;

	mat4 Perspective;

	u32 Shaders[4];

	u32 FBO;
	u32 RBO;
	u32 Texture;
	u32 TextureWidth;
	u32 TextureHeight;
	quad_2d Quad2d;

	asset_manager AssetManager;
};

struct temp_state
{
	b32 IsInitialized;
	memory_arena Arena;
};

#define GAME_H
#endif
