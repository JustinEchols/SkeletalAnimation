#include "platform.h"
#include "memory.h"
#include "intrinsics.h"
#include "math.h"
#include "strings.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"

#include <windows.h>
#include <dsound.h>
#include <xinput.h>
#include <gl/gl.h>
#include "wglext.h"
#include "glext.h"
#include "strings.cpp"
#include "win32_file_io.cpp"
#include "win32_opengl.cpp"

////////////////////////
// NOTE(Justin): Globals
global_variable b32 Win32GlobalRunning;
global_variable s64 Win32GlobalTicksPerSecond;
global_variable s32 Win32GlobalWindowWidth;
global_variable s32 Win32GlobalWindowHeight;
global_variable LPDIRECTSOUNDBUFFER Win32GlobalSecondaryBuffer;

global_variable f32 Win32GlobalMouseX;
global_variable f32 Win32GlobalMouseY;
global_variable f32 Win32GlobalMousedX;
global_variable f32 Win32GlobalMousedY;
global_variable b32 Win32GlobalMouseCentered;
global_variable open_gl OpenGL;
global_variable WINDOWPLACEMENT Win32GlobalWindowPos = {sizeof(Win32GlobalWindowPos)};

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

#include "renderer_opengl.h"
#include "renderer_opengl.cpp"

struct win32_sound_output
{
	int SamplesPerSecond;
	int BytesPerSample;
	u32 RunningSampleIndex;
	DWORD SecondaryBufferSize;
	DWORD SafetyButes;
};

struct win32_game_code
{
	b32 Valid;
	HMODULE DLL;
	FILETIME DLLLastWriteTime;
	game_update_and_render *UpdateAndRender;
	game_audio_update *AudioUpdate;
};

#if 0
internal void
Win32DirectSoundInitialize(HWND Window, s32 SamplesPerSecond, s32 SecondaryBufferSize)
{
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if(!DSoundLibrary)
	{
		// TODO: Diagnostic
		return;
	}

	direct_sound_create *DirectSoundCreate =
		(direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

	LPDIRECTSOUND DirectSound = {};
	if(!DirectSoundCreate && !SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
	{
		// TODO: Diagnostic
		return;
	}

	WAVEFORMATEX WaveFormat = {};
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = 2;
	WaveFormat.nSamplesPerSec = SamplesPerSecond;
	WaveFormat.wBitsPerSample = 16;	
	WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
	WaveFormat.cbSize = 0;

	if(!SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
	{
		// TODO: Diagnostic
		return;

	}

	DSBUFFERDESC BufferDesc = {};
	BufferDesc.dwSize = sizeof(BufferDesc);
	BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

	LPDIRECTSOUNDBUFFER PrimaryBuffer;
	if(!SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDesc, &PrimaryBuffer, 0)))
	{
		// TODO: Diagnostic
		return;
	}

	if(!SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
	{
		// TODO: Diagnostic
		return;
	}

	OutputDebugStringA("Primary buffer format set successfully.\n");
	BufferDesc = {};
	BufferDesc.dwSize = sizeof(BufferDesc);
	BufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
#if APP_INTERNAL
	BufferDesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
#endif
	BufferDesc.dwBufferBytes = SecondaryBufferSize;
	BufferDesc.lpwfxFormat = &WaveFormat;

	// NOTE(Justin): Using Win32GlobalSecondaryBuffer here!
	if(!SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDesc, &Win32GlobalSecondaryBuffer, 0)))
	{
		// TODO: Diagnostic
		return;
	}

	OutputDebugStringA("Secondary buffer created successfully.\n");
}
#else
internal void
Win32DirectSoundInitialize(HWND Window, s32 SamplesPerSecond, s32 SecondaryBufferSize)
{
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if(DSoundLibrary)
	{
		direct_sound_create *DirectSoundCreate =
			(direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND DirectSound;
		if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
		{
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;	
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
			WaveFormat.cbSize = 0;

			if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDesc = {};
				BufferDesc.dwSize = sizeof(BufferDesc);
				BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDesc, &PrimaryBuffer, 0)))
				{
					if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
					{
						OutputDebugStringA("Primary buffer format set successfully.\n");
					}
					else
					{
						// TODO: Diagnostic
					}
				}
				else
				{
					// TODO: Diagnostic
				}
			}
			else
			{
				// TODO: Diagnostic
			}

			DSBUFFERDESC BufferDesc = {};
			BufferDesc.dwSize = sizeof(BufferDesc);
			BufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
