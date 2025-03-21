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

typedef int8_t		s8;
typedef int16_t  	s16;
typedef int32_t  	s32;
typedef int64_t  	s64;
typedef uint8_t	 	u8;
typedef uint16_t 	u16;
typedef uint32_t 	u32;
typedef uint64_t 	u64;
typedef float	 	f32;
typedef double	 	f64;
typedef s32			b32;
typedef size_t		memory_index;
typedef uintptr_t	umm;

//////////////////////////////////////////
// NOTE(Justin): Helper Macros 

#define internal				static
#define local_persist			static
#define global_variable			static
#define Assert(Expression)		if(!(Expression)) {*(int *)0 = 0;}
#define InvalidDefaultCase		default: {Assert(0);}
#define ArrayCount(A)			(sizeof(A) / sizeof((A)[0]))
#define Kilobyte(Count)			(1024 * Count)
#define Megabyte(Count)			(1024 * Kilobyte(Count))
#define Gigabyte(Count)			(1024 * Megabyte(Count))
#define Pi32					3.1415926535897f
#define DegreeToRad(Degrees)	((Degrees) * (Pi32 / 180.0f))
#define RadToDegrees(Rad)		((Rad) * (180.0f / Pi32))
#define SmallNumber				(1.e-8f)
#define KindaSmallNumber		(1.e-4f)
#define F32Max					3.40282e+38
#define OffsetOf(type, Member)  (umm)&(((type *)0)->Member)
#define CString(String)			(char *)String.Data

#define FILE_AND_LINE__(A, B) A "|" #B
#define FILE_AND_LINE_(A, B) FILE_AND_LINE__(A, B)
#define FILE_AND_LINE FILE_AND_LINE_(__FILE__, __LINE__)

struct platform_file_info
{
	u64 FileDate;
	u64 FileSize;
	char *Path;
	void *Platform;
};

struct file_group_info
{
	u32 Count;
	char FileNames[64][256];
};

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

// TODO(Justin): Split this into a directory name and wildcard
#define DEBUG_PLATFORM_FILE_GROUP_LOAD(FunctionName) file_group_info FunctionName(char *DirectoryNameAndWildCard)
typedef DEBUG_PLATFORM_FILE_GROUP_LOAD(debug_platform_file_group_load);

#define DEBUG_PLATFORM_FILE_IS_DIRTY(FunctionName) b32 FunctionName(char *Path, u64 *Date)
typedef DEBUG_PLATFORM_FILE_IS_DIRTY(debug_platform_file_is_dirty);
#endif

// TODO(Justin): Consolidate these calls
typedef void platform_render_to_opengl(struct render_buffer *RenderBuffer, u32 WindowWidth, u32 WindowHeight);
typedef void platform_upload_animated_model_to_gpu(struct model *Model);
typedef void platform_upload_model_to_gpu(struct model *Model);
typedef void platform_upload_texture_to_gpu(struct texture *Texture);

typedef struct 
{
	platform_render_to_opengl *RenderToOpenGL;
	platform_upload_animated_model_to_gpu *UploadAnimatedModelToGPU;
	platform_upload_model_to_gpu *UploadModelToGPU;
	platform_upload_texture_to_gpu *UploadTextureToGPU;

#if DEVELOPER
	debug_platform_file_read_entire		*DebugFileReadEntire;
	debug_platform_file_write_entire	*DebugFileWriteEntire;
	debug_platform_file_free			*DebugFileFree;
	debug_platform_file_group_load		*DebugFileGroupLoad;
	debug_platform_file_is_dirty		*DebugFileIsDirty;
#endif
} platform_api;

inline u32
U64TruncateToU32(u64 U64)
{
	Assert(U64 <= 0xFFFFFFFF);
	u32 Result = (u32)U64;
	return(Result);
}

struct game_sound_buffer
{
	s32 SamplesPerSecond;
	s32 SampleCount;
	s16 *Samples;
};

struct game_button_state
{
	b32 EndedDown;
	u32 HalfTransitionCount;
};

inline b32
IsDown(game_button_state Button)
{
	b32 Result = Button.EndedDown;
	return(Result);
}

inline b32
WasDown(game_button_state Button)
{
	b32 Result = ((Button.HalfTransitionCount == 1) && (!Button.EndedDown));
	return(Result);
}

inline b32
WasPressed(game_button_state Button)
{
	b32 Result = (Button.HalfTransitionCount > 1) ||
				((Button.HalfTransitionCount == 1) && Button.EndedDown);
	return(Result);
}

enum 
{
	Key_MoveUp,
	Key_MoveLeft,
	Key_MoveRight,
	Key_MoveDown,

	Key_ActionUp,
	Key_ActionLeft,
	Key_ActionRight,
	Key_ActionDown,

	Key_LeftShoulder,
	Key_RightShoulder,

	Key_Start,
	Key_Back,

	Key_E,
	Key_Shift,
	Key_Space,
	Key_Enter,
	Key_Add,
	Key_Subtract,
	Key_Ctrl,
	Key_F9,
	Key_F10,

	Key_Count
};

enum mouse_button
{
	MouseButton_Left,
	MouseButton_Middle,
	MouseButton_Right,

	MouseButton_Count
};

struct game_controller_input
{
	b32 IsConnected;
	b32 IsAnalog;
	f32 StickAverageX;
	f32 StickAverageY;
	f32 StickdX;
	f32 StickdY;

	f32 RightStickAverageX;
	f32 RightStickAverageY;
	f32 RightStickdX;
	f32 RightStickdY;

	union
	{
		game_button_state Buttons[Key_Count];
		struct
		{
			game_button_state MoveForward;
			game_button_state MoveLeft;
			game_button_state MoveRight;
			game_button_state MoveBack;

			game_button_state ActionUp;
			game_button_state ActionLeft;
			game_button_state ActionRight;
			game_button_state ActionDown;

			game_button_state LeftShoulder;
			game_button_state RightShoulder;

			game_button_state Start;
			game_button_state Back;

			game_button_state E;
			game_button_state Shift;
			game_button_state Space;
			game_button_state Enter;
			game_button_state Add;
			game_button_state Subtract;
			game_button_state Ctrl;
			game_button_state F9;
			game_button_state F10;
		};
	};
};

struct game_input
{
	b32 ReloadingGame;
	f32 FPS;
	f32 DtForFrame;
	f32 MouseX, MouseY;
	f32 dXMouse, dYMouse;
	s32 BackBufferWidth, BackBufferHeight;

	game_controller_input Controllers[5];
	game_button_state MouseButtons[5];
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

inline game_controller_input *ControllerGet(game_input *Input, int unsigned ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return(Result);
}

#define GAME_AUDIO_UPDATE(FunctionName) void FunctionName(game_memory *GameMemory, game_sound_buffer *SoundBuffer)
typedef GAME_AUDIO_UPDATE(game_audio_update);


#ifdef __cplusplus
}
#endif

#define PLATFORM_H
#endif
