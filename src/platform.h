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

#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, Count * sizeof(Type))
#define PushStruct(Arena, Type) (Type *)PushSize_(Arena, sizeof(Type))
#define ArrayCopy(Count, Src, Dest) MemoryCopy((Count)*sizeof(*(Src)), (Src), (Dest))

#define SLLQueuePush_N(First,Last,Node,Next) (((First)==0?\
(First)=(Last)=(Node):\
((Last)->Next=(Node),(Last)=(Node))),\
(Node)->Next=0)
#define SLLQueuePush(First,Last,Node) SLLQueuePush_N(First,Last,Node,Next)

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

struct game_button
{
	b32 IsDown;
	u32 HalfTransitionCount;
};

enum 
{
	Key_W,
	Key_A,
	Key_S,
	Key_D,

	Key_Count
};

struct game_keyboard
{
	union
	{
		game_button Buttons[Key_Count];
		struct
		{
			game_button W;
			game_button A;
			game_button S;
			game_button D;
		};
	};
};

struct game_input
{
	game_keyboard Keyboard;
	f32 DtForFrame;
	//v3 MouseP;
};

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
