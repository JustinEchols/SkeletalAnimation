
#include "platform.h"

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl/gl.h>

#include "win32_file_io.cpp"
#include "win32_opengl.h"

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

internal void
Win32PixelFormatSet(HDC WindowDC)
{
	s32 PFDIndex = 0;
	GLuint EXTPick = 0;
	if(wglChoosePixelFormatARB)
	{
		s32 PixelFormatList[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_RED_BITS_ARB, 8,
			WGL_GREEN_BITS_ARB, 8,
			WGL_BLUE_BITS_ARB, 8,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 4,
			0,	
		};

		wglChoosePixelFormatARB(WindowDC, PixelFormatList, 0, 1, &PFDIndex, &EXTPick);
	}

	if(!EXTPick)
	{
		PIXELFORMATDESCRIPTOR PFD = {};
		PFD.nSize = sizeof(PFD);
		PFD.nVersion = 1;
		PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		PFD.iPixelType = PFD_TYPE_RGBA;
		PFD.cColorBits = 32;
		PFD.cAlphaBits = 8;
		PFD.cDepthBits = 24;
		PFD.cStencilBits = 8;
		PFD.iLayerType = PFD_MAIN_PLANE;

		PFDIndex = ChoosePixelFormat(WindowDC, &PFD);
	}

	PIXELFORMATDESCRIPTOR ActualPFD;
	DescribePixelFormat(WindowDC, PFDIndex, sizeof(ActualPFD), &ActualPFD);
	SetPixelFormat(WindowDC, PFDIndex, &ActualPFD);
}

internal void
Win32WGLExtensionsLoad(void)
{
	WNDCLASSA WindowClass = {};

	WindowClass.lpfnWndProc = DefWindowProcA;
	WindowClass.hInstance = GetModuleHandle(0);
	WindowClass.lpszClassName = "WGLLoader";

	if(RegisterClassA(&WindowClass))
	{
		HWND Window = CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Skeletal Animation",
				0,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				WindowClass.hInstance,
				0);

		HDC WindowDC = GetDC(Window);
		Win32PixelFormatSet(WindowDC);
		HGLRC OpenGLRC = wglCreateContext(WindowDC);
		if(wglMakeCurrent(WindowDC, OpenGLRC))
		{
			wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
			wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
			wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
			wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");

			if(wglGetExtensionsStringEXT)
			{
				// TODO(Justin): Parse extension strings here to determine frame
				// buffer formats that are supported and store result in open_gl
				// info struct

				wglMakeCurrent(0, 0);
			}
		}

		wglDeleteContext(OpenGLRC);
		ReleaseDC(Window, WindowDC);
		DestroyWindow(Window);
	}
}

int OpenGLAttribList[] =
{
	WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
	WGL_CONTEXT_MINOR_VERSION_ARB, 3,
	WGL_CONTEXT_FLAGS_ARB, (WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB),
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
	0
};