#if APP_INTERNAL
			BufferDesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
#endif
			BufferDesc.dwBufferBytes = SecondaryBufferSize;
			BufferDesc.lpwfxFormat = &WaveFormat;

			// NOTE(Justin): Using Win32GlobalSecondaryBuffer here!
			if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDesc, &Win32GlobalSecondaryBuffer, 0)))
			{
				OutputDebugStringA("Secondary buffer created successfully.\n");
			}
			else
			{
				// TODO: Diagnostic
			}
		}
		else
		{
			// TODO: Diagnostic
		}
	}
	else
	{
		// TODO: Diagnostic
	}
}
#endif

internal void
Win32SoundBufferClear(win32_sound_output *SoundOutput)
{
	VOID *Region1;
	DWORD Region1Size;
	VOID *Region2;
	DWORD Region2Size;
	if(SUCCEEDED(Win32GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
													&Region1, &Region1Size,
													&Region2, &Region2Size, 0)))
	{
		u8 *DestSample = (u8 *)Region1;
		for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
		{
			*DestSample++ = 0;
		}

		DestSample = (u8 *)Region2;
		for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
		{
			*DestSample++ = 0;
		}

		Win32GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

internal void
Win32SoundBufferFill(win32_sound_output *Win32SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
		game_sound_buffer *SourceBuffer)
{
	VOID *Region1;
	DWORD Region1Size;
	VOID *Region2;
	DWORD Region2Size;
	if(SUCCEEDED(Win32GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
												 &Region1, &Region1Size,
												 &Region2, &Region2Size,
												 0)))
	{
		DWORD Region1SampleCount = Region1Size / Win32SoundOutput->BytesPerSample;
		s16 *DestSample = (s16 *)Region1;
		s16 *SrcSample = SourceBuffer->Samples;
		for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
		{
			*DestSample++ = *SrcSample++;
			*DestSample++ = *SrcSample++;
			++Win32SoundOutput->RunningSampleIndex;
		}

		DWORD Region2SampleCount = Region2Size / Win32SoundOutput->BytesPerSample;
		DestSample = (s16 *)Region2;
		for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
		{
			*DestSample++ = *SrcSample++;
			*DestSample++ = *SrcSample++;
			++Win32SoundOutput->RunningSampleIndex;
		}

		Win32GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

inline LARGE_INTEGER
Win32WallClock(void)
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return(Result);
}

inline f32
Win32SecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	Assert(Win32GlobalTicksPerSecond > 0.0f);
	f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)Win32GlobalTicksPerSecond);
	return(Result);
}

