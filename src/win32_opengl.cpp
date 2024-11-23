
char *MainVS = R"(
#version 430 core
layout (location = 0) in vec3 P;
layout (location = 1) in vec3 Normal;
layout (location = 2) in uint JointCount;
layout (location = 3) in uvec3 JointTransformIndices;
layout (location = 4) in vec3 Weights;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

#define MAX_JOINT_COUNT 70
uniform mat4 Transforms[MAX_JOINT_COUNT];

out vec3 SurfaceP;
out vec3 SurfaceN;

void main()
{
	vec4 Pos = vec4(0.0);
	vec4 Norm = vec4(0.0);
	for(uint i = 0; i < 3; ++i)
	{
		if(i < JointCount)
		{
			uint JointIndex = JointTransformIndices[i];
			float Weight = Weights[i];
			mat4 Xform = Transforms[JointIndex];
			Pos += Weight * Xform * vec4(P, 1.0);
			Norm += Weight * Xform * vec4(Normal, 0.0);
		}
	}

	gl_Position = Projection * View * Model * Pos;
	SurfaceP = vec3(Model * Pos);
	SurfaceN = vec3(transpose(inverse(Model)) * Norm);
})";

char *MainFS= R"(
#version 430 core

in vec3 SurfaceP;
in vec3 SurfaceN;

uniform vec3 Ambient;
uniform vec4 Diffuse;
uniform vec4 Specular;

uniform float Shininess;

uniform vec3 LightDir;
uniform vec3 CameraP;

out vec4 Result;
void main()
{
	vec3 LightColor = vec3(1.0);
	vec3 SurfaceToCamera = CameraP - SurfaceP;

	vec3 Normal = normalize(SurfaceN);
	vec3 SurfaceToCameraNormalized = normalize(SurfaceToCamera);
	vec3 ReflectedDirection = reflect(Normal, SurfaceToCameraNormalized);

	float D = max(dot(-LightDir, Normal), 0.0);
	float S = pow(max(dot(ReflectedDirection, Normal), 0.0), Shininess);

	vec3 Diff = D * LightColor * Diffuse.xyz;
	vec3 Spec = S * LightColor * Specular.xyz;

	Result = vec4(Ambient + Diff + Spec, 1.0);
})";

char *BasicVS = R"(
#version 430 core
layout (location = 0) in vec3 P;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Tex;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 SurfaceN;
out vec2 UV;

void main()
{
	gl_Position = Projection * View * Model * vec4(P, 1.0);
	SurfaceN = transpose(inverse(mat3(Model))) * Normal;
	UV = Tex;
})";

char *BasicFS = R"(
#version 430 core

in vec3 SurfaceN;
in vec2 UV;

uniform bool OverRideTexture;
uniform sampler2D Texture;
uniform vec3 Ambient;
uniform vec3 LightDir;
uniform vec4 Color;

out vec4 Result;
void main()
{
	vec3 N = normalize(SurfaceN);
	float D = max(dot(-LightDir, N), 0.0);

	float A = texture(Texture, UV).a;
	vec3 Diffuse = vec3(0.0);
	if(OverRideTexture)
	{
		Diffuse = D * Color.xyz;
	}
	else
	{
		Diffuse = D *  texture(Texture, UV).rgb;
	}

	Result = vec4(Ambient + Diffuse, A);
})";

char *FontVS = R"(
#version 430 core
layout (location = 0) in vec4 VertexXYUV;

out vec2 UV;
uniform float WindowWidth;
uniform float WindowHeight;
void main()
{
	float X = (2.0 * VertexXYUV.x) / WindowWidth - 1.0;
	float Y = (2.0 * VertexXYUV.y) / WindowHeight - 1.0;
	gl_Position = vec4(X, Y, 0.0, 1.0);
	UV = VertexXYUV.zw;
})";

char *FontFS= R"(
#version 430 core
in vec2 UV;

uniform sampler2D Texture;
uniform vec3 Color;

out vec4 Result;
void main()
{
	float A = texture(Texture, UV).r;
	Result = vec4(Color, A);
})";

char *ScreenVS = R"(
#version 430 core
layout (location = 0) in vec4 VertexXYUV;

out vec2 UV;
uniform float WindowWidth;
uniform float WindowHeight;
void main()
{
	float X = ((2.0 * VertexXYUV.x) / WindowWidth) - 1.0;
	float Y = ((2.0 * VertexXYUV.y) / WindowHeight) - 1.0;
	gl_Position = vec4(X, Y, 0.0, 1.0);
	UV = VertexXYUV.zw;
})";

char *ScreenFS= R"(
#version 430 core
in vec2 UV;

uniform sampler2D Texture;

