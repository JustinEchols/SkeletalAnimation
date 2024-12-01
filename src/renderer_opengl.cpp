
#include "render.h"

internal void 
GLDebugCallback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam)
{
	printf("OpenGL Debug Callback: %s\n", Message);
}

internal void
GLIBOInit(u32 *IBO, u32 *Indices, u32 IndicesCount)
{
	OpenGL.glGenBuffers(1, IBO);
	OpenGL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBO);
	OpenGL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesCount * sizeof(u32), Indices, GL_STATIC_DRAW);
}

internal u32
GLCompilerShader(GLenum ShaderType, char *Source)
{
	u32 Shader = OpenGL.glCreateShader(ShaderType);
	OpenGL.glShaderSource(Shader, 1, &Source, 0);
	OpenGL.glCompileShader(Shader);

	b32 IsValid = false;
	char Buffer[1024];
	GLsizei LogLength = 0;
	OpenGL.glGetShaderiv(Shader, GL_COMPILE_STATUS, &IsValid);
	if(!IsValid)
	{
		OpenGL.glGetShaderInfoLog(Shader, 1024, &LogLength, Buffer);
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
	Program = OpenGL.glCreateProgram();
	OpenGL.glAttachShader(Program, VSHandle);
	OpenGL.glAttachShader(Program, FSHandle);
	OpenGL.glLinkProgram(Program);
	OpenGL.glValidateProgram(Program);

	char Buffer[512];
	b32 ProgramIsValid = false;
	OpenGL.glGetProgramiv(Program, GL_LINK_STATUS, &ProgramIsValid);
	if(!ProgramIsValid)
	{
		OpenGL.glGetProgramInfoLog(Program, 512, 0, Buffer);
		OutputDebugStringA("ERROR: Program link failed\n");
	}

	OpenGL.glDeleteShader(VSHandle);
	OpenGL.glDeleteShader(FSHandle);

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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		f32 maxAniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);

		glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA8,
				Texture->Width,
				Texture->Height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				Texture->Memory);


		OpenGL.glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

internal void 
AttributesCheck(u32 ShaderProgram, s32 ExpectedAttributeCount)
{
	s32 AttrCount;
	OpenGL.glGetProgramiv(ShaderProgram, GL_ACTIVE_ATTRIBUTES, &AttrCount);
	Assert(ExpectedAttributeCount == AttrCount);

	char Buff[512];
	char Name[256];
	s32 Size = 0;
	GLsizei Length = 0;
	GLenum Type;
	for(s32 i = 0; i < AttrCount; ++i)
	{
		OpenGL.glGetActiveAttrib(ShaderProgram, i, sizeof(Name), &Length, &Size, &Type, Name);

		sprintf(Buff, "Attribute:%d\nName:%s\nSize:%d\n\n", i, Name, Size);
	}
}

internal void
OpenGLAllocateAnimatedMesh(mesh *Mesh, u32 ShaderProgram)
{
	s32 ExpectedAttributeCount = 0;

	OpenGL.glGenVertexArrays(1, &Mesh->VA);
	OpenGL.glBindVertexArray(Mesh->VA);

	OpenGL.glGenBuffers(1, &Mesh->VB);
	OpenGL.glBindBuffer(GL_ARRAY_BUFFER, Mesh->VB);
	OpenGL.glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex), Mesh->Vertices, GL_STATIC_DRAW);

	OpenGL.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,	sizeof(vertex), 0);
	OpenGL.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,	sizeof(vertex), (void *)OffsetOf(vertex, N));
	OpenGL.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,	sizeof(vertex), (void *)OffsetOf(vertex, UV));
	OpenGL.glVertexAttribIPointer(3, 1,GL_UNSIGNED_INT,		sizeof(vertex), (void *)OffsetOf(vertex, JointInfo));
	OpenGL.glVertexAttribIPointer(4, 3,GL_UNSIGNED_INT,		sizeof(vertex), (void *)(OffsetOf(vertex, JointInfo) + OffsetOf(joint_info, JointIndex)));
	OpenGL.glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE,	sizeof(vertex), (void *)(OffsetOf(vertex, JointInfo) + OffsetOf(joint_info, Weights)));

	OpenGL.glEnableVertexAttribArray(0);
	OpenGL.glEnableVertexAttribArray(1);
	OpenGL.glEnableVertexAttribArray(2);
	OpenGL.glEnableVertexAttribArray(3);
	OpenGL.glEnableVertexAttribArray(4);
	OpenGL.glEnableVertexAttribArray(5);

	// TODO(Justin): Remove hardcoded value
	ExpectedAttributeCount = 6;

	GLIBOInit(&Mesh->IBO, Mesh->Indices, Mesh->IndicesCount);
	OpenGL.glBindVertexArray(0);

	AttributesCheck(ShaderProgram, ExpectedAttributeCount);
}

