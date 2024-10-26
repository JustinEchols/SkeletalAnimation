
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
#undef OpenGLProc

internal void
Win32LoadAllOpenGLProcedures(void)
{
#define OpenGLProc(Name, Type) Name = (PFN##Type##PROC)Win32LoadOpenGLProcedure(#Name);
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
		Win32LoadAllOpenGLProcedures();
	}

	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(1);
	}

	return(OpenGLRC);
}
