
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB	0x0002
#define WGL_CONTEXT_DEBUG_BIT_ARB				0x0001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_STATIC_DRAW              0x88E4
#define GL_ARRAY_BUFFER             0x8892
#define GL_ELEMENT_ARRAY_BUFFER     0x8893
#define GL_ACTIVE_ATTRIBUTES        0x8B89
#define GL_FRAGMENT_SHADER          0x8B30
#define GL_VERTEX_SHADER            0x8B31
#define GL_COMPILE_STATUS           0x8B81
#define GL_LINK_STATUS              0x8B82

#define GL_MULTISAMPLE              0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE      0x809F
#define GL_TRUE                     1
#define GL_FALSE					0
#define GL_COLOR_BUFFER_BIT			0x00004000
#define GL_DEPTH_BUFFER_BIT			0x00000100
#define GL_DEPTH_TEST               0x0B71
#define GL_LESS                     0x0201
#define GL_CCW                      0x0901
#define GL_CULL_FACE                0x0B44
#define GL_BACK                     0x0405

struct opengl_info
{
	char *Vendor;
	char *Renderer;
	char *Version;
	char *ShadingLanguageVersion;
	char *Extensions;
};

char *BasicVsSrc = R"(
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

out vec3 N;

void main()
{
	vec4 Pos = vec4(0.0);
	for(uint i = 0; i < 3; ++i)
	{
		if(i < JointCount)
		{
			uint JIndex = JointTransformIndices[i];
			float W = Weights[i];
			mat4 T = Transforms[JIndex];
			Pos += W * T * vec4(P, 1.0);
		}
	}

	gl_Position = Projection * View * Model * Pos;
	N = vec3(transpose(inverse(Model)) * vec4(Normal, 1.0));
})";

char *BasicFsSrc = R"(
#version 430 core

in vec3 N;

uniform vec4 Diffuse;
uniform vec4 Specular;
uniform float Shininess;

uniform vec3 LightDir;
uniform vec3 CameraP;

out vec4 Result;
void main()
{
	vec3 LightColor = vec3(1.0);
	vec3 Normal = normalize(N);
	bool FacingTowards = dot(Normal, LightDir) > 0.0;

	vec3 Diff = vec3(0.0);
	vec3 Spec = vec3(0.0);
	if(FacingTowards)
	{
		float D = max(dot(Normal, LightDir), 0.0);
		Diff = LightColor * D * Diffuse.xyz;

		vec3 ViewDir = normalize(CameraP);
		vec3 R = reflect(LightDir, Normal);
		float S = pow(max(dot(ViewDir, R), 0.0), Shininess);
		Spec = LightColor * S * Specular.xyz;
	}

	Result = vec4(Diff + Spec, 1.0);
})";


#if 0
internal void GLAPIENTRY
GLDebugCallback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length,
		const GLchar *Message, const void *UserParam)
{
	printf("OpenGL Debug Callback: %s\n", Message);
}
#endif

internal void
GLIBOInit(u32 *IBO, u32 *Indices, u32 IndicesCount)
{
	glGenBuffers(1, IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesCount * sizeof(u32), Indices, GL_STATIC_DRAW);
}

internal u32
GLProgramCreate(char *VS, char *FS)
{
	u32 VSHandle;
	u32 FSHandle;

	VSHandle = glCreateShader(GL_VERTEX_SHADER);
	FSHandle = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(VSHandle, 1, &VS, 0);
	glShaderSource(FSHandle, 1, &FS, 0);

	b32 VSIsValid = false;
	b32 FSIsValid = false;
	char Buffer[512];

	glCompileShader(VSHandle);
	glGetShaderiv(VSHandle, GL_COMPILE_STATUS, &VSIsValid);
	if(!VSIsValid)
	{
		glGetShaderInfoLog(VSHandle, 512, 0, Buffer);
		//printf("ERROR: Vertex Shader Compile Failed\n %s", Buffer);
	}

	glCompileShader(FSHandle);
	glGetShaderiv(FSHandle, GL_COMPILE_STATUS, &FSIsValid);
	if(!FSIsValid)
	{
		glGetShaderInfoLog(FSHandle, 512, 0, Buffer);
		//printf("ERROR: Fragment Shader Compile Failed\n %s", Buffer);
	}

	u32 Program;
	Program = glCreateProgram();
	glAttachShader(Program, VSHandle);
	glAttachShader(Program, FSHandle);
	glLinkProgram(Program);
	glValidateProgram(Program);

	b32 ProgramIsValid = false;
	glGetProgramiv(Program, GL_LINK_STATUS, &ProgramIsValid);
	if(!ProgramIsValid)
	{
		glGetProgramInfoLog(Program, 512, 0, Buffer);
		//printf("ERROR: Program link failed\n %s", Buffer);
	}

	glDeleteShader(VSHandle);
	glDeleteShader(FSHandle);

	u32 Result = Program;

	return(Result);
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
		glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex_list), Mesh->Vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_list), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_list), (void *)(3 * sizeof(f32)));
		glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(vertex_list), (void *)(8 * sizeof(f32)));
		glVertexAttribIPointer(3, 3, GL_UNSIGNED_INT, sizeof(vertex_list), (void *)(9 * sizeof(u32)));
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_list), (void *)(12 * sizeof(u32)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		ExpectedAttributeCount = 5;

		GLIBOInit(&Model->IBO[MeshIndex], Mesh->Indices, Mesh->IndicesCount);
		glBindVertexArray(0);

		s32 AttrCount;
		glGetProgramiv(ShaderProgram, GL_ACTIVE_ATTRIBUTES, &AttrCount);
		Assert(ExpectedAttributeCount == AttrCount);

		char Name[256];
		s32 Size = 0;
		GLsizei Length = 0;
		GLenum Type;
		for(s32 i = 0; i < AttrCount; ++i)
		{
			glGetActiveAttrib(ShaderProgram, i, sizeof(Name), &Length, &Size, &Type, Name);
			printf("Attribute:%d\nName:%s\nSize:%d\n\n", i, Name, Size);
		}
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
		glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex_list), Mesh->Vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_list), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_list), (void *)(3 * sizeof(f32)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_list), (void *)(6 * sizeof(f32)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		ExpectedAttributeCount = 3;

		GLIBOInit(&Model->IBO[MeshIndex], Mesh->Indices, Mesh->IndicesCount);
		glBindVertexArray(0);

		s32 AttrCount;
		glGetProgramiv(ShaderProgram, GL_ACTIVE_ATTRIBUTES, &AttrCount);
		Assert(ExpectedAttributeCount == AttrCount);

		char Name[256];
		s32 Size = 0;
		GLsizei Length = 0;
		GLenum Type;
		for(s32 i = 0; i < AttrCount; ++i)
		{
			glGetActiveAttrib(ShaderProgram, i, sizeof(Name), &Length, &Size, &Type, Name);
			//printf("Attribute:%d\nName:%s\nSize:%d\n\n", i, Name, Size);
		}
	}
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
OpenGLDrawAnimatedModel(model *Model, u32 ShaderProgram)
{
	mat4 M = Mat4Translate(Model->Basis.O) * Mat4(Model->Basis.X, Model->Basis.Y, Model->Basis.Z);
	UniformMatrixSet(ShaderProgram, "Model", M);
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
OpenGLDrawModel(model *Model, u32 ShaderProgram)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		glBindTexture(GL_TEXTURE_2D, Mesh->TextureHandle);
		glBindVertexArray(Model->VA[MeshIndex]);
		glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}


