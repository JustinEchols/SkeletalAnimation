
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

global_varible b32 Win32GlobalRunning;
global_varible s64 Win32GlobalTicksPerSecond;
global_varible int Win32GlobalWindowWidth;
global_varible int Win32GlobalWindowHeight;

#include "game.h"
#include "game.cpp"

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
			RECT ClientR = {};
			GetClientRect(Window, &ClientR);
			Win32GlobalWindowWidth = ClientR.right - ClientR.left;
			Win32GlobalWindowHeight = ClientR.bottom - ClientR.top;
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
		GameMemory.PermanentStorageSize = Megabyte(512);
		GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		GameMemory.TemporaryStorageSize = Megabyte(512);
		GameMemory.TemporaryStorage = VirtualAlloc(0, GameMemory.TemporaryStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		game_input GameInput[2] = {};
		game_input *NewInput = &GameInput[0];
		game_input *OldInput = &GameInput[1];

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
								if(NewKeyboard->W.EndedDown != IsDown)
								{
									NewKeyboard->W.EndedDown = IsDown;
									NewKeyboard->W.HalfTransitionCount++;
								}
							}
							else if(KeyCode == 'A')
							{
								if(NewKeyboard->A.EndedDown != IsDown)
								{
									NewKeyboard->Buttons[Key_A].EndedDown = IsDown;
									NewKeyboard->Buttons[Key_A].HalfTransitionCount++;
								}
							}
							else if(KeyCode == 'S')
							{

								if(NewKeyboard->S.EndedDown != IsDown)
								{
									NewKeyboard->Buttons[Key_S].EndedDown = IsDown;
									NewKeyboard->Buttons[Key_S].HalfTransitionCount++;
								}
							}
							else if(KeyCode == 'D')
							{

								if(NewKeyboard->D.EndedDown != IsDown)
								{
									NewKeyboard->Buttons[Key_D].EndedDown = IsDown;
									NewKeyboard->Buttons[Key_D].HalfTransitionCount++;
								}
							}
							else if(KeyCode == 'E')
							{

								if(NewKeyboard->E.EndedDown != IsDown)
								{
									NewKeyboard->Buttons[Key_E].EndedDown = IsDown;
									NewKeyboard->Buttons[Key_E].HalfTransitionCount++;
								}
							}
							else if(KeyCode == VK_SHIFT)
							{
								if(NewKeyboard->Shift.EndedDown != IsDown)
								{
									NewKeyboard->Buttons[Key_Shift].EndedDown = IsDown;
									NewKeyboard->Buttons[Key_Shift].HalfTransitionCount++;
								}
							}
							else if(KeyCode == VK_SPACE)
							{

								if(NewKeyboard->Space.EndedDown != IsDown)
								{
									NewKeyboard->Buttons[Key_Space].EndedDown = IsDown;
									NewKeyboard->Buttons[Key_Space].HalfTransitionCount++;
								}
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


