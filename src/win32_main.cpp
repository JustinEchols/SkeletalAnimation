#include "platform.h"
#include "memory.h"
#include "intrinsics.h"
#include "math.h"
#include "strings.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"

#include <windows.h>
#include <gl/gl.h>
#include "wglext.h"
#include "glext.h"
#include "win32_file_io.cpp"
#include "win32_opengl.cpp"

// NOTE(Justin): Globals
global_varible b32 Win32GlobalRunning;
global_varible s64 Win32GlobalTicksPerSecond;
global_varible s32 Win32GlobalWindowWidth;
global_varible s32 Win32GlobalWindowHeight;
global_varible f32 Win32GlobalMouseX;
global_varible f32 Win32GlobalMouseY;

static open_gl OpenGL;
#include "renderer_opengl.h"
#include "renderer_opengl.cpp"

struct win32_game_code
{
	b32 Valid;
	HMODULE DLL;
	FILETIME DLLLastWriteTime;
	game_update_and_render *UpdateAndRender;
};

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
Win32ClientRectGet(HWND Window)
{
	// TODO(Justin): Routine hides the fact that globals are updated. Is this ok to do?
	RECT ClientR = {};
	GetClientRect(Window, &ClientR);
	Win32GlobalWindowWidth = ClientR.right - ClientR.left;
	Win32GlobalWindowHeight = ClientR.bottom - ClientR.top;
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
			Win32ClientRectGet(Window);
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
Win32KeyStateUpdate(game_button *Button, b32 IsDown)
{
	if(Button->EndedDown != IsDown)
	{
		Button->EndedDown = IsDown;
		Button->HalfTransitionCount++;
	}
}

int WINAPI
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
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
								  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								  0,
								  0,
								  Instance,
								  0);
	if(Window)
	{
		LARGE_INTEGER TicksPerSecond;
		QueryPerformanceFrequency(&TicksPerSecond);
		Win32GlobalTicksPerSecond = TicksPerSecond.QuadPart;

		s32 MonitorRefreshRate = GetDeviceCaps(GetDC(Window), VREFRESH);
		s32 GameUpdateHz = 0;
		if(MonitorRefreshRate > 1)
		{
			GameUpdateHz = MonitorRefreshRate;
		}

		f32 TargetSecondsPerFrame = 1.0f / (f32)MonitorRefreshRate;

		HGLRC OpenGLRC		= Win32OpenGLInit(GetDC(Window), &OpenGL);
		OpenGL.MainShader	= GLProgramCreate(MainVS, MainFS);
		OpenGL.BasicShader	= GLProgramCreate(BasicVS, BasicFS);
		OpenGL.FontShader	= GLProgramCreate(FontVS, FontFS);
		OpenGL.ScreenShader = GLProgramCreate(ScreenVS, ScreenFS);
		OpenGL.ShadowMapShader = GLProgramCreate(ShadowMapVS, ShadowMapFS);

		glGenTextures(1, &OpenGL.NullTexture);
		OpenGLAllocateQuad2d(&OpenGL.Quad2dVA, &OpenGL.Quad2dVB, OpenGL.ScreenShader);
		OpenGLAllocateQuad3d(&OpenGL.Quad3dVA, &OpenGL.Quad3dVB, OpenGL.BasicShader);

		OpenGL.TextureWidth = 256;
		OpenGL.TextureHeight = 256;
		OpenGLFrameBufferInit(&OpenGL.FBO,
							  &OpenGL.TextureHandle,
							  &OpenGL.RBO,
							  OpenGL.TextureWidth, OpenGL.TextureHeight);

		OpenGL.ShadowMapWidth = 1024;
		OpenGL.ShadowMapHeight = 1024;
		OpenGLFrameBufferInit(&OpenGL.ShadowMapFBO,
							  &OpenGL.ShadowMapHandle,
							  &OpenGL.ShadowMapRBO,
							  OpenGL.ShadowMapWidth, OpenGL.ShadowMapHeight);

		game_memory GameMemory = {};
		GameMemory.IsInitialized = false;
		GameMemory.PermanentStorageSize = Megabyte(64);
		GameMemory.PermanentStorage		= VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		GameMemory.TemporaryStorageSize = Gigabyte(1);
		GameMemory.TemporaryStorage		= VirtualAlloc(0, GameMemory.TemporaryStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		GameMemory.PlatformAPI.RenderToOpenGL		= RenderBufferToOutput;
		GameMemory.PlatformAPI.DebugFileReadEntire	= DebugPlatformFileReadEntire;
		GameMemory.PlatformAPI.DebugFileWriteEntire = DebugPlatformFileWriteEntire;
		GameMemory.PlatformAPI.DebugFileFree		= DebugPlatformFileFree;

		game_input GameInput[2] = {};
		game_input *NewInput = &GameInput[0];
		game_input *OldInput = &GameInput[1];

		Win32MousePositionGet(Window);

		win32_game_code Game = Win32GameCodeLoad("../build/game.dll", "../build/game_temp.dll", "../build/lock.tmp");

		Win32GlobalRunning = true;
		f32 DtForFrame = 0.0f;
		LARGE_INTEGER QueryTickCount;
		QueryPerformanceCounter(&QueryTickCount);
		s64 TickCountStart = QueryTickCount.QuadPart;
		while(Win32GlobalRunning)
		{
			if(Win32FileHasUpdated("../build/game.dll", Game.DLLLastWriteTime))
			{
				Win32GameCodeUnload(&Game);
				Game = Win32GameCodeLoad("../build/game.dll", "../build/game_temp.dll", "../build/lock.tmp");
			}

			NewInput->DtForFrame = TargetSecondsPerFrame;
			game_keyboard *OldKeyboard = &OldInput->Keyboard;
			game_keyboard *NewKeyboard = &NewInput->Keyboard;
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
								Win32KeyStateUpdate(&NewKeyboard->W, IsDown);
							}
							else if(KeyCode == 'A')
							{
								Win32KeyStateUpdate(&NewKeyboard->A, IsDown);
							}
							else if(KeyCode == 'S')
							{
								Win32KeyStateUpdate(&NewKeyboard->S, IsDown);
							}
							else if(KeyCode == 'D')
							{
								Win32KeyStateUpdate(&NewKeyboard->D, IsDown);
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

			if(Game.UpdateAndRender)
			{
				Game.UpdateAndRender(&GameMemory, NewInput);
			}

			HDC WindowDC = GetDC(Window);
			SwapBuffers(WindowDC);
			ReleaseDC(Window, WindowDC);

			QueryPerformanceCounter(&QueryTickCount);
			s64 TickCountEnd = QueryTickCount.QuadPart;

			DtForFrame = (f32)(((f64)TickCountEnd - (f64)TickCountStart) / (f64)Win32GlobalTicksPerSecond);
			TickCountStart = TickCountEnd;

			game_input *Temp = NewInput;
			NewInput = OldInput;
			OldInput = Temp;
		}
	}

    return(0);
}