internal void
OpenGLAllocateAnimatedModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		OpenGLAllocateAnimatedMesh(Mesh, ShaderProgram);
	}
}

internal void
OpenGLAllocateMesh(mesh *Mesh, u32 ShaderProgram)
{
	s32 ExpectedAttributeCount = 0;

	OpenGL.glGenVertexArrays(1, &Mesh->VA);
	OpenGL.glBindVertexArray(Mesh->VA);

	OpenGL.glGenBuffers(1, &Mesh->VB);
	OpenGL.glBindBuffer(GL_ARRAY_BUFFER, Mesh->VB);
	OpenGL.glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex), Mesh->Vertices, GL_STATIC_DRAW);

	OpenGL.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
	OpenGL.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)OffsetOf(vertex, N));
	OpenGL.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)OffsetOf(vertex, UV));

	OpenGL.glEnableVertexAttribArray(0);
	OpenGL.glEnableVertexAttribArray(1);
	OpenGL.glEnableVertexAttribArray(2);

	ExpectedAttributeCount = 3;

	GLIBOInit(&Mesh->IBO, Mesh->Indices, Mesh->IndicesCount);
	OpenGL.glBindVertexArray(0);

	AttributesCheck(ShaderProgram, ExpectedAttributeCount);
}

internal void
OpenGLAllocateModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		OpenGLAllocateMesh(Mesh, ShaderProgram);
	}
}

internal void
OpenGLAllocateQuad(quad *Quad, u32 ShaderProgram)
{
	OpenGL.glGenVertexArrays(1, &Quad->VA);
	OpenGL.glBindVertexArray(Quad->VA);
	OpenGL.glGenBuffers(1, &Quad->VB);
	OpenGL.glBindBuffer(GL_ARRAY_BUFFER, Quad->VB);
	OpenGL.glBufferData(GL_ARRAY_BUFFER, ArrayCount(Quad->Vertices) * sizeof(quad_vertex), Quad->Vertices, GL_STATIC_DRAW);
	OpenGL.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), 0);
	OpenGL.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), (void *)(3 * sizeof(f32)));
	OpenGL.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), (void *)(6 * sizeof(f32)));
	OpenGL.glEnableVertexAttribArray(0);
	OpenGL.glEnableVertexAttribArray(1);
	OpenGL.glEnableVertexAttribArray(2);
	s32 ExpectedAttributeCount = 3;
	AttributesCheck(ShaderProgram, ExpectedAttributeCount);
	OpenGL.glBindVertexArray(0);
}

internal void
OpenGLAllocateQuad2d(u32 *VA, u32 *VB, u32 ShaderProgram)
{
	OpenGL.glGenVertexArrays(1, VA);
	OpenGL.glBindVertexArray(*VA);
	OpenGL.glGenBuffers(1, VB);
	OpenGL.glBindBuffer(GL_ARRAY_BUFFER, *VB);
	OpenGL.glBufferData(GL_ARRAY_BUFFER, 6 * (4 * sizeof(f32)), 0, GL_DYNAMIC_DRAW);
	OpenGL.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
	OpenGL.glEnableVertexAttribArray(0);
	s32 ExpectedAttributeCount = 1;
	AttributesCheck(ShaderProgram, ExpectedAttributeCount);
	OpenGL.glBindVertexArray(0);
}

internal b32 
OpenGLFrameBufferInit(u32 *FBO, u32 *Texture, u32 *RBO, u32 Width, u32 Height)
{
	b32 Result = true;

	OpenGL.glGenFramebuffers(1, FBO);
	OpenGL.glBindFramebuffer(GL_FRAMEBUFFER, *FBO);

	glGenTextures(1, Texture);
	glBindTexture(GL_TEXTURE_2D, *Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	OpenGL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *Texture, 0);

	OpenGL.glGenRenderbuffers(1, RBO);
	OpenGL.glBindRenderbuffer(GL_RENDERBUFFER, *RBO); 
	OpenGL.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);  
	OpenGL.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *RBO);
	OpenGL.glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(OpenGL.glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Result = false;
	}

	return(Result);
}