out vec4 Result;
void main()
{
	Result = vec4(texture(Texture, UV).rgb, 1.0);
})";

#define OpenGLFunctionDeclare(Name, Type) PFN##Type##PROC Name
struct open_gl
{
	u32 MainShader;
	u32 BasicShader;
	u32 FontShader;
	u32 ScreenShader;
	u32 NullTexture;

	OpenGLFunctionDeclare(glGenBuffers, GLGENBUFFERS);
	OpenGLFunctionDeclare(glBindBuffer, GLBINDBUFFER);
	OpenGLFunctionDeclare(glGenVertexArrays, GLGENVERTEXARRAYS);
	OpenGLFunctionDeclare(glBindVertexArray, GLBINDVERTEXARRAY);
	OpenGLFunctionDeclare(glCreateProgram, GLCREATEPROGRAM);
	OpenGLFunctionDeclare(glCreateShader, GLCREATESHADER);
	OpenGLFunctionDeclare(glShaderSource, GLSHADERSOURCE);
	OpenGLFunctionDeclare(glCompileShader, GLCOMPILESHADER);
	OpenGLFunctionDeclare(glGetShaderiv, GLGETSHADERIV);
	OpenGLFunctionDeclare(glGetShaderInfoLog, GLGETSHADERINFOLOG);
	OpenGLFunctionDeclare(glGetProgramiv, GLGETPROGRAMIV);
	OpenGLFunctionDeclare(glGetProgramInfoLog, GLGETPROGRAMINFOLOG);
	OpenGLFunctionDeclare(glAttachShader, GLATTACHSHADER);
	OpenGLFunctionDeclare(glLinkProgram, GLLINKPROGRAM);
	OpenGLFunctionDeclare(glValidateProgram, GLVALIDATEPROGRAM);
	OpenGLFunctionDeclare(glDeleteShader, GLDELETESHADER);
	OpenGLFunctionDeclare(glGetUniformLocation, GLGETUNIFORMLOCATION);
	OpenGLFunctionDeclare(glGetAttribLocation, GLGETATTRIBLOCATION);
	OpenGLFunctionDeclare(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY);
	OpenGLFunctionDeclare(glVertexAttribPointer, GLVERTEXATTRIBPOINTER);
	OpenGLFunctionDeclare(glBufferData, GLBUFFERDATA);
	OpenGLFunctionDeclare(glBufferSubData, GLBUFFERSUBDATA);
	OpenGLFunctionDeclare(glBlendFuncSeparate, GLBLENDFUNCSEPARATE);
	OpenGLFunctionDeclare(glUseProgram, GLUSEPROGRAM);
	OpenGLFunctionDeclare(glUniform1i, GLUNIFORM1I);
	OpenGLFunctionDeclare(glUniform1f, GLUNIFORM1F);
	OpenGLFunctionDeclare(glUniform2f, GLUNIFORM2F);
	OpenGLFunctionDeclare(glUniform3f, GLUNIFORM3F);
	OpenGLFunctionDeclare(glUniform4f, GLUNIFORM4F);
	OpenGLFunctionDeclare(glTexSubImage3D, GLTEXSUBIMAGE3D);
	OpenGLFunctionDeclare(glTexImage3D, GLTEXIMAGE3D);
	OpenGLFunctionDeclare(glGenerateMipmap, GLGENERATEMIPMAP);
	OpenGLFunctionDeclare(glVertexAttribIPointer, GLVERTEXATTRIBIPOINTER);
	OpenGLFunctionDeclare(glGetActiveAttrib, GLGETACTIVEATTRIB);
	OpenGLFunctionDeclare(glUniform1ui, GLUNIFORM1UI);
	OpenGLFunctionDeclare(glUniform3fv, GLUNIFORM3FV);
	OpenGLFunctionDeclare(glUniform4fv, GLUNIFORM4FV);
	OpenGLFunctionDeclare(glUniformMatrix4fv, GLUNIFORMMATRIX4FV);
	OpenGLFunctionDeclare(glActiveTexture, GLACTIVETEXTURE);
	OpenGLFunctionDeclare(glGenFramebuffers, GLGENFRAMEBUFFERS);
	OpenGLFunctionDeclare(glBindFramebuffer, GLBINDFRAMEBUFFER);
	OpenGLFunctionDeclare(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D);
	OpenGLFunctionDeclare(glGenRenderbuffers, GLGENRENDERBUFFERS);
	OpenGLFunctionDeclare(glBindRenderbuffer, GLBINDRENDERBUFFER);
	OpenGLFunctionDeclare(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER);
	OpenGLFunctionDeclare(glRenderbufferStorage, GLRENDERBUFFERSTORAGE);
	OpenGLFunctionDeclare(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS);
};
#undef OpenGLFunctionDeclare

