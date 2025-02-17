
char *MainVS = R"(
#version 430 core
layout (location = 0) in vec3 P;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Tex;
layout (location = 3) in uint JointCount;
layout (location = 4) in uvec3 JointTransformIndices;
layout (location = 5) in vec3 Weights;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 LightTransform;
uniform bool UsingRig;

#define MAX_JOINT_COUNT 100
uniform mat4 Transforms[MAX_JOINT_COUNT];

out vec3 SurfaceP;
out vec3 SurfaceN;
out vec4 SurfaceLightP;
out vec2 UV;

void main()
{
	vec4 Pos = vec4(0.0);
	vec4 Norm = vec4(0.0);
	if(UsingRig)
	{
		for(uint i = 0; i < 4; ++i)
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
	}
	else
	{
		Pos = vec4(P, 1.0f);
		Norm = vec4(Normal, 0.0f);
	}

	gl_Position = Projection * View * Model * Pos;
	SurfaceP = vec3(Model * Pos);
	SurfaceN = vec3(transpose(inverse(Model)) * Norm);
	SurfaceLightP = LightTransform * vec4(SurfaceP, 1.0);
	UV = Tex;
})";

char *MainFS= R"(
#version 430 core

in vec3 SurfaceP;
in vec3 SurfaceN;
in vec4 SurfaceLightP;
in vec2 UV;

uniform bool OverRideTexture;
uniform bool UsingDiffuse;
uniform bool UsingSpecular;

uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D ShadowMapTexture;

uniform vec3 Ambient;
uniform vec3 LightDir;
uniform vec3 CameraP;
uniform vec4 Diffuse;
uniform vec4 Specular;

uniform float Shininess;

float
SimplePCFShadowMapValue(vec4 SurfaceLightP)
{
	float Result = 0.0;

	vec3 LightProjectedP = SurfaceLightP.xyz / SurfaceLightP.w;
	LightProjectedP = 0.5 * LightProjectedP + 0.5;

	vec2 TexelSize = 1.0f / textureSize(ShadowMapTexture, 0);
	vec2 TexelUV = LightProjectedP.xy;

	float U, V;
	float TestDepth = 0.0;
	float ShadowBias = 0.0001;
	float CurrentDepth = LightProjectedP.z;
	for(V = -1.5; V <= 1.5; V+=1.0)
	{
		for(U = -1.5; U <= 1.5; U+=1.0)
		{
			vec2 SampleUV = TexelUV + vec2(U, V) * TexelSize;
			TestDepth = texture(ShadowMapTexture, SampleUV).r;
			Result += ((CurrentDepth - ShadowBias) > TestDepth ? TestDepth : 1.0);
		}
	}

	Result /= 16.0f;
	return(Result);
}

out vec4 Result;
void main()
{
	vec3 LightColor = vec3(1.0);
	vec3 SurfaceToCamera = CameraP - SurfaceP;

	// Directional
	vec3 SurfaceToLight = -LightDir;

	vec3 Normal = normalize(SurfaceN);
	vec3 SurfaceToCameraNormalized = normalize(SurfaceToCamera);
	vec3 SurfaceToLightNormalized = normalize(SurfaceToLight);
	vec3 ReflectedDirection = reflect(Normal, SurfaceToCameraNormalized);

	float D = max(dot(SurfaceToLightNormalized, Normal), 0.0);
	float S = pow(max(dot(ReflectedDirection, Normal), 0.0), 32);
	float A = 1.0f;

	vec3 Diff = vec3(0.0);
	vec3 Amb = vec3(0.0);
	vec3 Spec = vec3(0.0);

	Amb = Ambient * Diffuse.xyz;
	if(OverRideTexture)
	{
		Diff = D * LightColor * Diffuse.xyz;
		Spec = S * LightColor * Specular.xyz;
	}

	if(UsingDiffuse)
	{
		vec4 DiffuseTexel = texture(DiffuseTexture, UV);
		Amb = Ambient * DiffuseTexel.xyz;
		Diff = D * LightColor * DiffuseTexel.xyz;
		A = DiffuseTexel.a;
	}

	if(UsingSpecular)
	{
		vec4 SpecularTexel = texture(SpecularTexture, UV);
		Spec = S * LightColor * SpecularTexel.xyz;
	}

	float ShadowValue = SimplePCFShadowMapValue(SurfaceLightP);
	Result = vec4(Amb + ShadowValue*(Diff + Spec), A);
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

char *Quad2dVS = R"(
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

char *Quad2dFS = R"(
#version 430 core
in vec2 UV;

uniform sampler2D Texture;

out vec4 Result;
void main()
{
	Result = vec4(texture(Texture, UV).rgb, 1.0);
})";