internal void
UniformBoolSet(u32 ShaderProgram, char *UniformName, b32 B32)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniform1i(UniformLocation, B32);
}

internal void
UniformU32Set(u32 ShaderProgram, char *UniformName, u32 U32)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniform1ui(UniformLocation, U32);
}

internal void
UniformF32Set(u32 ShaderProgram, char *UniformName, f32 F32)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniform1f(UniformLocation, F32);
}

internal void
UniformV3Set(u32 ShaderProgram, char *UniformName, v3 V)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniform3fv(UniformLocation, 1, &V.E[0]);
}
internal void
UniformV4Set(u32 ShaderProgram, char *UniformName, v4 V)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniform4fv(UniformLocation, 1, &V.E[0]);
}

internal void
UniformMatrixArraySet(u32 ShaderProgram, char *UniformName, mat4 *M, u32 Count)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniformMatrix4fv(UniformLocation, Count, GL_TRUE, &M[0].E[0][0]);
}

internal void
UniformMatrixSet(u32 ShaderProgram, char *UniformName, mat4 M)
{
	s32 UniformLocation = OpenGL.glGetUniformLocation(ShaderProgram, UniformName);
	OpenGL.glUniformMatrix4fv(UniformLocation, 1, GL_TRUE, &M.E[0][0]);
}

internal void
OpenGLDrawAnimatedMesh(mesh *Mesh, u32 ShaderProgram)
{
	OpenGL.glUseProgram(ShaderProgram);
	//UniformMatrixSet(ShaderProgram, "Model", Transform);
	OpenGL.glBindVertexArray(Mesh->VA);
	UniformMatrixArraySet(ShaderProgram, "Transforms", Mesh->ModelSpaceTransforms, Mesh->JointCount);
	UniformV4Set(ShaderProgram, "Diffuse", Mesh->MaterialSpec.Diffuse);
	UniformV4Set(ShaderProgram, "Specular", Mesh->MaterialSpec.Specular);
	UniformF32Set(ShaderProgram, "Shininess", Mesh->MaterialSpec.Shininess);
	glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, 0);
	OpenGL.glBindVertexArray(0);
}

internal void
OpenGLDrawAnimatedModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		OpenGLDrawAnimatedMesh(Mesh, ShaderProgram);
	}
}

internal void
OpenGLDrawMesh(mesh *Mesh, u32 ShaderProgram)
{
	OpenGL.glUseProgram(ShaderProgram);
	OpenGL.glActiveTexture(GL_TEXTURE0);
	UniformBoolSet(ShaderProgram, "Texture", 0);
	glBindTexture(GL_TEXTURE_2D, Mesh->Texture->Handle);
	OpenGL.glBindVertexArray(Mesh->VA);
	glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, 0);
	OpenGL.glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
OpenGLDrawModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		OpenGLDrawMesh(Mesh, ShaderProgram);
	}
}

