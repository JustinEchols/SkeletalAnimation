
// NOTE(Justin): Must include this here. Need to further investigate what is causing the conflicts..
#include <ft2build.h>
#include FT_FREETYPE_H

#include "platform.h"
#include <windows.h>

#include <stdio.h>
#include <math.h>
#include <gl/gl.h>
#include "wglext.h"
#include "glext.h"
#include "win32_file_io.cpp"
#include "win32_opengl.cpp"

// NOTE(Justin): Globals
global_varible b32 Win32GlobalRunning;
global_varible s64 Win32GlobalTicksPerSecond;
global_varible int Win32GlobalWindowWidth;
global_varible int Win32GlobalWindowHeight;
global_varible f32 Win32GlobalMouseX;
global_varible f32 Win32GlobalMouseY;

#include "game.h"
//#include "render.h"
//#include "render.cpp"
#include "game.cpp"



// TODO(Justin): Routine hides the fact that globals are updated. Is this ok to do?
internal void 
Win32MousePositionGet(HWND Window)
{
	POINT Mouse;
	GetCursorPos(&Mouse);
	ScreenToClient(Window, &Mouse);
	Win32GlobalMouseX = (f32)Mouse.x;
	Win32GlobalMouseY = (f32)Win32GlobalWindowHeight - (f32)Mouse.y; // Invert MouseY st y=0 is the bottom of the client area
}

// TODO(Justin): Routine hides the fact that globals are updated. Is this ok to do?
internal void
Win32ClientRectGet(HWND Window)
{
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

		HGLRC OpenGLRC = Win32OpenGLInit(GetDC(Window));

		game_memory GameMemory = {};
		GameMemory.IsInitialized = false;
		GameMemory.PermanentStorageSize = Megabyte(64);
		GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		GameMemory.TemporaryStorageSize = Gigabyte(1);
		GameMemory.TemporaryStorage = VirtualAlloc(0, GameMemory.TemporaryStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		game_input GameInput[2] = {};
		game_input *NewInput = &GameInput[0];
		game_input *OldInput = &GameInput[1];

		Win32MousePositionGet(Window);

		Win32GlobalRunning = true;
		f32 DtForFrame = 0.0f;
		LARGE_INTEGER QueryTickCount;
		QueryPerformanceCounter(&QueryTickCount);
		s64 TickCountStart = QueryTickCount.QuadPart;
		while(Win32GlobalRunning)
		{
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
					// NOTE(Justin): All these cases are bundled together!
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
						}
					} break;
					default:
					{
						TranslateMessage(&Message);
						DispatchMessage(&Message);
					} break;
				};
			}

			Win32MousePositionGet(Window);
			Win32KeyStateUpdate(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
			Win32KeyStateUpdate(&NewInput->MouseButtons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
			Win32KeyStateUpdate(&NewInput->MouseButtons[2], GetKeyState(VK_RBUTTON) & (1 << 15));
			Win32KeyStateUpdate(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
			Win32KeyStateUpdate(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));

			NewInput->dXMouse = Win32GlobalMouseX - OldInput->MouseX;
			NewInput->dYMouse = Win32GlobalMouseY - OldInput->MouseY;
			NewInput->MouseX  = Win32GlobalMouseX;
			NewInput->MouseY  = Win32GlobalMouseY;
			NewInput->BackBufferWidth = Win32GlobalWindowWidth;
			NewInput->BackBufferHeight = Win32GlobalWindowHeight;

			HDC WindowDC = GetDC(Window);

			GameUpdateAndRender(&GameMemory, NewInput);

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