char *DebugShadowMapVS = R"(
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

char *DebugShadowMapFS= R"(
#version 430 core
in vec2 UV;

uniform sampler2D Texture;

out vec4 Result;
void main()
{
	float Depth = texture(Texture, UV).r;
	Result = vec4(Depth, Depth, Depth, 1.0f);
})";


char *ShadowMapVS = R"(
#version 430 core
layout (location = 0) in vec3 P;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Tex;
layout (location = 3) in uint JointCount;
layout (location = 4) in uvec3 JointTransformIndices;
layout (location = 5) in vec3 Weights;

uniform mat4 Model;
uniform mat4 LightTransform;
uniform bool UsingRig;

#define MAX_JOINT_COUNT 70
uniform mat4 Transforms[MAX_JOINT_COUNT];

void main()
{
	vec4 Pos = vec4(0.0);
	if(UsingRig)
	{
		for(uint i = 0; i < 3; ++i)
		{
			if(i < JointCount)
			{
				uint JointIndex = JointTransformIndices[i];
				float Weight = Weights[i];
				mat4 Xform = Transforms[JointIndex];
				Pos += Weight * Xform * vec4(P, 1.0);
			}
		}
	}
	else
	{
		Pos = vec4(P, 1.0f);
	}

	gl_Position = LightTransform * Model * Pos;
})";

char *ShadowMapFS = R"(
#version 430 core

void main()
{
})";

char *DebugBBoxVS = R"(
#version 430 core
layout (location = 0) in vec3 P;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Tex;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
	gl_Position = Projection * View * Model * vec4(P, 1.0);
})";

char *DebugBBoxFS = R"(
#version 430 core

uniform vec3 Ambient;
uniform vec4 Diffuse;

out vec4 Result;
void main()
{
	Result = vec4(Ambient + Diffuse.xyz, 1.0);
})";

// TODO(Justin): Move all the vertex array's and vertex buffers
// to the open_gl struct and figure out a way to map and asset
// to an allocated vertex array and vertex buffer. The vertex array
// and vertex buffer are part of the graphics backend specification
// and should not be part of the game implementation.

// If each asset has a unique id that the game uses then we can
// use that or turn it into an index
#define OpenGLFunctionDeclare(Name, Type) PFN##Type##PROC Name
struct open_gl
{
	u32 MainShader;
	u32 FontShader;
	u32 Quad2dShader;
	u32 DebugShadowMapShader;
	u32 ShadowMapShader;
	u32 DebugBBoxShader;

	u32 NullTexture;

	u32 Quad2dVA;
	u32 Quad2dVB;

	u32 Quad3dVA;
	u32 Quad3dVB;

	u32 FBO;
	u32 RBO;
	u32 TextureHandle;
	u32 TextureWidth;
	u32 TextureHeight;