internal void
OpenGLDrawQuad(u32 VA, u32 ShaderProgram, mat4 Transform, u32 TextureHandle)
{
	OpenGL.glUseProgram(ShaderProgram);
	UniformMatrixSet(ShaderProgram, "Model", Transform);
	OpenGL.glActiveTexture(GL_TEXTURE0);
	UniformBoolSet(ShaderProgram, "Texture", 0);
	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	OpenGL.glBindVertexArray(VA);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	OpenGL.glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
OpenGLAllocateGlyph(glyph *Glyph)
{
	glGenTextures(1, &Glyph->TextureHandle);
	glBindTexture(GL_TEXTURE_2D, Glyph->TextureHandle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_R8,
			Glyph->Dim.width,
			Glyph->Dim.height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			Glyph->Memory);

	OpenGL.glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
OpenGLDrawText(char *Text, u32 Shader, font *Font, v2 P, f32 Scale, v3 Color,
		s32 WindowWidth, s32 WindowHeight)
{
	OpenGL.glUseProgram(Shader);
	UniformV3Set(Shader, "Color", Color);
	OpenGL.glBindVertexArray(Font->VA);
	for(char *C = Text; *C; ++C)
	{
		glyph Glyph = Font->Glyphs[*C];

		if(*C != ' ')
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

			OpenGL.glActiveTexture(GL_TEXTURE0);
			UniformBoolSet(Shader, "Texture", 0);
			UniformF32Set(Shader, "WindowWidth", (f32)WindowWidth);
			UniformF32Set(Shader, "WindowHeight", (f32)WindowHeight);
			glBindTexture(GL_TEXTURE_2D, Glyph.TextureHandle);
			OpenGL.glBindBuffer(GL_ARRAY_BUFFER, Font->VB);
			OpenGL.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		P.x += (Glyph.Advance >> 6) * Scale;
	}

	OpenGL.glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
RenderBufferToOutput(render_buffer *RenderBuffer, u32 WindowWidth, u32 WindowHeight)
{
	if(RenderBuffer->OutputTargetIndex == 0)
	{
		OpenGL.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else if(RenderBuffer->OutputTargetIndex == 1)
	{
		OpenGL.glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.ShadowMapFBO);
	}
	else
	{
		OpenGL.glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.FBO);
	}

	glViewport(0, 0, WindowWidth, WindowHeight);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	u32 MainShader	= OpenGL.MainShader;
	u32 BasicShader = OpenGL.BasicShader;
	u32 FontShader	= OpenGL.FontShader;
	u32 ScreenShader = OpenGL.ScreenShader;

	mat4 View = RenderBuffer->View;
	mat4 Perspective = RenderBuffer->Perspective;
	v3 CameraP	= RenderBuffer->CameraP;
	v3 LightDir = V3(1.0f, -1.0f, -0.5f);

	for(u32 BaseOffset = 0; BaseOffset < RenderBuffer->Size; )
	{
		render_buffer_entry_header *Header = (render_buffer_entry_header *)(RenderBuffer->Base + BaseOffset);
		BaseOffset += sizeof(*Header);

		void *Data = (u8 *)Header + sizeof(*Header);
		switch(Header->Type)
		{
			case RenderBuffer_render_entry_clear:
			{
				render_entry_clear *Entry = (render_entry_clear *)Data;
				glClearColor(Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_texture:
			{
				render_entry_texture *Entry = (render_entry_texture *)Data;
				texture *Texture = RenderBuffer->Textures[Entry->Index];
				if(Texture->Handle == 0)
				{
					OpenGLAllocateTexture(Texture);
				}
				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_quad_3d:
			{
				render_entry_quad_3d *Entry = (render_entry_quad_3d *)Data;
				quad *Quad = Entry->Quad;
				if(!Quad->UploadedToGPU)
				{
					OpenGLAllocateQuad(Quad, BasicShader);
				}

				// TODO(Justin): May not want to always be using a texture. Need to differentiate 
				// between those cases. Use an index that is invalid?
				texture *Texture = RenderBuffer->Textures[Entry->TextureIndex];
				Assert(Texture->Handle);
				
				OpenGL.glUseProgram(BasicShader);
				UniformMatrixSet(BasicShader, "View", View);
				UniformMatrixSet(BasicShader, "Projection", Perspective);
				UniformBoolSet(BasicShader, "OverRideTexture", false);
				UniformV3Set(BasicShader, "Ambient", V3(0.1f));
				UniformV3Set(BasicShader, "LightDir", LightDir);
				OpenGLDrawQuad(Quad->VA, BasicShader, Entry->Transform, Texture->Handle);
				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_quad_2d:
			{
				render_entry_quad_2d *Entry = (render_entry_quad_2d *)Data;
				texture *Texture = RenderBuffer->Textures[Entry->TextureIndex];
				Assert(Texture->Handle);

				OpenGL.glUseProgram(ScreenShader);
				OpenGL.glActiveTexture(GL_TEXTURE0);
				UniformBoolSet(ScreenShader, "Texture", 0);
				UniformF32Set(ScreenShader, "WindowWidth", (f32)WindowWidth);
				UniformF32Set(ScreenShader, "WindowHeight", (f32)WindowHeight);
				glBindTexture(GL_TEXTURE_2D, Texture->Handle);
				OpenGL.glBindVertexArray(OpenGL.Quad2dVA);
				OpenGL.glBindBuffer(GL_ARRAY_BUFFER, OpenGL.Quad2dVB);
				OpenGL.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Entry->Vertices), Entry->Vertices);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				
				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_model:
			{
				render_entry_model *Entry = (render_entry_model *)Data;
				model *Model = Entry->Model;
				if(Model->HasSkeleton)
				{
					if(!Model->UploadedToGPU)
					{
						OpenGLAllocateAnimatedModel(Entry->Model, MainShader);
						Model->UploadedToGPU = true;
					}

					// VS
					OpenGL.glUseProgram(MainShader);
					UniformMatrixSet(MainShader, "Model", Entry->Transform);
					UniformMatrixSet(MainShader, "View", View);
					UniformMatrixSet(MainShader, "Projection", Perspective);
					UniformBoolSet(MainShader, "UsingRig", true);

					// FS 
					UniformBoolSet(MainShader, "OverRideTexture", true);
					UniformV3Set(MainShader, "Ambient", V3(0.1f));
					UniformV3Set(MainShader, "CameraP", CameraP);
					UniformV3Set(MainShader, "LightDir", LightDir);

					OpenGLDrawAnimatedModel(Model, MainShader);
				}
				else
				{
					if(!Model->UploadedToGPU)
					{
						OpenGLAllocateModel(Entry->Model, BasicShader);
						Model->UploadedToGPU = true;
					}

					// VS
					OpenGL.glUseProgram(BasicShader);
					UniformMatrixSet(BasicShader, "Model", Entry->Transform);
					UniformMatrixSet(BasicShader, "View", View);
					UniformMatrixSet(BasicShader, "Projection", Perspective);

					// FS
					UniformBoolSet(BasicShader, "OverRideTexture", false);
					UniformV3Set(BasicShader, "Ambient", V3(0.1f));
					UniformV3Set(BasicShader, "LightDir", LightDir);
					UniformV4Set(BasicShader, "Color", V4(1.0f));
					OpenGLDrawModel(Model, BasicShader);
				}

				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_text:
			{
				render_entry_text *Entry = (render_entry_text *)Data;
				if(!Entry->Font->UploadedToGPU)
				{
					OpenGLAllocateQuad2d(&Entry->Font->VA, &Entry->Font->VB, FontShader);
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					for(char CharIndex = ' '; CharIndex < '~'; ++CharIndex)
					{
						glyph *Glyph = Entry->Font->Glyphs + CharIndex;
						OpenGLAllocateGlyph(Glyph);
					}
					Entry->Font->UploadedToGPU = true;
					glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				}

				OpenGLDrawText(Entry->Text, FontShader, Entry->Font, Entry->P, Entry->Scale, Entry->Color, WindowWidth, WindowHeight);
				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_aabb:
			{
				render_entry_aabb *Entry = (render_entry_aabb *)Data;
				model *Model = Entry->Model;
				if(!Model->UploadedToGPU)
				{
					OpenGLAllocateModel(Entry->Model, BasicShader);
					Model->UploadedToGPU = true;
				}

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				OpenGL.glUseProgram(BasicShader);
				UniformMatrixSet(BasicShader, "Model", Entry->Transform);
				UniformMatrixSet(BasicShader, "View", View);
				UniformMatrixSet(BasicShader, "Projection", Perspective);
				UniformBoolSet(BasicShader, "OverRideTexture", true);
				UniformV3Set(BasicShader, "Ambient", V3(0.1f));
				UniformV3Set(BasicShader, "LightDir", LightDir);
				UniformV4Set(BasicShader, "Color", V4(Entry->Color, 1.0f));
				OpenGLDrawModel(Model, BasicShader);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				BaseOffset += sizeof(*Entry);
			} break;
			case RenderBuffer_render_entry_render_to_texture:
			{
				render_entry_render_to_texture *Entry = (render_entry_render_to_texture *)Data;

				OpenGL.glUseProgram(ScreenShader);
				OpenGL.glActiveTexture(GL_TEXTURE0);
				UniformBoolSet(ScreenShader, "Texture", 0);
				UniformF32Set(ScreenShader, "WindowWidth", (f32)WindowWidth);
				UniformF32Set(ScreenShader, "WindowHeight", (f32)WindowHeight);
				glBindTexture(GL_TEXTURE_2D, OpenGL.TextureHandle);
				OpenGL.glBindVertexArray(OpenGL.Quad2dVA);
				OpenGL.glBindBuffer(GL_ARRAY_BUFFER, OpenGL.Quad2dVB);
				OpenGL.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Entry->Vertices), Entry->Vertices);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				
				BaseOffset += sizeof(*Entry);
			} break;
		}
	}
}
