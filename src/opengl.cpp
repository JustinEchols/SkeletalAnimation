
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


internal void 
GLDebugCallback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam)
{
	printf("OpenGL Debug Callback: %s\n", Message);
}

internal void
GLIBOInit(u32 *IBO, u32 *Indices, u32 IndicesCount)
{
	glGenBuffers(1, IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesCount * sizeof(u32), Indices, GL_STATIC_DRAW);
}

internal u32
GLCompilerShader(GLenum ShaderType, char *Source)
{
	u32 Shader = glCreateShader(ShaderType);
	glShaderSource(Shader, 1, &Source, 0);
	glCompileShader(Shader);

	b32 IsValid = false;
	char Buffer[1024];
	GLsizei LogLength = 0;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &IsValid);
	if(!IsValid)
	{
		glGetShaderInfoLog(Shader, 1024, &LogLength, Buffer);
		switch(ShaderType)
		{
			case GL_VERTEX_SHADER:
			{
				OutputDebugStringA("ERROR: Vertex shader compile failed\n");

			} break;
			case GL_FRAGMENT_SHADER:
			{
				OutputDebugStringA("ERROR: Fragment shader compile failed\n");
			} break;
		};

		OutputDebugStringA(Buffer);

	}

	return(Shader);
}

internal u32
GLProgramCreate(char *VS, char *FS)
{
	u32 VSHandle = GLCompilerShader(GL_VERTEX_SHADER, VS);
	u32 FSHandle = GLCompilerShader(GL_FRAGMENT_SHADER, FS);

	u32 Program;
	Program = glCreateProgram();
	glAttachShader(Program, VSHandle);
	glAttachShader(Program, FSHandle);
	glLinkProgram(Program);
	glValidateProgram(Program);

	char Buffer[512];
	b32 ProgramIsValid = false;
	glGetProgramiv(Program, GL_LINK_STATUS, &ProgramIsValid);
	if(!ProgramIsValid)
	{
		glGetProgramInfoLog(Program, 512, 0, Buffer);
		OutputDebugStringA("ERROR: Program link failed\n");
	}

	glDeleteShader(VSHandle);
	glDeleteShader(FSHandle);

	u32 Result = Program;

	return(Result);
}

internal void
OpenGLAllocateTexture(texture *Texture)
{
	if(Texture->Memory)
	{
		glGenTextures(1, &Texture->Handle);
		glBindTexture(GL_TEXTURE_2D, Texture->Handle);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		f32 maxAniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);

		glTexImage2D(GL_TEXTURE_2D,
				0,
				Texture->StoredFormat,
				Texture->Width,
				Texture->Height,
				0,
				Texture->SrcFormat,
				GL_UNSIGNED_BYTE,
				Texture->Memory);


		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

internal void 
AttributesCheck(u32 ShaderProgram, s32 ExpectedAttributeCount)
{
	s32 AttrCount;
	glGetProgramiv(ShaderProgram, GL_ACTIVE_ATTRIBUTES, &AttrCount);
	Assert(ExpectedAttributeCount == AttrCount);

	char Buff[512];
	char Name[256];
	s32 Size = 0;
	GLsizei Length = 0;
	GLenum Type;
	for(s32 i = 0; i < AttrCount; ++i)
	{
		glGetActiveAttrib(ShaderProgram, i, sizeof(Name), &Length, &Size, &Type, Name);

		sprintf(Buff, "Attribute:%d\nName:%s\nSize:%d\n\n", i, Name, Size);
	}
}

internal void
OpenGLAllocateAnimatedModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		s32 ExpectedAttributeCount = 0;

		mesh *Mesh = Model->Meshes + MeshIndex;

		glGenVertexArrays(1, &Model->VA[MeshIndex]);
		glBindVertexArray(Model->VA[MeshIndex]);

		glGenBuffers(1, &Model->VB[MeshIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, Model->VB[MeshIndex]);
		glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex), Mesh->Vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(3 * sizeof(f32)));
		glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(vertex), (void *)(8 * sizeof(f32)));
		glVertexAttribIPointer(3, 3, GL_UNSIGNED_INT, sizeof(vertex), (void *)(9 * sizeof(u32)));
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(12 * sizeof(u32)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		ExpectedAttributeCount = 5;

		GLIBOInit(&Model->IBO[MeshIndex], Mesh->Indices, Mesh->IndicesCount);
		glBindVertexArray(0);

		AttributesCheck(ShaderProgram, ExpectedAttributeCount);
	}
}

