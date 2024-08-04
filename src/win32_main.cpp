
#include "platform.h"

#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>

global_varible b32 Win32GlobalRunning;
global_varible s64 Win32GlobalTicksPerSecond;

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9

#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_SAMPLES_ARB							0x2042

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int *piAttributes,
    int *piValues);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv_arb(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int *piAttributes,
    FLOAT *pfValues);

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
    const int *piAttribIList,
    const FLOAT *pfAttribFList,
    UINT nMaxFormats,
    int *piFormats,
    UINT *nNumFormats);

typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef const char * WINAPI wgl_get_extensions_string_ext(void);

typedef char GLchar;
typedef uintptr_t GLsizeiptr;

typedef void 	WINAPI gl_attach_shader(GLuint program, GLuint shader);
typedef void 	WINAPI gl_compile_shader(GLuint shader);
typedef GLuint	WINAPI gl_create_program(void);
typedef GLuint	WINAPI gl_create_shader(GLenum type);
typedef void	WINAPI gl_delete_program(GLuint program);
typedef void 	WINAPI gl_delete_shader(GLuint shader);
typedef void 	WINAPI gl_enable_vertex_attrib_array(GLuint index);
typedef void 	WINAPI gl_get_active_attrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void 	WINAPI gl_get_programiv(GLuint program, GLenum pname, GLint *params);
typedef void 	WINAPI gl_get_program_info_log(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void 	WINAPI gl_get_shaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void 	WINAPI gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint	WINAPI gl_get_uniform_location(GLuint program, const GLchar *name);
typedef void	WINAPI gl_bind_vertex_array(GLuint array);
typedef void 	WINAPI gl_gen_vertex_arrays(GLsizei n, GLuint *arrays);
typedef void 	WINAPI gl_bind_buffer(GLenum target, GLuint buffer);
typedef void 	WINAPI gl_gen_buffers(GLsizei n, GLuint *buffers);
typedef void 	WINAPI gl_vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void 	WINAPI gl_vertex_attribi_pointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void 	WINAPI gl_buffer_data(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void 	WINAPI gl_link_program(GLuint program);
typedef void 	WINAPI gl_shader_source(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void 	WINAPI gl_use_program(GLuint program);
typedef void 	WINAPI gl_validate_program(GLuint program);
typedef void 	WINAPI gl_uniform_1ui(GLint location, GLuint v0);
typedef void 	WINAPI gl_uniform_1f(GLint location, GLfloat v0);
typedef void 	WINAPI gl_uniform_3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void 	WINAPI gl_uniform_4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void 	WINAPI gl_uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

global_varible gl_attach_shader                 *glAttachShader;
global_varible gl_compile_shader                *glCompileShader;
global_varible gl_create_program                *glCreateProgram;
global_varible gl_create_shader                 *glCreateShader;
global_varible gl_delete_program                *glDeleteProgram;
global_varible gl_delete_shader                 *glDeleteShader;
global_varible gl_enable_vertex_attrib_array    *glEnableVertexAttribArray;
global_varible gl_get_active_attrib             *glGetActiveAttrib;
global_varible gl_get_programiv                 *glGetProgramiv;
global_varible gl_get_program_info_log          *glGetProgramInfoLog;
global_varible gl_get_shaderiv                  *glGetShaderiv;
global_varible gl_get_shader_info_log           *glGetShaderInfoLog;
global_varible gl_get_uniform_location          *glGetUniformLocation;

global_varible gl_bind_vertex_array				*glBindVertexArray;
global_varible gl_gen_vertex_arrays				*glGenVertexArrays;
global_varible gl_bind_buffer					*glBindBuffer;
global_varible gl_gen_buffers					*glGenBuffers;
global_varible gl_buffer_data					*glBufferData;

global_varible gl_vertex_attrib_pointer			*glVertexAttribPointer;
global_varible gl_vertex_attribi_pointer		*glVertexAttribIPointer;

global_varible gl_link_program					*glLinkProgram;
global_varible gl_shader_source					*glShaderSource;
global_varible gl_use_program					*glUseProgram;
global_varible gl_validate_program				*glValidateProgram;

global_varible gl_uniform_1ui					*glUniform1ui;
global_varible gl_uniform_1f					*glUniform1f;
global_varible gl_uniform_3fv					*glUniform3fv;
global_varible gl_uniform_4fv					*glUniform4fv;
global_varible gl_uniform_matrix_4fv			*glUniformMatrix4fv;


global_varible wgl_create_context_attribs_arb	*wglCreateContextAttribsARB;
global_varible wgl_choose_pixel_format_arb		*wglChoosePixelFormatARB;
global_varible wgl_swap_interval_ext			*wglSwapIntervalEXT;
global_varible wgl_get_extensions_string_ext	*wglGetExtensionsStringEXT;

global_varible int Win32GlobalWindowWidth;
global_varible int Win32GlobalWindowHeight;

#include "math_util.h"
#include "strings.h"
#include "strings.cpp"
#include "mesh.h"
#include "mesh.cpp"
#include "animation.cpp"

internal void
Win32FileFree(void *Memory)
{
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

internal debug_file
Win32FileReadEntire(char *FileName)
{
	debug_file Result = {};
	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			u32 FileSize32 = U64TruncateToU32(FileSize.QuadPart);
			Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(Result.Content)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, 0) && (BytesRead == FileSize32))
				{
					Result.Size = FileSize32;
				}
				else
				{
					Win32FileFree(Result.Content);
					Result.Content = 0;
				}
			}
			else
			{
			}
		}
		else
		{
		}
		CloseHandle(FileHandle);
	}
	else
	{
	}

	return(Result);
}