	u32 ShadowMapFBO;
	u32 ShadowMapHandle;
	u32 ShadowMapWidth;
	u32 ShadowMapHeight;

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

internal void
Win32LoadAllOpenGLProcedures(open_gl *OpenGL)
{
#define OpenGLFunction(Name, Type) OpenGL->Name = (PFN##Type##PROC)Win32LoadOpenGLProcedure(#Name);
OpenGLFunction(glGenBuffers, GLGENBUFFERS)
OpenGLFunction(glBindBuffer, GLBINDBUFFER)
OpenGLFunction(glGenVertexArrays, GLGENVERTEXARRAYS)
OpenGLFunction(glBindVertexArray, GLBINDVERTEXARRAY)
OpenGLFunction(glCreateProgram, GLCREATEPROGRAM)
OpenGLFunction(glCreateShader, GLCREATESHADER)
OpenGLFunction(glShaderSource, GLSHADERSOURCE)
OpenGLFunction(glCompileShader, GLCOMPILESHADER)
OpenGLFunction(glGetShaderiv, GLGETSHADERIV)
OpenGLFunction(glGetShaderInfoLog, GLGETSHADERINFOLOG)
OpenGLFunction(glGetProgramiv, GLGETPROGRAMIV)
OpenGLFunction(glGetProgramInfoLog, GLGETPROGRAMINFOLOG)
OpenGLFunction(glAttachShader, GLATTACHSHADER)
OpenGLFunction(glLinkProgram, GLLINKPROGRAM)
OpenGLFunction(glValidateProgram, GLVALIDATEPROGRAM)
OpenGLFunction(glDeleteShader, GLDELETESHADER)
OpenGLFunction(glGetUniformLocation, GLGETUNIFORMLOCATION)
OpenGLFunction(glGetAttribLocation, GLGETATTRIBLOCATION)
OpenGLFunction(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY)
OpenGLFunction(glVertexAttribPointer, GLVERTEXATTRIBPOINTER)
OpenGLFunction(glBufferData, GLBUFFERDATA)
OpenGLFunction(glBufferSubData, GLBUFFERSUBDATA)
OpenGLFunction(glBlendFuncSeparate, GLBLENDFUNCSEPARATE)
OpenGLFunction(glUseProgram, GLUSEPROGRAM)
OpenGLFunction(glUniform1i, GLUNIFORM1I)
OpenGLFunction(glUniform1f, GLUNIFORM1F)
OpenGLFunction(glUniform2f, GLUNIFORM2F)
OpenGLFunction(glUniform3f, GLUNIFORM3F)
OpenGLFunction(glUniform4f, GLUNIFORM4F)
OpenGLFunction(glTexSubImage3D, GLTEXSUBIMAGE3D)
OpenGLFunction(glTexImage3D, GLTEXIMAGE3D)
OpenGLFunction(glGenerateMipmap, GLGENERATEMIPMAP)
OpenGLFunction(glVertexAttribIPointer, GLVERTEXATTRIBIPOINTER)
OpenGLFunction(glGetActiveAttrib, GLGETACTIVEATTRIB)
OpenGLFunction(glUniform1ui, GLUNIFORM1UI)
OpenGLFunction(glUniform3fv, GLUNIFORM3FV)
OpenGLFunction(glUniform4fv, GLUNIFORM4FV)
OpenGLFunction(glUniformMatrix4fv, GLUNIFORMMATRIX4FV)
OpenGLFunction(glActiveTexture, GLACTIVETEXTURE)
OpenGLFunction(glGenFramebuffers, GLGENFRAMEBUFFERS)
OpenGLFunction(glBindFramebuffer, GLBINDFRAMEBUFFER)
OpenGLFunction(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D)
OpenGLFunction(glBindFramebuffer, GLBINDFRAMEBUFFER)
OpenGLFunction(glGenRenderbuffers, GLGENRENDERBUFFERS)
OpenGLFunction(glBindRenderbuffer, GLBINDRENDERBUFFER)
OpenGLFunction(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER)
OpenGLFunction(glRenderbufferStorage, GLRENDERBUFFERSTORAGE)
OpenGLFunction(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS)
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
Win32OpenGLInitialize(HDC WindowDC, open_gl *OpenGL)
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