internal void
OpenGLAllocateModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		s32 ExpectedAttributeCount = 0;

		mesh *Mesh = Model->Meshes + MeshIndex;

		glGenVertexArrays(1, &Model->VA[MeshIndex]);
		glBindVertexArray(Model->VA[MeshIndex]);

		glGenBuffers(1, &Model->VB[MeshIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, Model->VB[MeshIndex]);
		glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex), Mesh->Vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(3 * sizeof(f32)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(6 * sizeof(f32)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		ExpectedAttributeCount = 3;

		GLIBOInit(&Model->IBO[MeshIndex], Mesh->Indices, Mesh->IndicesCount);
		glBindVertexArray(0);

		AttributesCheck(ShaderProgram, ExpectedAttributeCount);
	}
}

internal void
OpenGLAllocateQuad(quad *Quad, u32 ShaderProgram)
{
	glGenVertexArrays(1, &Quad->VA);
	glBindVertexArray(Quad->VA);
	glGenBuffers(1, &Quad->VB);
	glBindBuffer(GL_ARRAY_BUFFER, Quad->VB);
	glBufferData(GL_ARRAY_BUFFER, ArrayCount(Quad->Vertices) * sizeof(quad_vertex), Quad->Vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), (void *)(3 * sizeof(f32)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), (void *)(6 * sizeof(f32)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	s32 ExpectedAttributeCount = 3;
	AttributesCheck(ShaderProgram, ExpectedAttributeCount);
	glBindVertexArray(0);
}

internal void
OpenGLAllocateQuad2d(u32 *VA, u32 *VB, u32 ShaderProgram)
{
	glGenVertexArrays(1, VA);
	glBindVertexArray(*VA);
	glGenBuffers(1, VB);
	glBindBuffer(GL_ARRAY_BUFFER, *VB);
	glBufferData(GL_ARRAY_BUFFER, 6 * (4 * sizeof(f32)), 0, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
	glEnableVertexAttribArray(0);
	s32 ExpectedAttributeCount = 1;
	AttributesCheck(ShaderProgram, ExpectedAttributeCount);
	glBindVertexArray(0);
}

internal b32 
OpenGLFrameBufferInit(u32 *FBO, u32 *Texture, u32 *RBO, u32 Width, u32 Height)
{
	b32 Result = true;

	glGenFramebuffers(1, FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, *FBO);

	glGenTextures(1, Texture);
	glBindTexture(GL_TEXTURE_2D, *Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *Texture, 0);

	glGenRenderbuffers(1, RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, *RBO); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);  
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *RBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Result = false;
	}

	return(Result);

}

internal void
UniformBoolSet(u32 ShaderProgram, char *UniformName, b32 B32)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniform1i(UniformLocation, B32);
}

internal void
UniformU32Set(u32 ShaderProgram, char *UniformName, u32 U32)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniform1ui(UniformLocation, U32);
}

internal void
UniformF32Set(u32 ShaderProgram, char *UniformName, f32 F32)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniform1f(UniformLocation, F32);
}

internal void
UniformV3Set(u32 ShaderProgram, char *UniformName, v3 V)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniform3fv(UniformLocation, 1, &V.E[0]);
}
internal void
UniformV4Set(u32 ShaderProgram, char *UniformName, v4 V)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniform4fv(UniformLocation, 1, &V.E[0]);
}

