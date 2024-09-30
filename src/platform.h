#if !defined(PLATFORM_H)

#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef s32 b32;
typedef size_t memory_index;

#define global_varible static
#define internal static
#define local_persist static

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define ArrayCount(A) sizeof(A) / sizeof((A)[0])
#define Kilobyte(Count) (1024 * Count)
#define Megabyte(Count) (1024 * Kilobyte(Count))
#define Gigabyte(Count) (1024 * Megabyte(Count))

#define PI32 3.1415926535897f
#define SMALL_NUMBER (1.e-8f)
#define DegreeToRad(Degrees) ((Degrees) * (PI32 / 180.0f))


#define COLLADA_ATTRIBUTE_MAX_COUNT 10
#define COLLADA_NODE_CHILDREN_MAX_COUNT 75

#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, Count * sizeof(Type))
#define PushStruct(Arena, Type) (Type *)PushSize_(Arena, sizeof(Type))
#define ArrayCopy(Count, Src, Dest) MemoryCopy((Count)*sizeof(*(Src)), (Src), (Dest))

#define SLLQueuePush_N(First,Last,Node,Next) (((First)==0?\
(First)=(Last)=(Node):\
((Last)->Next=(Node),(Last)=(Node))),\
(Node)->Next=0)
#define SLLQueuePush(First,Last,Node) SLLQueuePush_N(First,Last,Node,Next)

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

struct debug_file
{
	void *Content;
	u32 Size;
};

inline u32
U64TruncateToU32(u64 U64)
{
	Assert(U64 <= 0xFFFFFFFF);
	u32 Result = (u32)U64;
	return(Result);
}

struct game_memory
{
	b32 IsInitialized;

	u64 PermanentStorageSize;
	void *PermanentStorage;


	u64 TemporaryStorageSize;
	void *TemporaryStorage;

};

#define PLATFORM_H
#endif