internal void *
Win32LoadOpenGLProcedure(char *Name)
{
	void *Proc = (void *)wglGetProcAddress(Name);
	if(!Proc || Proc == (void *)0x1 || Proc == (void *)0x2 || Proc == (void *)0x3 || Proc == (void *)-1)
	{
		return(0);
	}
	else
	{
		return(Proc);
	}
}

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT;

internal void
Win32LoadWGLProcedures(HINSTANCE Instance)
{
	wglChoosePixelFormatARB		= (PFNWGLCHOOSEPIXELFORMATARBPROC)		Win32LoadOpenGLProcedure("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB	= (PFNWGLCREATECONTEXTATTRIBSARBPROC)	Win32LoadOpenGLProcedure("wglCreateContextAttribsARB");
	wglMakeContextCurrentARB	= (PFNWGLMAKECONTEXTCURRENTARBPROC)		Win32LoadOpenGLProcedure("wglMakeContextCurrentARB");
	wglSwapIntervalEXT			= (PFNWGLSWAPINTERVALEXTPROC)			Win32LoadOpenGLProcedure("wglSwapIntervalEXT");
	wglGetExtensionsStringEXT	= (PFNWGLGETEXTENSIONSSTRINGEXTPROC)	Win32LoadOpenGLProcedure("wglGetExtensionsStringEXT");

}

#define OpenGLProc(Name, Type) PFN##Type##PROC Name = 0;
OpenGLProc(glGenBuffers, GLGENBUFFERS)
OpenGLProc(glBindBuffer, GLBINDBUFFER)
OpenGLProc(glGenVertexArrays, GLGENVERTEXARRAYS)
OpenGLProc(glBindVertexArray, GLBINDVERTEXARRAY)
OpenGLProc(glCreateProgram, GLCREATEPROGRAM)
OpenGLProc(glCreateShader, GLCREATESHADER)
OpenGLProc(glShaderSource, GLSHADERSOURCE)
OpenGLProc(glCompileShader, GLCOMPILESHADER)
OpenGLProc(glGetShaderiv, GLGETSHADERIV)
OpenGLProc(glGetShaderInfoLog, GLGETSHADERINFOLOG)
OpenGLProc(glGetProgramiv, GLGETPROGRAMIV)
OpenGLProc(glGetProgramInfoLog, GLGETPROGRAMINFOLOG)
OpenGLProc(glAttachShader, GLATTACHSHADER)
OpenGLProc(glLinkProgram, GLLINKPROGRAM)
OpenGLProc(glValidateProgram, GLVALIDATEPROGRAM)
OpenGLProc(glDeleteShader, GLDELETESHADER)
OpenGLProc(glGetUniformLocation, GLGETUNIFORMLOCATION)
OpenGLProc(glGetAttribLocation, GLGETATTRIBLOCATION)
OpenGLProc(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY)
OpenGLProc(glVertexAttribPointer, GLVERTEXATTRIBPOINTER)
OpenGLProc(glBufferData, GLBUFFERDATA)
OpenGLProc(glBufferSubData, GLBUFFERSUBDATA)
OpenGLProc(glBlendFuncSeparate, GLBLENDFUNCSEPARATE)
OpenGLProc(glUseProgram, GLUSEPROGRAM)
OpenGLProc(glUniform1i, GLUNIFORM1I)
OpenGLProc(glUniform1f, GLUNIFORM1F)
OpenGLProc(glUniform2f, GLUNIFORM2F)
OpenGLProc(glUniform3f, GLUNIFORM3F)
OpenGLProc(glUniform4f, GLUNIFORM4F)
OpenGLProc(glTexSubImage3D, GLTEXSUBIMAGE3D)
OpenGLProc(glTexImage3D, GLTEXIMAGE3D)
OpenGLProc(glGenerateMipmap, GLGENERATEMIPMAP)
OpenGLProc(glVertexAttribIPointer, GLVERTEXATTRIBIPOINTER)
OpenGLProc(glGetActiveAttrib, GLGETACTIVEATTRIB)
OpenGLProc(glUniform1ui, GLUNIFORM1UI)
OpenGLProc(glUniform3fv, GLUNIFORM3FV)
OpenGLProc(glUniform4fv, GLUNIFORM4FV)
OpenGLProc(glUniformMatrix4fv, GLUNIFORMMATRIX4FV)
OpenGLProc(glActiveTexture, GLACTIVETEXTURE)
OpenGLProc(glGenFramebuffers, GLGENFRAMEBUFFERS)
OpenGLProc(glBindFramebuffer, GLBINDFRAMEBUFFER)
OpenGLProc(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D)
OpenGLProc(glGenRenderbuffers, GLGENRENDERBUFFERS)
OpenGLProc(glBindRenderbuffer, GLBINDRENDERBUFFER)
OpenGLProc(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER)
OpenGLProc(glRenderbufferStorage, GLRENDERBUFFERSTORAGE)
OpenGLProc(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS)
#undef OpenGLProc

internal void
Win32LoadAllOpenGLProcedures(open_gl *OpenGL)
{
#define OpenGLProc(Name, Type) OpenGL->Name = (PFN##Type##PROC)Win32LoadOpenGLProcedure(#Name);
OpenGLProc(glGenBuffers, GLGENBUFFERS)
OpenGLProc(glBindBuffer, GLBINDBUFFER)
OpenGLProc(glGenVertexArrays, GLGENVERTEXARRAYS)
OpenGLProc(glBindVertexArray, GLBINDVERTEXARRAY)
OpenGLProc(glCreateProgram, GLCREATEPROGRAM)
OpenGLProc(glCreateShader, GLCREATESHADER)
OpenGLProc(glShaderSource, GLSHADERSOURCE)
OpenGLProc(glCompileShader, GLCOMPILESHADER)
OpenGLProc(glGetShaderiv, GLGETSHADERIV)
OpenGLProc(glGetShaderInfoLog, GLGETSHADERINFOLOG)
OpenGLProc(glGetProgramiv, GLGETPROGRAMIV)
OpenGLProc(glGetProgramInfoLog, GLGETPROGRAMINFOLOG)
OpenGLProc(glAttachShader, GLATTACHSHADER)
OpenGLProc(glLinkProgram, GLLINKPROGRAM)
OpenGLProc(glValidateProgram, GLVALIDATEPROGRAM)
OpenGLProc(glDeleteShader, GLDELETESHADER)
OpenGLProc(glGetUniformLocation, GLGETUNIFORMLOCATION)
OpenGLProc(glGetAttribLocation, GLGETATTRIBLOCATION)
OpenGLProc(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY)
OpenGLProc(glVertexAttribPointer, GLVERTEXATTRIBPOINTER)
OpenGLProc(glBufferData, GLBUFFERDATA)
OpenGLProc(glBufferSubData, GLBUFFERSUBDATA)
OpenGLProc(glBlendFuncSeparate, GLBLENDFUNCSEPARATE)
OpenGLProc(glUseProgram, GLUSEPROGRAM)
OpenGLProc(glUniform1i, GLUNIFORM1I)
OpenGLProc(glUniform1f, GLUNIFORM1F)
OpenGLProc(glUniform2f, GLUNIFORM2F)
OpenGLProc(glUniform3f, GLUNIFORM3F)
OpenGLProc(glUniform4f, GLUNIFORM4F)
OpenGLProc(glTexSubImage3D, GLTEXSUBIMAGE3D)
OpenGLProc(glTexImage3D, GLTEXIMAGE3D)
OpenGLProc(glGenerateMipmap, GLGENERATEMIPMAP)
OpenGLProc(glVertexAttribIPointer, GLVERTEXATTRIBIPOINTER)
OpenGLProc(glGetActiveAttrib, GLGETACTIVEATTRIB)
OpenGLProc(glUniform1ui, GLUNIFORM1UI)
OpenGLProc(glUniform3fv, GLUNIFORM3FV)
OpenGLProc(glUniform4fv, GLUNIFORM4FV)
OpenGLProc(glUniformMatrix4fv, GLUNIFORMMATRIX4FV)
OpenGLProc(glActiveTexture, GLACTIVETEXTURE)
OpenGLProc(glGenFramebuffers, GLGENFRAMEBUFFERS)
OpenGLProc(glBindFramebuffer, GLBINDFRAMEBUFFER)
OpenGLProc(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D)
OpenGLProc(glBindFramebuffer, GLBINDFRAMEBUFFER)
OpenGLProc(glGenRenderbuffers, GLGENRENDERBUFFERS)
OpenGLProc(glBindRenderbuffer, GLBINDRENDERBUFFER)
OpenGLProc(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER)
OpenGLProc(glRenderbufferStorage, GLRENDERBUFFERSTORAGE)
OpenGLProc(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS)
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
				"",
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
			Win32LoadWGLProcedures(WindowClass.hInstance);
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
Win32OpenGLInit(HDC WindowDC, open_gl *OpenGL)
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
		Win32LoadAllOpenGLProcedures(OpenGL);
	}

	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(1);
	}

	// TODO Call out to start opengl initialization. E.g. shaders...

	return(OpenGLRC);
}
