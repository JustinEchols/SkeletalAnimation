
#include "platform.h"

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl/gl.h>

#include "win32_fileio.cpp"
#include "win32_opengl.h"
#include "intrinsics.h"
#include "math_util.h"
#include "strings.h"
#include "strings.cpp"
#include "mesh.h"
#include "mesh.cpp"
#include "animation.h"
#include "animation.cpp"
#include "asset.h"
#include "asset.cpp"

global_varible b32 Win32GlobalRunning;
global_varible s64 Win32GlobalTicksPerSecond;
global_varible int Win32GlobalWindowWidth;
global_varible int Win32GlobalWindowHeight;

char *AnimationFiles[] =
{
	"..\\data\\XBot_IdleToSprint.animation",
	"..\\data\\XBot_Running.animation",
	"..\\data\\XBot_ActionIdle.animation",
	"..\\data\\XBot_IdleLookAround.animation",
	"..\\data\\XBot_RightTurn.animation",
	"..\\data\\XBot_LeftTurn.animation",
	"..\\data\\XBot_PushingStart.animation",
	"..\\data\\XBot_Pushing.animation",
	"..\\data\\XBot_PushingStop.animation",
	"..\\data\\XBot_ActionIdleToStandingIdle.animation",
	"..\\data\\XBot_RunningToTurn.animation",
	"..\\data\\XBot_RunningChangeDirection.animation",
	"..\\data\\XBot_FemaleWalk.animation",
};

