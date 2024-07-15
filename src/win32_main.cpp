
#include "skeletal_platform.h"

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
#include "xml.h"
#include "xml.cpp"
#include "mesh.h"
#include "mesh.cpp"
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
			glViewport(0.0f, 0.0f, Win32GlobalWindowWidth, Win32GlobalWindowHeight);

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

internal u32
MeshHeaderNumberGet(memory_arena *Arena, string_node *Line, u8 *LineDelimeters, u32 DelimCount)
{
	u32 Result = 0;

	string_list LineData = StringSplit(Arena, Line->String, LineDelimeters, DelimCount);
	string_node *Node = LineData.First;
	Node = Node->Next;
	string Value = Node->String;

	Result = U32FromASCII(Value.Data);

	return(Result);
}

internal void
ParseMeshU32Array(memory_arena *Arena, u32 **U32, u32 *Count, string_node *Line, u8 *Delimeters, u32 DelimCount,
		char *HeaderName)
{
	string_list LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	string_node *Node = LineData.First;
	string Header = Node->String;
	Header.Data[Header.Size] = 0;
	Assert(StringsAreSame(Header, HeaderName));
	Node = Node->Next;
	string StrCount = Node->String;
	StrCount.Data[StrCount.Size] = 0;

	*Count  = U32FromASCII(StrCount.Data);
	*U32 = PushArray(Arena, *Count, u32);

	Line = Line->Next;
	LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	Assert(LineData.Count == *Count);
	Node = LineData.First;

	u32 *A = *U32;
	for(u32 Index = 0; Index < *Count; ++Index)
	{
		string Value = Node->String;
		Value.Data[Value.Size] = 0;
		A[Index] = U32FromASCII(Value);
		Node = Node->Next;
	}
}

internal void
ParseMeshF32Array(memory_arena *Arena, f32 **F32, u32 *Count, string_node *Line, u8 *Delimeters, u32 DelimCount,
		char *HeaderName)
{
	string_list LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	string_node *Node = LineData.First;
	string Header = Node->String;
	Header.Data[Header.Size] = 0;
	Assert(StringsAreSame(Header, HeaderName));
	Node = Node->Next;
	string StrCount = Node->String;
	StrCount.Data[StrCount.Size] = 0;

	*Count  = U32FromASCII(StrCount.Data);
	*F32 = PushArray(Arena, *Count, f32);

	Line = Line->Next;
	LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	Assert(LineData.Count == *Count);
	Node = LineData.First;

	f32 *A = *F32;
	for(u32 Index = 0; Index < *Count; ++Index)
	{
		string Value = Node->String;
		Value.Data[Value.Size] = 0;
		A[Index] = F32FromASCII(Value);
		Node = Node->Next;
	}
}

internal b32
DoneProcessingMesh(string_node *Line)
{
	b32 Result = (Line->String.Data[0] == '*');
	return(Result);
}