internal win32_game_code 
Win32GameCodeLoad(char *SrcDLLName, char *TempDLLName, char *LockFileName)
{

	win32_game_code Result = {};

	WIN32_FILE_ATTRIBUTE_DATA Ignored;
	if(!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
	{
		Result.DLLLastWriteTime = Win32FileLastWriteTime(SrcDLLName);
		CopyFile(SrcDLLName, TempDLLName, FALSE);
		Result.DLL = LoadLibraryA(TempDLLName);
		if(Result.DLL)
		{
			Result.UpdateAndRender = (game_update_and_render *)GetProcAddress(Result.DLL, "GameUpdateAndRender");
			Result.Valid = (Result.UpdateAndRender != 0);
		}
	}

	if(!Result.Valid)
	{
		Result.UpdateAndRender = 0;
	}

	return(Result);
}

internal void 
Win32GameCodeUnload(win32_game_code *Game)
{
	if(Game->DLL)
	{
		FreeLibrary(Game->DLL);
		Game->DLL = 0;
	}
}

internal void 
Win32MousePositionGet(HWND Window)
{
	// TODO(Justin): Routine hides the fact that globals are updated. Is this ok to do?
	POINT Mouse;
	GetCursorPos(&Mouse);
	ScreenToClient(Window, &Mouse);
	Win32GlobalMouseX = (f32)Mouse.x;
	Win32GlobalMouseY = (f32)Win32GlobalWindowHeight - (f32)Mouse.y; // Invert MouseY st y=0 is the bottom of the client area
}

internal void 
Win32MousePositionCenter(HWND Window)
{
	RECT WindowRect;
	GetWindowRect(Window, &WindowRect);
	u32 CenterX = WindowRect.left + (Win32GlobalWindowWidth / 2);
	u32 CenterY = WindowRect.top + (Win32GlobalWindowHeight / 2);

	RECT SnapPosRect;
	SnapPosRect.left = (s32)CenterX;
	SnapPosRect.right = (s32)CenterX;
	SnapPosRect.top = (s32)CenterY;
	SnapPosRect.bottom = (s32)CenterY;
	ClipCursor(&SnapPosRect);	
}

internal void
Win32GlobalClientDimUpdate(HWND Window)
{
	// TODO(Justin): Routine hides the fact that globals are updated. Is this ok to do?
	RECT ClientR = {};
	GetClientRect(Window, &ClientR);
	Win32GlobalWindowWidth = ClientR.right - ClientR.left;
	Win32GlobalWindowHeight = ClientR.bottom - ClientR.top;
}

internal v2i
Win32ClientDimGet(HWND Window)
{
	v2i Result = {};

	RECT ClientR = {};
	GetClientRect(Window, &ClientR);
	Result.width = ClientR.right - ClientR.left;
	Result.height = ClientR.bottom - ClientR.top;

	return(Result);
}

internal void
Win32FullScreen(HWND Window)
{
	// NOTE(Justin): This follows Raymond Chen's solution, for fullscreen toggling. See:
	// http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &Win32GlobalWindowPos) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &Win32GlobalWindowPos);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

LRESULT CALLBACK
Win32WindowCallBack(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
    switch(Message)
    {
		case WM_CLOSE:
		case WM_DESTROY:
		{
			Win32GlobalRunning = false;
		} break;
		case WM_SIZE:
		{
			Win32GlobalClientDimUpdate(Window);
			glViewport(0, 0, (u32)Win32GlobalWindowWidth, (u32)Win32GlobalWindowHeight);
		} break;
		case WM_MOVE:
		{
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT PS;
			HDC DC = BeginPaint(Window, &PS);
			EndPaint(Window, &PS);
		} break;
		default:
		{
			Result = DefWindowProc(Window, Message, wParam, lParam);
		}
    }

	return(Result);
}

internal void 
Win32KeyStateUpdate(game_button_state *Button, b32 IsDown)
{
	if(Button->EndedDown != IsDown)
	{
		Button->EndedDown = IsDown;
		Button->HalfTransitionCount++;
	}
}

internal void
Win32XInputDigitalButtonUpdate(DWORD XInputButtonState,
							   game_button_state *OldState, DWORD ButtonBit,
							   game_button_state *NewState)
{
	NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1: 0;
}

internal f32
Win32XInputStickValueUpdate(SHORT Value, SHORT DeadZoneThreshold)
{
    f32 Result = 0;

    if(Value < -DeadZoneThreshold)
    {
        Result = (f32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (f32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(Result);
}

int WINAPI
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
	LARGE_INTEGER TicksPerSecond;
	QueryPerformanceFrequency(&TicksPerSecond);
	Win32GlobalTicksPerSecond = TicksPerSecond.QuadPart;

	UINT DesiredSchedulerMS = 1;
	b32 SchedulerGranularitySet = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    WNDCLASSA WindowClass = {};

	WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc   = Win32WindowCallBack;
    WindowClass.hInstance     = Instance;
    WindowClass.lpszClassName = "Main Window";

    RegisterClass(&WindowClass);

    HWND Window = CreateWindowExA(0,
								  WindowClass.lpszClassName,
								  "Skeletal Animation",
								  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								  CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
								  0,
								  0,
								  Instance,
								  0);
	if(Window)
	{
		Win32MousePositionGet(Window);

		s32 MonitorRefreshRate = 60;
		HDC RefreshDC = GetDC(Window);
		s32 Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
		ReleaseDC(Window, RefreshDC);

		s32 GameUpdateHz = 0;
		if(Win32RefreshRate > 1)
		{
			MonitorRefreshRate = Win32RefreshRate;
		}

		f32 GameRefreshRate = (f32)MonitorRefreshRate / 2.0f;
		f32 TargetSecondsPerFrame = 1.0f / GameRefreshRate;

		win32_sound_output Win32SoundOutput = {};
		Win32SoundOutput.SamplesPerSecond = 48000;
		Win32SoundOutput.BytesPerSample = sizeof(s16) * 2;
		Win32SoundOutput.SecondaryBufferSize = Win32SoundOutput.SamplesPerSecond * Win32SoundOutput.BytesPerSample;
		Win32SoundOutput.SafetyButes = (int)(((f32)Win32SoundOutput.SamplesPerSecond * (f32)Win32SoundOutput.BytesPerSample / GameRefreshRate) / 3.0f);
		Win32DirectSoundInitialize(Window, Win32SoundOutput.SamplesPerSecond, Win32SoundOutput.SecondaryBufferSize);
		Win32SoundBufferClear(&Win32SoundOutput);
		Win32GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
		u32 PaddingForSIMDAlignment = 2 * 4 * sizeof(s16);
		s16 *SoundSamples = (s16 *)VirtualAlloc(0, Win32SoundOutput.SecondaryBufferSize + PaddingForSIMDAlignment, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		
		HGLRC OpenGLRC		= Win32OpenGLInitialize(GetDC(Window), &OpenGL);
		OpenGL.MainShader	= GLProgramCreate(MainVS, MainFS);
		OpenGL.FontShader	= GLProgramCreate(FontVS, FontFS);
		OpenGL.Quad2dShader = GLProgramCreate(Quad2dVS, Quad2dFS);
		OpenGL.DebugShadowMapShader = GLProgramCreate(DebugShadowMapVS, DebugShadowMapFS);
		OpenGL.ShadowMapShader = GLProgramCreate(ShadowMapVS, ShadowMapFS);
		OpenGL.DebugBBoxShader = GLProgramCreate(DebugBBoxVS, DebugBBoxFS);

		glGenTextures(1, &OpenGL.NullTexture);
		OpenGLAllocateQuad2d(&OpenGL.Quad2dVA, &OpenGL.Quad2dVB, OpenGL.Quad2dShader);
		OpenGLAllocateQuad3d(&OpenGL.Quad3dVA, &OpenGL.Quad3dVB, OpenGL.MainShader);

		OpenGL.TextureWidth = 256;
		OpenGL.TextureHeight = 256;
		OpenGLFrameBufferInitialize(&OpenGL.FBO,
								    &OpenGL.TextureHandle,
								    &OpenGL.RBO,
								    OpenGL.TextureWidth, 
									OpenGL.TextureHeight);

		OpenGL.ShadowMapWidth = 2048;
		OpenGL.ShadowMapHeight = 2048;
		//OpenGL.ShadowMapWidth = 4096;
		//OpenGL.ShadowMapHeight = 4096;
		OpenGLShadowMapInitialize(&OpenGL.ShadowMapFBO,
							      &OpenGL.ShadowMapHandle,
							      OpenGL.ShadowMapWidth,
								  OpenGL.ShadowMapHeight);

		game_memory GameMemory = {};
		GameMemory.IsInitialized = false;
		GameMemory.PermanentStorageSize = Megabyte(256);
		GameMemory.PermanentStorage		= VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		GameMemory.TemporaryStorageSize = Gigabyte(1);
		GameMemory.TemporaryStorage		= VirtualAlloc(0, GameMemory.TemporaryStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		GameMemory.PlatformAPI.RenderToOpenGL		= RenderBufferToOutput;
		GameMemory.PlatformAPI.UploadAnimatedModelToGPU	= OpenGLAllocateAnimatedModel;
		GameMemory.PlatformAPI.UploadModelToGPU		= OpenGLAllocateModel;
		GameMemory.PlatformAPI.UploadTextureToGPU	= OpenGLAllocateTexture;

		GameMemory.PlatformAPI.DebugFileReadEntire	= DebugPlatformFileReadEntire;
		GameMemory.PlatformAPI.DebugFileWriteEntire = DebugPlatformFileWriteEntire;
		GameMemory.PlatformAPI.DebugFileFree		= DebugPlatformFileFree;
		GameMemory.PlatformAPI.DebugFileGroupLoad	= DebugPlatformFileGroupLoad;
		GameMemory.PlatformAPI.DebugFileIsDirty		= DebugPlatformFileIsDirty;

		if(!SoundSamples || !GameMemory.PermanentStorage || !GameMemory.TemporaryStorage)
		{
			// TODO(Justin): Diagnostic
			return(0);
		}

		game_input GameInput[2] = {};
		game_input *NewInput = &GameInput[0];
		game_input *OldInput = &GameInput[1];

		win32_game_code Game = Win32GameCodeLoad("../build/game.dll", "../build/game_temp.dll", "../build/lock.tmp");

		Win32GlobalRunning = true;

		DWORD AudioLatencyBytes = 0;
		f32 AudioLatencySeconds = 0.0f;
		b32 SoundIsValid = false;

		LARGE_INTEGER QueryTickCount;
		QueryPerformanceCounter(&QueryTickCount);
		s64 TickCountStart = QueryTickCount.QuadPart;

		LARGE_INTEGER LastTickCount = Win32WallClock();
		LARGE_INTEGER FrameBoundaryTickCount = Win32WallClock();

		while(Win32GlobalRunning)
		{
			if(Win32FileHasUpdated("../build/game.dll", Game.DLLLastWriteTime))
			{
				Win32GameCodeUnload(&Game);
				Game = Win32GameCodeLoad("../build/game.dll", "../build/game_temp.dll", "../build/lock.tmp");
				NewInput->ReloadingGame = true;
			}

			NewInput->DtForFrame = TargetSecondsPerFrame;
			game_controller_input *OldKeyboard = ControllerGet(OldInput, 0);
			game_controller_input *NewKeyboard = ControllerGet(NewInput, 0);
			*NewKeyboard = {};
			for(u32 ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboard->Buttons); ++ButtonIndex)
			{
				NewKeyboard->Buttons[ButtonIndex].EndedDown =
					OldKeyboard->Buttons[ButtonIndex].EndedDown;
			}

			MSG Message = {};
			while(PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
			{
				switch(Message.message)
				{
					case WM_SYSKEYDOWN:
					case WM_SYSKEYUP:
					case WM_KEYDOWN:
					case WM_KEYUP:
					{
						u32 KeyCode = (u32)Message.wParam;
						b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
						b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
						if(WasDown != IsDown)
						{
							if(KeyCode == 'W')
							{
								Win32KeyStateUpdate(&NewKeyboard->MoveForward, IsDown);
							}
							else if(KeyCode == 'A')
							{
								Win32KeyStateUpdate(&NewKeyboard->MoveLeft, IsDown);
							}
							else if(KeyCode == 'S')
							{
								Win32KeyStateUpdate(&NewKeyboard->MoveBack, IsDown);
							}
							else if(KeyCode == 'D')
							{
								Win32KeyStateUpdate(&NewKeyboard->MoveRight, IsDown);
							}
							else if(KeyCode == 'E')
							{
								Win32KeyStateUpdate(&NewKeyboard->E, IsDown);
							}
							else if(KeyCode == VK_SHIFT)
							{
								Win32KeyStateUpdate(&NewKeyboard->Shift, IsDown);
							}
							else if(KeyCode == VK_SPACE)
							{
								Win32KeyStateUpdate(&NewKeyboard->Space, IsDown);
							}
							else if(KeyCode == VK_RETURN)
							{
								Win32KeyStateUpdate(&NewKeyboard->Enter, IsDown);
							}
							else if(KeyCode == VK_ADD)
							{
								Win32KeyStateUpdate(&NewKeyboard->Add, IsDown);
							}
							else if(KeyCode == VK_SUBTRACT)
							{
								Win32KeyStateUpdate(&NewKeyboard->Subtract, IsDown);
							}
							else if(KeyCode == VK_CONTROL)
							{
								Win32KeyStateUpdate(&NewKeyboard->Ctrl, IsDown);
							}
							else if(KeyCode == VK_F9)
							{
								Win32KeyStateUpdate(&NewKeyboard->F9, IsDown);
							}
							else if(KeyCode == VK_F10)
							{
								Win32KeyStateUpdate(&NewKeyboard->F10, IsDown);
							}
						}

						if(IsDown)
						{
							b32 AltKeyWasDown = (Message.lParam & (1 << 29));
							if(AltKeyWasDown && (KeyCode == VK_RETURN))
							{
								if(Message.hwnd)
								{
									Win32FullScreen(Message.hwnd);
								}
							}

							if(AltKeyWasDown && (KeyCode == VK_F4))
							{
								Win32GlobalRunning = false;
							}

							if(!AltKeyWasDown && KeyCode == VK_RETURN)
							{
								Win32KeyStateUpdate(&NewKeyboard->Start, IsDown);
							}
						}

						if(WasDown && (KeyCode == VK_F10))
						{
							Win32GlobalMouseCentered = !Win32GlobalMouseCentered;
						}
					} break;
					default:
					{
						TranslateMessage(&Message);
						DispatchMessage(&Message);
					} break;
				};
			}

			DWORD Win32MouseKeyCodes[] = 
			{
				VK_LBUTTON,
				VK_MBUTTON,
				VK_RBUTTON,
				VK_XBUTTON1,
				VK_XBUTTON2
			};

			for(u32 ButtonIndex = 0; ButtonIndex < ArrayCount(NewInput->MouseButtons); ++ButtonIndex)
			{
				NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
				NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
				Win32KeyStateUpdate(&NewInput->MouseButtons[ButtonIndex], GetKeyState(Win32MouseKeyCodes[ButtonIndex]) & (1 << 15));
			}

			Win32MousePositionGet(Window);
			NewInput->dXMouse = Win32GlobalMouseX - OldInput->MouseX;
			NewInput->dYMouse = Win32GlobalMouseY - OldInput->MouseY;
			NewInput->MouseX  = Win32GlobalMouseX;
			NewInput->MouseY  = Win32GlobalMouseY;
			NewInput->BackBufferWidth = Win32GlobalWindowWidth;
			NewInput->BackBufferHeight = Win32GlobalWindowHeight;

			DWORD MaxControllerCount = XUSER_MAX_COUNT;
			if(MaxControllerCount > ArrayCount(NewInput->Controllers) - 1)
			{
				MaxControllerCount = ArrayCount(NewInput->Controllers) - 1;
			}

			for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
			{
				// Keyboard is the first controller
				DWORD OurControllerIndex = ControllerIndex + 1;
				game_controller_input *OldController = ControllerGet(OldInput, OurControllerIndex);
				game_controller_input *NewController = ControllerGet(NewInput, OurControllerIndex);

				XINPUT_STATE ControllerState;
				if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
				{
					NewController->IsConnected = true;
					//NewController->IsAnalog = OldController->IsAnalog;
					NewController->IsAnalog = true;

					// TODO(Justin): See if ControllerState.dwPacketNumber increments too rapidly
					XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

					f32 StickAverageX = Win32XInputStickValueUpdate(
							Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
					f32 StickAverageY = Win32XInputStickValueUpdate(
							Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

					f32 RightStickAverageX = Win32XInputStickValueUpdate(
							Pad->sThumbLX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
					f32 RightStickAverageY = Win32XInputStickValueUpdate(
							Pad->sThumbLY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

					NewController->StickAverageX = StickAverageX;
					NewController->StickAverageY = StickAverageY;
					NewController->StickdX = NewController->StickAverageX - OldController->StickAverageX;
					NewController->StickdY = NewController->StickAverageY - OldController->StickAverageY;

					NewController->RightStickAverageX = RightStickAverageX;
					NewController->RightStickAverageY = RightStickAverageY;
					NewController->RightStickdX = NewController->RightStickAverageX - OldController->RightStickAverageX;
					NewController->RightStickdY = NewController->RightStickAverageY - OldController->RightStickAverageY;

					if((NewController->StickAverageX != 0.0f) ||
					   (NewController->StickAverageY != 0.0f))
					{
						NewController->IsAnalog = true;
					}

					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
					{
						NewController->StickAverageY = 1.0f;
						NewController->IsAnalog = false;
					}

					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
					{
						NewController->StickAverageY = -1.0f;
						NewController->IsAnalog = false;
					}

					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
					{
						NewController->StickAverageX = -1.0f;
						NewController->IsAnalog = false;
					}

					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
					{
						NewController->StickAverageX = 1.0f;
						NewController->IsAnalog = false;
					}

					f32 Threshold = 0.5f;
					Win32XInputDigitalButtonUpdate(
							(NewController->StickAverageX < -Threshold) ? 1 : 0,
							&OldController->MoveLeft, 1,
							&NewController->MoveLeft);
					Win32XInputDigitalButtonUpdate(
							(NewController->StickAverageX > Threshold) ? 1 : 0,
							&OldController->MoveRight, 1,
							&NewController->MoveRight);
					Win32XInputDigitalButtonUpdate(
							(NewController->StickAverageY < -Threshold) ? 1 : 0,
							&OldController->MoveBack, 1,
							&NewController->MoveBack);
					Win32XInputDigitalButtonUpdate(
							(NewController->StickAverageY > Threshold) ? 1 : 0,
							&OldController->MoveForward, 1,
							&NewController->MoveForward);

					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->ActionDown, XINPUT_GAMEPAD_A,
							&NewController->ActionDown);
					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->ActionRight, XINPUT_GAMEPAD_B,
							&NewController->ActionRight);
					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->ActionLeft, XINPUT_GAMEPAD_X,
							&NewController->ActionLeft);
					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->ActionUp, XINPUT_GAMEPAD_Y,
							&NewController->ActionUp);
					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
							&NewController->LeftShoulder);
					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
							&NewController->RightShoulder);

					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->Start, XINPUT_GAMEPAD_START,
							&NewController->Start);
					Win32XInputDigitalButtonUpdate(Pad->wButtons,
							&OldController->Back, XINPUT_GAMEPAD_BACK,
							&NewController->Back);
				}
				else
				{
					NewController->IsConnected = false;
				}
			}

			// Do game

			if(Game.UpdateAndRender)
			{
				Game.UpdateAndRender(&GameMemory, NewInput);
			}

			// Do platform audio

			LARGE_INTEGER AudioWallClock = Win32WallClock();
			f32 TickCountFromFrameToAudioStart = Win32SecondsElapsed(FrameBoundaryTickCount, AudioWallClock);

			DWORD PlayCursor;
			DWORD WriteCursor;
			if(Win32GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
			{
				if(!SoundIsValid)
				{
					Win32SoundOutput.RunningSampleIndex = WriteCursor / Win32SoundOutput.BytesPerSample;
					SoundIsValid = true;
				}

				DWORD ByteToLock = (Win32SoundOutput.RunningSampleIndex * Win32SoundOutput.BytesPerSample) % Win32SoundOutput.SecondaryBufferSize;
				DWORD ExpectedSoundBytesPerFrame = (int)((f32)(Win32SoundOutput.SamplesPerSecond * Win32SoundOutput.BytesPerSample) / GameRefreshRate);
				f32 SecondsLeftUntilFlip = TargetSecondsPerFrame - TickCountFromFrameToAudioStart;
				DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip / TargetSecondsPerFrame) * (f32)ExpectedSoundBytesPerFrame);
				DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
				DWORD SafeWriteCursor = WriteCursor;
				if(SafeWriteCursor < PlayCursor)
				{
					SafeWriteCursor += Win32SoundOutput.SecondaryBufferSize;
				}
				Assert(SafeWriteCursor >= PlayCursor);
				SafeWriteCursor += Win32SoundOutput.SafetyButes;

				b32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

				DWORD TargetCursor = 0;
				if(AudioCardIsLowLatency)
				{
					TargetCursor = ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame;
				}
				else
				{
					TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame + Win32SoundOutput.SafetyButes);
				}
				TargetCursor = (TargetCursor % Win32SoundOutput.SecondaryBufferSize);

				DWORD BytesToWrite = 0;
				if(ByteToLock > TargetCursor)
				{
					BytesToWrite = Win32SoundOutput.SecondaryBufferSize - ByteToLock;
					BytesToWrite += TargetCursor;
				}
				else
				{
					BytesToWrite = TargetCursor - ByteToLock;
				}

				game_sound_buffer SoundBuffer = {};
				SoundBuffer.SamplesPerSecond = Win32SoundOutput.SamplesPerSecond;
				SoundBuffer.SampleCount = BytesToWrite / Win32SoundOutput.BytesPerSample;
				SoundBuffer.Samples = SoundSamples;
				if(Game.AudioUpdate)
				{
					Game.AudioUpdate(&GameMemory, &SoundBuffer);
				}

				Win32SoundBufferFill(&Win32SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);

			}
			else
			{
				SoundIsValid = false;
			}


			// Do timings

			LARGE_INTEGER WorkTickCount = Win32WallClock();
			f32 SecondsElapsedForWork = Win32SecondsElapsed(LastTickCount, WorkTickCount);
			f32 SecondsElapsedForFrame = SecondsElapsedForWork;
			if(SecondsElapsedForFrame < TargetSecondsPerFrame)
			{
				if(SchedulerGranularitySet)
				{
					DWORD TimeToSleep = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
					if(TimeToSleep > 0)
					{
						Sleep(TimeToSleep);
					}
				}

				f32 NewSecondsElapsedForFrame = Win32SecondsElapsed(LastTickCount, Win32WallClock());
				if(NewSecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					// TODO(Justin): Missed sleep.
				}

				while(SecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					SecondsElapsedForFrame = Win32SecondsElapsed(LastTickCount, Win32WallClock());
				}
			}
			else
			{
				// TODO(Justin): Missed frame.
			}

			// Tick count right before flip
			LARGE_INTEGER EndTickCount = Win32WallClock();

			HDC WindowDC = GetDC(Window);
			SwapBuffers(WindowDC);
			ReleaseDC(Window, WindowDC);

			// Tick count right after flip
			FrameBoundaryTickCount = Win32WallClock();

			s64 ElapsedTickCount = EndTickCount.QuadPart - LastTickCount.QuadPart;
			f32 MsPerFrame = (f32)(((1000.0f * (f32)(ElapsedTickCount)) / (f32)Win32GlobalTicksPerSecond));
			f32 FPS = (f32)Win32GlobalTicksPerSecond / (f32)ElapsedTickCount;
			LastTickCount = EndTickCount;

			NewInput->FPS = FPS;
			game_input *Temp = NewInput;
			NewInput = OldInput;
			OldInput = Temp;
		}
	}

    return(0);
}