#include "asset.cpp"
#include "opengl.cpp"

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

		void *Memory = VirtualAlloc(0, Megabyte(256), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		memory_arena Arena_;
		ArenaInitialize(&Arena_, (u8 *)Memory, Megabyte(256));
		memory_arena *Arena = &Arena_;

		model *Models[3] = {};
		Models[0] = PushStruct(Arena, model);
		*Models[0] = ModelLoad(Arena, "..\\data\\XBot.mesh");

		char *AnimationFiles[] =
		{
			"..\\data\\XBot_ActionIdle.animation",
			"..\\data\\XBot_RightTurn.animation",
			"..\\data\\XBot_LeftTurn.animation",
			"..\\data\\XBot_RunningToTurn.animation",
			"..\\data\\XBot_RunningChangeDirection.animation",
			"..\\data\\XBot_IdleToSprint.animation",
			"..\\data\\XBot_ActionIdleToStandingIdle.animation",
			"..\\data\\XBot_IdleLookAround.animation",
			"..\\data\\XBot_FemaleWalk.animation",
			"..\\data\\XBot_PushingStart.animation",
			"..\\data\\XBot_Pushing.animation",
			"..\\data\\XBot_PushingStop.animation",
		};

		Models[0]->Animations.Count = ArrayCount(AnimationFiles);
		Models[0]->Animations.Info = PushArray(Arena, Models[0]->Animations.Count, animation_info);
		for(u32 AnimIndex = 0; AnimIndex < ArrayCount(AnimationFiles); ++AnimIndex)
		{
			animation_info *Info = Models[0]->Animations.Info + AnimIndex;
			*Info = AnimationInfoLoad(Arena, AnimationFiles[AnimIndex]);
		}
		
		Models[0]->Basis.O = V3(0.0f, -80.0f, -400.0f);
		Models[0]->Basis.X = XAxis();
		Models[0]->Basis.Y = YAxis();
		Models[0]->Basis.Z = ZAxis();
		mat4 Scale = Mat4Identity();

		//
		// NOTE(Justin): Transformations
		//

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

			//
			// NOTE(Justin): HACKED animation.
			//

			for(u32 ModelIndex = 0; ModelIndex < ArrayCount(Models); ++ModelIndex)
			{
				model *Model = Models[ModelIndex];
				if(Model)
				{
					if(Model->HasSkeleton)
					{
						AnimationUpdate(Model, DtForFrame);
					}
				}
			}

			//
			// NOTE(Justin): Render.
			//


			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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