internal model
ModelLoad(memory_arena *Arena, char *FileName)
{
	model Model = {};

	debug_file File = Win32FileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;
		Content[File.Size] = 0;
		string Data = String(Content);

		u8 Delimeters[] = "\n";
		u8 LineDelimeters[] = " \n\r";
		string Count = {};

		string_list Lines = StringSplit(Arena, Data, Delimeters, 1);
		string_node *Line = Lines.First;

		Model.MeshCount = MeshHeaderNumberGet(Arena, Line, LineDelimeters, 3);
		Model.Meshes = PushArray(Arena, Model.MeshCount, mesh);

		for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
		{
			mesh *Mesh = Model.Meshes + MeshIndex;

			Line = Line->Next;
			string_list LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			string_node *Node = LineData.First;
			string Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "MESH:"));

			Node = Node->Next;
			string MeshName = Node->String;
			MeshName.Data[MeshName.Size] = 0;
			Mesh->Name = MeshName;

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "LIGHTING:"));

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 4);
			Node = LineData.First;

			for(u32 Index = 0; Index < 4; ++Index)
			{
				string Float = Node->String;
				Float.Data[Float.Size] = 0;
				Mesh->MaterialSpec.Ambient.E[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 4);
			Node = LineData.First;

			for(u32 Index = 0; Index < 4; ++Index)
			{
				string Float = Node->String;
				Float.Data[Float.Size] = 0;
				Mesh->MaterialSpec.Diffuse.E[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 4);
			Node = LineData.First;

			for(u32 Index = 0; Index < 4; ++Index)
			{
				string Float = Node->String;
				Float.Data[Float.Size] = 0;
				Mesh->MaterialSpec.Specular.E[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 1);
			Node = LineData.First;
			string Float = Node->String;
			Float.Data[Float.Size] = 0;
			Mesh->MaterialSpec.Shininess = F32FromASCII(Float);

			Line = Line->Next;
			ParseMeshU32Array(Arena, &Mesh->Indices, &Mesh->IndicesCount, Line, LineDelimeters, 3, "INDICES:");

			Line = Line->Next;

			Line = Line->Next;
			Line = Line->Next;
			ParseMeshF32Array(Arena, &Mesh->Positions, &Mesh->PositionsCount, Line, LineDelimeters, 3, "POSITIONS:");

			Line = Line->Next;
			Line = Line->Next;
			ParseMeshF32Array(Arena, &Mesh->Normals, &Mesh->NormalsCount, Line, LineDelimeters, 3, "NORMALS:");

			Line = Line->Next;
			Line = Line->Next;
			ParseMeshF32Array(Arena, &Mesh->UV, &Mesh->UVCount, Line, LineDelimeters, 3, "UVS:");

			Line = Line->Next;
			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "JOINT_INFO:"));

			Node = Node->Next;
			Count = Node->String;
			Count.Data[Count.Size] = 0;

			Mesh->JointInfoCount = U32FromASCII(Count);
			Mesh->JointsInfo = PushArray(Arena, Mesh->JointInfoCount, joint_info);
			for(u32 Index = 0; Index < Mesh->JointInfoCount; ++Index)
			{
				joint_info *Info = Mesh->JointsInfo + Index;

				Line = Line->Next;

				Count = Line->String;
				Count.Data[Count.Size] = 0;

				Info->Count = U32FromASCII(Count);

				Line = Line->Next;
				string_list IndicesList = StringSplit(Arena, Line->String, LineDelimeters, 3);

				Node = IndicesList.First;
				string StrIndex = Node->String;
				StrIndex.Data[StrIndex.Size] = 0;
				Info->JointIndex[0] = U32FromASCII(StrIndex);

				Node = Node->Next;
				StrIndex = Node->String;
				StrIndex.Data[StrIndex.Size] = 0;
				Info->JointIndex[1] = U32FromASCII(StrIndex);

				Node = Node->Next;
				StrIndex = Node->String;
				StrIndex.Data[StrIndex.Size] = 0;
				Info->JointIndex[2] = U32FromASCII(StrIndex);

				Line = Line->Next;
				string_list WeightsList = StringSplit(Arena, Line->String, LineDelimeters, 3);

				Node = WeightsList.First;
				string Weight = Node->String;
				Weight.Data[Weight.Size] = 0;
				Info->Weights[0] = F32FromASCII(Weight);

				Node = Node->Next;
				Weight = Node->String;
				Weight.Data[Weight.Size] = 0;
				Info->Weights[1] = F32FromASCII(Weight);

				Node = Node->Next;
				Weight = Node->String;
				Weight.Data[Weight.Size] = 0;
				Info->Weights[2] = F32FromASCII(Weight);
			}

			Line = Line->Next;
			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "JOINTS:"));

			Node = Node->Next;
			Count = Node->String;
			Count.Data[Count.Size] = 0;

			Mesh->JointCount = U32FromASCII(Count);
			Mesh->Joints = PushArray(Arena, Mesh->JointCount, joint);
			Mesh->JointNames = PushArray(Arena, Mesh->JointCount, string);
			Mesh->JointTransforms = PushArray(Arena, Mesh->JointCount, mat4);
			Mesh->ModelSpaceTransforms = PushArray(Arena, Mesh->JointCount, mat4);

			Line = Line->Next;
			string RootJointName = Line->String;
			RootJointName.Data[RootJointName.Size] = 0;

			Mesh->Joints[0].Name = RootJointName;
			Mesh->Joints[0].ParentIndex = -1;

			Line = Line->Next;
			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			f32 *T = &Mesh->Joints[0].Transform.E[0][0];
			for(u32 Index = 0; Index < 16; Index++)
			{
				Float = Node->String;
				Float.Data[Float.Size] = 0;
				T[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			for(u32 Index = 1; Index < Mesh->JointCount; ++Index)
			{
				Line = Line->Next;

				joint *Joint = Mesh->Joints + Index;

				string JointName = Line->String;
				JointName.Data[JointName.Size] = 0;
				Joint->Name = JointName;

				Line = Line->Next;
				string ParentIndex = Line->String;
				ParentIndex.Data[ParentIndex.Size] = 0;
				Joint->ParentIndex = U32FromASCII(ParentIndex);

				Line = Line->Next;
				LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
				Node = LineData.First;
				T = &Joint->Transform.E[0][0];
				for(u32 k = 0; k < 16; k++)
				{
					Float = Node->String;
					Float.Data[Float.Size] = 0;
					T[k] = F32FromASCII(Float);
					Node = Node->Next;
				}
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "BIND:"));

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			f32 *BindT = &Mesh->BindTransform.E[0][0];
			for(u32 Index = 0; Index < 16; Index++)
			{
				Float = Node->String;
				Float.Data[Float.Size] = 0;
				BindT[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "INV_BIND:"));

			Node = Node->Next;
			Count = Node->String;
			Count.Data[Count.Size] = 0;
			u32 InvBindTCount = U32FromASCII(Count);
			Assert(InvBindTCount == (Mesh->JointCount * 16));

			Mesh->InvBindTransforms = PushArray(Arena, Mesh->JointCount, mat4);
			for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
			{
				Line = Line->Next;
				LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
				Node = LineData.First;

				mat4 *InvBindT = Mesh->InvBindTransforms + Index;
				T = &InvBindT->E[0][0];

				for(u32 k = 0; k < 16; k++)
				{
					Float = Node->String;
					Float.Data[Float.Size] = 0;
					T[k] = F32FromASCII(Float);
					Node = Node->Next;
				}
			}

			Line = Line->Next;
			Line = Line->Next;
			Assert(DoneProcessingMesh(Line));

			mat4 I = Mat4Identity();
			for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
			{
				joint *Joint = Mesh->Joints + Index;

				Mesh->JointNames[Index] = Joint->Name;
				Mesh->JointTransforms[Index] = I;
				Mesh->ModelSpaceTransforms[Index] = I;
			}
		}
	}

	return(Model);
}

