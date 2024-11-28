#if !defined(PLATFORM_H)

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////
// NOTE(Justin): C Standard Library

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MemorySet(Source, Value, Size) memset(Source, Value, Size)
#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, Count * sizeof(Type))
#define PushStruct(Arena, Type) (Type *)PushSize_(Arena, sizeof(Type))
#define ArrayCopy(Count, Src, Dest) MemoryCopy((Count)*sizeof(*(Src)), (Src), (Dest))

//////////////////////////////////////////
// NOTE(Justin): Base Types

typedef int8_t	 s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t	 u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float	 f32;
typedef double	 f64;
typedef s32		 b32;
typedef size_t	 memory_index;

//////////////////////////////////////////
// NOTE(Justin): Helper Macros 

#define global_varible			static
#define internal				static
#define local_persist			static
#define Assert(Expression)		if(!(Expression)) {*(int *)0 = 0;}
#define ArrayCount(A)			sizeof(A) / sizeof((A)[0])
#define Kilobyte(Count)			(1024 * Count)
#define Megabyte(Count)			(1024 * Kilobyte(Count))
#define Gigabyte(Count)			(1024 * Megabyte(Count))
#define Pi32					3.1415926535897f
#define DegreeToRad(Degrees)	((Degrees) * (Pi32 / 180.0f))
#define SmallNumber				(1.e-8f)
#define KindaSmallNumber		(1.e-4f)

//////////////////////////////////////////
// NOTE(Justin): Linked List Macros

#define SLLQueuePush_N(First,Last,Node,Next) (((First)==0?\
(First)=(Last)=(Node):\
((Last)->Next=(Node),(Last)=(Node))),\
(Node)->Next=0)
#define SLLQueuePush(First,Last,Node) SLLQueuePush_N(First,Last,Node,Next)

#if DEVELOPER
struct debug_file
{
	void *Content;
	u64 Size;
};

#define DEBUG_PLATFORM_FILE_READ_ENTIRE(FunctionName) debug_file FunctionName(char *FileName)
typedef DEBUG_PLATFORM_FILE_READ_ENTIRE(debug_platform_file_read_entire);

#define DEBUG_PLATFORM_FILE_WRITE_ENTIRE(FunctionName) b32 FunctionName(char *FileName, void *Memory, u32 MemorySize)
typedef DEBUG_PLATFORM_FILE_WRITE_ENTIRE(debug_platform_file_write_entire);

#define DEBUG_PLATFORM_FILE_FREE(FunctionName) void FunctionName(void *Memory)
typedef DEBUG_PLATFORM_FILE_FREE(debug_platform_file_free);

#define DEBUG_PLATFORM_FILE_HAS_UPDATED(FunctionName) b32 FunctionName(char *FileName)
typedef DEBUG_PLATFORM_FILE_HAS_UPDATED(debug_platform_file_has_updated);
#endif

typedef void platform_render_to_opengl(struct render_buffer *RenderBuffer, u32 WindowWidth, u32 WindowHeight);

typedef struct 
{
	platform_render_to_opengl *RenderToOpenGL;
#if DEVELOPER
	debug_platform_file_read_entire		*DebugFileReadEntire;
	debug_platform_file_write_entire	*DebugFileWriteEntire;
	debug_platform_file_free			*DebugFileFree;
#endif
} platform_api;

inline u32
U64TruncateToU32(u64 U64)
{
	Assert(U64 <= 0xFFFFFFFF);
	u32 Result = (u32)U64;
	return(Result);
}

struct game_button
{
	b32 EndedDown;
	u32 HalfTransitionCount;
};

inline b32
IsDown(game_button Button)
{
	b32 Result = (Button.EndedDown);
	return(Result);
}

inline b32
WasPressed(game_button Button)
{
	b32 Result = (Button.HalfTransitionCount > 1) ||
				((Button.HalfTransitionCount == 1) && Button.EndedDown);
	return(Result);
}

enum 
{
	Key_W,
	Key_A,
	Key_S,
	Key_D,
	Key_E,
	Key_Shift,
	Key_Space,
	Key_Add,
	Key_Subtract,
	Key_Control,

	Key_Count
};

enum mouse_button
{
	MouseButton_Left,
	MouseButton_Middle,
	MouseButton_Right,

	MouseButton_Count
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
			game_button E;
			game_button Shift;
			game_button Space;
			game_button Add;
			game_button Subtract;
			game_button Ctrl;
		};
	};
};

struct game_input
{
	game_keyboard Keyboard;
	f32 DtForFrame;
	f32 MouseX, MouseY;
	f32 dXMouse, dYMouse;
	game_button MouseButtons[5];
	s32 BackBufferWidth, BackBufferHeight;
};

struct game_memory
{
	b32 IsInitialized;

	u64 PermanentStorageSize;
	void *PermanentStorage;

	u64 TemporaryStorageSize;
	void *TemporaryStorage;

	platform_api PlatformAPI;
};

#define GAME_UPDATE_AND_RENDER(FunctionName) void FunctionName(game_memory *GameMemory, game_input *GameInput)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#ifdef __cplusplus
}
#endif

#define PLATFORM_H
#endif