internal void
UniformMatrixArraySet(u32 ShaderProgram, char *UniformName, mat4 *M, u32 Count)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniformMatrix4fv(UniformLocation, Count, GL_TRUE, &M[0].E[0][0]);
}

internal void
UniformMatrixSet(u32 ShaderProgram, char *UniformName, mat4 M)
{
	s32 UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
	glUniformMatrix4fv(UniformLocation, 1, GL_TRUE, &M.E[0][0]);
}

internal void
OpenGLDrawAnimatedModel(model *Model, u32 ShaderProgram, mat4 Transform)
{
	glUseProgram(ShaderProgram);

	UniformMatrixSet(ShaderProgram, "Model", Transform);
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		glBindVertexArray(Model->VA[MeshIndex]);
		UniformMatrixArraySet(ShaderProgram, "Transforms", Mesh->ModelSpaceTransforms, Mesh->JointCount);
		UniformV4Set(ShaderProgram, "Diffuse", Mesh->MaterialSpec.Diffuse);
		UniformV4Set(ShaderProgram, "Specular", Mesh->MaterialSpec.Specular);
		UniformF32Set(ShaderProgram, "Shininess", Mesh->MaterialSpec.Shininess);
		glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

internal void
OpenGLDrawModel(model *Model, u32 ShaderProgram, mat4 Transform)
{
	glUseProgram(ShaderProgram);
	UniformMatrixSet(ShaderProgram, "Model", Transform);
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		glActiveTexture(GL_TEXTURE0);
		UniformBoolSet(ShaderProgram, "Texture", 0);
		glBindTexture(GL_TEXTURE_2D, Mesh->TextureHandle);
		glBindVertexArray(Model->VA[MeshIndex]);
		glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

internal void
OpenGLDrawQuad(u32 VA, u32 ShaderProgram, mat4 Transform, u32 TextureHandle)
{
	glUseProgram(ShaderProgram);
	UniformMatrixSet(ShaderProgram, "Model", Transform);
	glActiveTexture(GL_TEXTURE0);
	UniformBoolSet(ShaderProgram, "Texture", 0);
	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	glBindVertexArray(VA);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
OpenGLDrawText(char *Text, u32 Shader, font *Font, v2 P, f32 Scale, v3 Color,
		s32 WindowWidth, s32 WindowHeight)
{
	glUseProgram(Shader);
	UniformV3Set(Shader, "Color", Color);
	glBindVertexArray(Font->VA);
	for(char *C = Text; *C; ++C)
	{
		glyph Glyph = Font->Glyphs[*C];

		if(!IsSpace(*C))
		{
			f32 X = P.x + Glyph.Bearing.x * Scale;
			f32 Y = P.y - (Glyph.Dim.y - Glyph.Bearing.y) * Scale;

			f32 Width = Glyph.Dim.x * Scale;
			f32 Height = Glyph.Dim.y * Scale;

			f32 Vertices[6][4] =
			{
				{X,			Y + Height, 0.0f, 0.0f},
				{X,			Y,			0.0f, 1.0f},
				{X + Width, Y,			1.0f, 1.0f},

				{X,			Y + Height, 0.0f, 0.0f},
				{X + Width,	Y,			1.0f, 1.0f},
				{X + Width, Y + Height,	1.0f, 0.0f},
			};

			glActiveTexture(GL_TEXTURE0);
			UniformBoolSet(Shader, "Texture", 0);
			UniformF32Set(Shader, "WindowWidth", (f32)WindowWidth);
			UniformF32Set(Shader, "WindowHeight", (f32)WindowHeight);
			glBindTexture(GL_TEXTURE_2D, Glyph.TextureHandle);
			glBindBuffer(GL_ARRAY_BUFFER, Font->VB);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		P.x += (Glyph.Advance >> 6) * Scale;

	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