internal animation_info
AnimationInfoLoad(memory_arena *Arena, char *FileName)
{
	animation_info Info = {};
	debug_file File = Win32FileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;
		Content[File.Size] = 0;
		string Data = String(Content);
		string Header = {};
		string Count = {};

		char Delimeters[] = "\n\r";
		char LineDelimeters[] = " \n\r";

		string_list Lines = StringSplit(Arena, Data,(u8 *)Delimeters, 1);
		string_node *Line = Lines.First;

		string_list LineData = StringSplit(Arena, Line->String,(u8 *)LineDelimeters, 3);
		string_node *Node = LineData.First;
		Header = Node->String;
		Header.Data[Header.Size] = 0;
		Assert(StringsAreSame(Header, "JOINTS:"));

		Node = Node->Next;
		Count = Node->String;
		Count.Data[Count.Size] = 0;

		Info.JointCount = U32FromASCII(Count);

		Line = Line->Next;
		LineData = StringSplit(Arena, Line->String,(u8 *)LineDelimeters, 3);
		Node = LineData.First;
		Header = Node->String;
		Header.Data[Header.Size] = 0;
		Assert(StringsAreSame(Header, "TIMES:"));

		Node = Node->Next;
		Count = Node->String;
		Count.Data[Count.Size] = 0;

		Info.TimeCount = U32FromASCII(Count);

		Line = Line->Next;
		LineData = StringSplit(Arena, Line->String,(u8 *)LineDelimeters, 3);
		Node = LineData.First;
		Header = Node->String;
		Header.Data[Header.Size] = 0;
		Assert(StringsAreSame(Header, "TRANSFORMS:"));

		Node = Node->Next;
		Count = Node->String;
		Count.Data[Count.Size] = 0;

		Info.TransformCount = U32FromASCII(Count);
		Assert(Info.TimeCount == Info.TransformCount);

		Line = Line->Next;
		Assert(Line->String.Data[0] == '*');

		Info.JointNames = PushArray(Arena, Info.JointCount, string);
		Info.Times = PushArray(Arena, Info.JointCount, f32 *);
		Info.Transforms = PushArray(Arena, Info.JointCount, mat4 *);

		for(u32 JointIndex = 0; JointIndex < Info.JointCount; ++JointIndex)
		{
			Info.Times[JointIndex] = PushArray(Arena, Info.TimeCount, f32);
			Info.Transforms[JointIndex] = PushArray(Arena, Info.TimeCount, mat4);

			Line = Line->Next;
			string JointName = Line->String;
			//JointName.Data[JointName.Size - 1] = 0;
			//JointName.Size--;
			Info.JointNames[JointIndex] = JointName;

			Line = Line->Next;
			string_array StrTimes = StringSplitIntoArray(Arena, Line->String,(u8 *)LineDelimeters, 3);

			Line = Line->Next;
			string_array StrTransforms = StringSplitIntoArray(Arena, Line->String,(u8 *)LineDelimeters, 3);

			for(u32 TimeIndex = 0; TimeIndex < Info.TimeCount; ++TimeIndex)
			{
				string Time = StrTimes.Strings[TimeIndex];
				Info.Times[JointIndex][TimeIndex] = F32FromASCII(Time);
			}

			for(u32 MatrixIndex = 0; MatrixIndex < Info.TransformCount; ++MatrixIndex)
			{
				mat4 *M = &Info.Transforms[JointIndex][MatrixIndex];
				f32 *Float = &M->E[0][0];
				for(u32 FloatIndex = 0; FloatIndex < 16; ++FloatIndex)
				{
					string StrFloat = StrTransforms.Strings[16 * MatrixIndex + FloatIndex];
					Float[FloatIndex] = F32FromASCII(StrFloat);
				}
			}
		}
	}
	else
	{
		printf("Error could not read %s\n", FileName);
		perror("");
	}

	return(Info);
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

		loaded_dae LoadedDAE = {};
		model Model = ModelLoad(Arena, "..\\data\\XBot.mesh");
		Model.AnimationsInfo = AnimationInfoLoad(Arena, "..\\data\\XBot_IdleShiftWeight.animation");

		Model.Basis.O = V3(0.0f, -80.0f, -350.0f);
		Model.Basis.X = XAxis();
		Model.Basis.Y = YAxis();
		Model.Basis.Z = ZAxis();

		//
		// NOTE(Justin): Transformations
		//

		mat4 ModelTransform = Mat4Translate(Model.Basis.O);

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

		glViewport(0.0f, 0.0f, Win32GlobalWindowWidth, Win32GlobalWindowHeight);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

		//
		// NOTE(Justin): Opengl shader initialization
		//

		u32 ShaderProgram = GLProgramCreate(BasicVsSrc, BasicFsSrc);

		for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
		{
			s32 ExpectedAttributeCount = 0;

			mesh *Mesh = Model.Meshes + MeshIndex;

			glGenVertexArrays(1, &Model.VA[MeshIndex]);
			glBindVertexArray(Model.VA[MeshIndex]);

			glGenBuffers(1, &Model.PosVB[MeshIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, Model.PosVB[MeshIndex]);
			glBufferData(GL_ARRAY_BUFFER, Mesh->PositionsCount * sizeof(f32), Mesh->Positions, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);
			ExpectedAttributeCount++;

			glGenBuffers(1, &Model.NormVB[MeshIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, Model.NormVB[MeshIndex]);
			glBufferData(GL_ARRAY_BUFFER, Mesh->NormalsCount * sizeof(f32), Mesh->Normals, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			ExpectedAttributeCount++;

			glGenBuffers(1, &Model.JointInfoVB[MeshIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, Model.JointInfoVB[MeshIndex]);
			glBufferData(GL_ARRAY_BUFFER, Mesh->JointInfoCount * sizeof(joint_info), Mesh->JointsInfo, GL_STATIC_DRAW);

			glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(joint_info), 0);
			glVertexAttribIPointer(3, 3, GL_UNSIGNED_INT, sizeof(joint_info), (void *)(1 * sizeof(u32)));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(joint_info), (void *)(4 * sizeof(u32)));

			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
			ExpectedAttributeCount += 3;

			GLIBOInit(&Model.IBO[MeshIndex], Mesh->Indices, Mesh->IndicesCount);
			glBindVertexArray(0);

			s32 AttrCount;
			glGetProgramiv(ShaderProgram, GL_ACTIVE_ATTRIBUTES, &AttrCount);
			Assert(ExpectedAttributeCount == AttrCount);

			char Name[256];
			s32 Size = 0;
			GLsizei NameLength = 0;
			GLenum Type;
			for(s32 i = 0; i < AttrCount; ++i)
			{
				glGetActiveAttrib(ShaderProgram, i, sizeof(Name), &NameLength, &Size, &Type, Name);
				//printf("Attribute:%d\nName:%s\nSize:%d\n\n", i, Name, Size);
			}
		}

		glUseProgram(ShaderProgram);
		UniformMatrixSet(ShaderProgram, "Model", ModelTransform);
		UniformMatrixSet(ShaderProgram, "View", CameraTransform);
		UniformMatrixSet(ShaderProgram, "Projection", PerspectiveTransform);
		UniformV3Set(ShaderProgram, "CameraP", CameraP);

		Model.AnimationsInfo.CurrentTime = 0.0f;
		Model.AnimationsInfo.KeyFrameIndex = 0;
		f32 Angle = 0.0f;
		f32 DtForFrame = TargetSecondsPerFrame;

		Win32GlobalRunning = true;
		while(Win32GlobalRunning)
		{
			MSG Message = {};
			while(PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
			{
				switch(Message.message)
				{
					default:
					{
						TranslateMessage(&Message);
						DispatchMessage(&Message);
					};
				}
			}

			HDC WindowDC = GetDC(Window);

			//
			// NOTE(Justin): HACKED animation.
			//

			animation_info *AnimInfo = &Model.AnimationsInfo;
			AnimInfo->CurrentTime += DtForFrame;
			if(AnimInfo->CurrentTime > 0.03f)
			{
				AnimInfo->KeyFrameIndex += 1;
				if(AnimInfo->KeyFrameIndex >= AnimInfo->TimeCount)
				{
					AnimInfo->KeyFrameIndex = 0;
				}

				AnimInfo->CurrentTime = 0.0f;
			}

			for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
			{
				mesh Mesh = Model.Meshes[MeshIndex];
				if(Mesh.JointInfoCount != 0)
				{
					mat4 Bind = Mesh.BindTransform;

					joint RootJoint = Mesh.Joints[0];
					mat4 RootJointT = RootJoint.Transform;
					mat4 RootInvBind = Mesh.InvBindTransforms[0];

					s32 JointIndex = JointIndexGet(AnimInfo->JointNames, AnimInfo->JointCount, RootJoint.Name);
					if(JointIndex != -1)
					{
						RootJointT = AnimInfo->Transforms[JointIndex][AnimInfo->KeyFrameIndex];
					}

					Mesh.JointTransforms[0] = RootJointT;
					Mesh.ModelSpaceTransforms[0] = RootJointT * RootInvBind * Bind;
					for(u32 Index = 1; Index < Mesh.JointCount; ++Index)
					{
						joint *Joint = Mesh.Joints + Index;
						mat4 JointTransform = Joint->Transform;

						JointIndex = JointIndexGet(AnimInfo->JointNames, AnimInfo->JointCount, Joint->Name);
						if(JointIndex != -1)
						{
							JointTransform = AnimInfo->Transforms[JointIndex][AnimInfo->KeyFrameIndex];
						}

						mat4 ParentTransform = Mesh.JointTransforms[Joint->ParentIndex];
						JointTransform = ParentTransform * JointTransform;
						mat4 InvBind = Mesh.InvBindTransforms[Index];

						Mesh.JointTransforms[Index] = JointTransform;
						Mesh.ModelSpaceTransforms[Index] = JointTransform * InvBind * Bind;
					}
				}
			}

			//
			// NOTE(Justin): Render.
			//

			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(ShaderProgram);

			Angle += DtForFrame;
			UniformV3Set(ShaderProgram, "LightDir", V3(2.0f * cosf(Angle), 0.0f, 2.0f * sinf(Angle)));

			glBindVertexArray(Model.VA[0]);
			UniformMatrixArraySet(ShaderProgram, "Transforms", Model.Meshes[0].ModelSpaceTransforms, Model.Meshes[0].JointCount);
			UniformV4Set(ShaderProgram, "Diffuse", Model.Meshes[0].MaterialSpec.Diffuse);
			UniformV4Set(ShaderProgram, "Specular", Model.Meshes[0].MaterialSpec.Specular);
			UniformF32Set(ShaderProgram, "Shininess", Model.Meshes[0].MaterialSpec.Shininess);
			glDrawElements(GL_TRIANGLES, Model.Meshes[0].IndicesCount, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			glBindVertexArray(Model.VA[1]);
			UniformMatrixArraySet(ShaderProgram, "Transforms", Model.Meshes[1].ModelSpaceTransforms, Model.Meshes[1].JointCount);
			UniformV4Set(ShaderProgram, "Diffuse", Model.Meshes[1].MaterialSpec.Diffuse);
			UniformV4Set(ShaderProgram, "Specular", Model.Meshes[1].MaterialSpec.Specular);
			UniformF32Set(ShaderProgram, "Shininess", Model.Meshes[1].MaterialSpec.Shininess);
			glDrawElements(GL_TRIANGLES, Model.Meshes[1].IndicesCount, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			SwapBuffers(WindowDC);
			ReleaseDC(Window, WindowDC);
		}
	}

    return 0;
}