#include "opengl.cpp"
//#include "game.h"
//#include "game.cpp"

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

    HWND Window = CreateWindowExA(
						0,
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

		void *Memory = VirtualAlloc(0, Megabyte(512), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if 0
		game_memory GameMemory = {};
		GameMemory.IsInitialized = false;
		GameMemory.PermanentStorageSize = Megabyte(512);
		GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		GameMemory.TemporaryStorageSize = Megabyte(512);
		GameMemory.TemporaryStorage = VirtualAlloc(0, GameMemory.TemporaryStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#endif

		memory_arena Arena_;
		ArenaInitialize(&Arena_, (u8 *)Memory, Megabyte(256));
		memory_arena *Arena = &Arena_;

		memory_arena TempArena_;
		ArenaInitialize(&TempArena_, (u8 *)(Arena->Base + Arena->Size), Megabyte(64));
		memory_arena *TempArena = &TempArena_;

		model *Models[3] = {};

		Models[0] = PushStruct(Arena, model);
		*Models[0] = ModelLoad(Arena, "..\\data\\XBot.mesh");
		Models[0]->Basis.O = V3(0.0f, -80.0f, -400.0f);
		Models[0]->Basis.X = XAxis();
		Models[0]->Basis.Y = YAxis();
		Models[0]->Basis.Z = ZAxis();
		mat4 Scale = Mat4Identity();

		animation_player AnimationPlayer = {};
		AnimationPlayerInitialize(&AnimationPlayer, Models[0], Arena);

		animation_info *AnimationInfos = PushArray(Arena, ArrayCount(AnimationFiles), animation_info);
		animation *Animations = PushArray(Arena, ArrayCount(AnimationFiles), animation);
		for(u32 AnimIndex = 0; AnimIndex < ArrayCount(AnimationFiles); ++AnimIndex)
		{
			animation_info *Info = AnimationInfos + AnimIndex;
			animation *Animation = Animations + AnimIndex;

			*Info = AnimationLoad(Arena, AnimationFiles[AnimIndex]);
			if(Info)
			{
				Animation->ID.Value = AnimIndex;
				Animation->Info = Info;
			}
		}

		//NOTE(Justin): Transformations


		v3 CameraP = V3(0.0f, 5.0f, 3.0f);
		v3 Direction = V3(0.0f, 0.0f, -1.0f);
		mat4 CameraTransform = Mat4Camera(CameraP, CameraP + Direction);

		RECT ClientR = {};
		GetClientRect(Window, &ClientR);
		Win32GlobalWindowWidth = ClientR.right - ClientR.left;
		Win32GlobalWindowHeight = ClientR.bottom - ClientR.top;

		f32 FOV = DegreeToRad(45.0f);
		f32 Aspect = (f32)Win32GlobalWindowWidth / Win32GlobalWindowHeight;
		f32 ZNear = 0.1f;
		f32 ZFar = 100.0f;
		mat4 PerspectiveTransform = Mat4Perspective(FOV, Aspect, ZNear, ZFar);

		glViewport(0, 0, (u32)Win32GlobalWindowWidth, (u32)Win32GlobalWindowHeight);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

		//
		// NOTE(Justin): Opengl info initialization
		//

#if 1
		u32 Shaders[2];
		Shaders[0] = GLProgramCreate(BasicVsSrc, BasicFsSrc);
		for(u32 ModelIndex = 0; ModelIndex < ArrayCount(Models); ++ModelIndex)
		{
			model *Model = Models[ModelIndex];
			if(Model)
			{
				if(Model->HasSkeleton)
				{
					OpenGLAllocateAnimatedModel(Models[ModelIndex], Shaders[0]);
					glUseProgram(Shaders[0]);
					UniformMatrixSet(Shaders[0], "View", CameraTransform);
					UniformMatrixSet(Shaders[0], "Projection", PerspectiveTransform);
					UniformV3Set(Shaders[0], "CameraP", CameraP);

				}
				else
				{
					OpenGLAllocateModel(Models[ModelIndex], Shaders[1]);
					glUseProgram(Shaders[1]);
					UniformMatrixSet(Shaders[1], "View", CameraTransform);
					UniformMatrixSet(Shaders[1], "Projection", PerspectiveTransform);
				}
			}
		}
#endif

		Win32GlobalRunning = true;
		f32 Angle = 0.0f;
		f32 DtForFrame = 0.0f;
		LARGE_INTEGER QueryTickCount;
		QueryPerformanceCounter(&QueryTickCount);
		s64 TickCountStart = QueryTickCount.QuadPart;
		while(Win32GlobalRunning)
		{
			MSG Message = {};
			while(PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}

			HDC WindowDC = GetDC(Window);

			//GameUpdateAndRender(&GameMemory, DtForFrame);

#if 1
			for(u32 ModelIndex = 0; ModelIndex < ArrayCount(Models); ++ModelIndex)
			{
				model *Model = Models[ModelIndex];
				if(Model)
				{
					if(Model->HasSkeleton)
					{
						AnimationPlay(&AnimationPlayer, &Animations[0], true, false);
						AnimationPlayerUpdate(&AnimationPlayer, TempArena, DtForFrame);
						ModelUpdate(&AnimationPlayer);
					}
				}
			}

			//
			// NOTE(Justin): Render.
			//


			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Angle += DtForFrame;
			for(u32 ModelIndex = 0; ModelIndex < ArrayCount(Models); ++ModelIndex)
			{
				model *Model = Models[ModelIndex];
				if(Model)
				{
					if(Model->HasSkeleton)
					{
						glUseProgram(Shaders[0]);
						UniformV3Set(Shaders[0], "LightDir", V3(2.0f * cosf(Angle), 0.0f, 2.0f * sinf(Angle)));
						OpenGLDrawAnimatedModel(Models[ModelIndex], Shaders[0]);
					}
					else
					{
						glUseProgram(Shaders[1]);
						f32 A = 20.0f * Angle;
						mat4 R = Mat4YRotation(DegreeToRad(A));
						UniformMatrixSet(Shaders[1], "Model", Mat4Translate(Models[ModelIndex]->Basis.O) * R * Scale);
						OpenGLDrawModel(Models[ModelIndex], Shaders[1]);
					}
				}
			}
#endif
			SwapBuffers(WindowDC);
			ReleaseDC(Window, WindowDC);

			QueryPerformanceCounter(&QueryTickCount);
			s64 TickCountEnd = QueryTickCount.QuadPart;

			DtForFrame = (f32)(((f64)TickCountEnd - (f64)TickCountStart) / (f64)Win32GlobalTicksPerSecond);
			TickCountStart = TickCountEnd;
		}
	}

    return 0;
}