internal HGLRC
Win32OpenGLInit(HDC WindowDC)
{
	Win32WGLExtensionsLoad();
	Win32PixelFormatSet(WindowDC);

	b32 ModernContext = true;
	HGLRC OpenGLRC = 0;
	if(wglCreateContextAttribsARB)
	{
		OpenGLRC = wglCreateContextAttribsARB(WindowDC, 0, OpenGLAttribList);
	}

	if(!OpenGLRC)
	{
		ModernContext = false;
		OpenGLRC = wglCreateContext(WindowDC);
	}

	if(wglMakeCurrent(WindowDC, OpenGLRC))
	{
		glAttachShader 				= (gl_attach_shader *)wglGetProcAddress("glAttachShader");
		glCompileShader 			= (gl_compile_shader *)wglGetProcAddress("glCompileShader");
		glCreateProgram 			= (gl_create_program *)wglGetProcAddress("glCreateProgram");
		glCreateShader 				= (gl_create_shader *)wglGetProcAddress("glCreateShader");
		glDeleteProgram 			= (gl_delete_program *)wglGetProcAddress("glDeleteProgram");
		glDeleteShader 				= (gl_delete_shader *)wglGetProcAddress("glDeleteShader");
		glEnableVertexAttribArray 	= (gl_enable_vertex_attrib_array *)wglGetProcAddress("glEnableVertexAttribArray");
		glGetActiveAttrib 			= (gl_get_active_attrib *)wglGetProcAddress("glGetActiveAttrib");
		glGetProgramiv 				= (gl_get_programiv *)wglGetProcAddress("glGetProgramiv");
		glGetProgramInfoLog 		= (gl_get_program_info_log *)wglGetProcAddress("glGetProgramInfoLog");
		glGetShaderiv 				= (gl_get_shaderiv *)wglGetProcAddress("glGetShaderiv");
		glGetUniformLocation 		= (gl_get_uniform_location *)wglGetProcAddress("glGetUniformLocation");
		glBindVertexArray 			= (gl_bind_vertex_array *)wglGetProcAddress("glBindVertexArray");
		glGenVertexArrays 			= (gl_gen_vertex_arrays *)wglGetProcAddress("glGenVertexArrays");
		glBindBuffer 				= (gl_bind_buffer *)wglGetProcAddress("glBindBuffer");
		glGenBuffers 				= (gl_gen_buffers *)wglGetProcAddress("glGenBuffers");
		glVertexAttribPointer 		= (gl_vertex_attrib_pointer *)wglGetProcAddress("glVertexAttribPointer");
		glVertexAttribIPointer 		= (gl_vertex_attribi_pointer *)wglGetProcAddress("glVertexAttribIPointer");
		glBufferData 				= (gl_buffer_data *)wglGetProcAddress("glBufferData");
		glLinkProgram 				= (gl_link_program *)wglGetProcAddress("glLinkProgram");
		glShaderSource 				= (gl_shader_source *)wglGetProcAddress("glShaderSource");
		glUseProgram 				= (gl_use_program *)wglGetProcAddress("glUseProgram");
		glValidateProgram 			= (gl_validate_program *)wglGetProcAddress("glValidateProgram");
		glUniform1ui 				= (gl_uniform_1ui *)wglGetProcAddress("glUniform1ui");
		glUniform1f 				= (gl_uniform_1f *)wglGetProcAddress("glUniform1f");
		glUniform3fv 				= (gl_uniform_3fv *)wglGetProcAddress("glUniform3fv");
		glUniform4fv 				= (gl_uniform_4fv *)wglGetProcAddress("glUniform4fv");
		glUniformMatrix4fv 			= (gl_uniform_matrix_4fv *)wglGetProcAddress("glUniformMatrix4fv");
	}

	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(1);
	}

	return(OpenGLRC);
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
		game_input *OldInput = &GameInput[0];
		game_input *NewInput = &GameInput[1];

		Win32GlobalRunning = true;
		f32 Angle = 0.0f;
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
				NewKeyboard->Buttons[ButtonIndex].IsDown =
					OldKeyboard->Buttons[ButtonIndex].IsDown;
			}

			MSG Message = {};
			while(PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
			{
				//UINT MessageType = Message.message;
				switch(Message.message)
				{
					// NOTE(Justin): ALL THESE CASES ARE BUNDLED TOGETHER!!!! 
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
								if(NewKeyboard->W.IsDown != IsDown)
								{
									NewKeyboard->W.IsDown = IsDown;
									NewKeyboard->W.HalfTransitionCount++;
								}
							}
							else if(KeyCode == 'A')
							{
								NewKeyboard->Buttons[Key_A].IsDown = IsDown;
								NewKeyboard->Buttons[Key_A].HalfTransitionCount++;
							}
							else if(KeyCode == 'S')
							{
								NewKeyboard->Buttons[Key_S].IsDown = IsDown;
								NewKeyboard->Buttons[Key_S].HalfTransitionCount++;
							}
							else if(KeyCode == 'D')
							{
								NewKeyboard->Buttons[Key_D].IsDown = IsDown;
								NewKeyboard->Buttons[Key_D].HalfTransitionCount++;
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


