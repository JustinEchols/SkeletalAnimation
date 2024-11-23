#define PushRenderElement(RenderBuffer, type) (type *)PushRenderElement_(RenderBuffer, sizeof(type), RenderBuffer_##type)
inline void *
PushRenderElement_(render_buffer *RenderBuffer, u32 TypeSize, render_buffer_entry_type Type)
{
	void *Result = 0;

	TypeSize += sizeof(render_buffer_entry_header);

	if((RenderBuffer->Size + TypeSize) < RenderBuffer->MaxSize)
	{
		// Init header of new type
		render_buffer_entry_header *Header = (render_buffer_entry_header *)(RenderBuffer->Base + RenderBuffer->Size);
		Header->Type = Type;

		// Get pointer to memory after header
		Result = (u8 *)Header + sizeof(*Header);

		// Adjust size
		RenderBuffer->Size += TypeSize;
	}

	return(Result);
}

inline void
PushClear(render_buffer *RenderBuffer, v4 Color)
{
	render_entry_clear *Entry = PushRenderElement(RenderBuffer, render_entry_clear);
	if(Entry)
	{
		Entry->Color = Color;
	}
}

inline void
PushQuad3D(render_buffer *RenderBuffer, u32 VA, mat4 Transform, u32 TextureHandle)
{
	render_entry_quad_3d *Entry = PushRenderElement(RenderBuffer, render_entry_quad_3d);
	if(Entry)
	{
		Entry->VA = VA;
		Entry->Transform = Transform;
		Entry->TextureHandle = TextureHandle;
	}
}

inline void
PushModel(render_buffer *RenderBuffer, model *Model, mat4 Transform)
{
	render_entry_model *Entry = PushRenderElement(RenderBuffer, render_entry_model);
	if(Entry)
	{
		Entry->Model = Model;
		Entry->Transform = Transform;
	}
}

internal render_buffer * 
RenderBufferAllocate(memory_arena *Arena, u32 MaxSize,
		mat4 View, mat4 Perspective,
		u32 MainShader, u32 BasicShader,
		asset_manager *Assets,
		v3 CameraP)
{
	render_buffer *Result = (render_buffer *)PushStruct(Arena, render_buffer);

	Result->Base = (u8 *)PushSize_(Arena, MaxSize);
	Result->Size = 0;
	Result->MaxSize = MaxSize;
	Result->View = View;
	Result->Perspective = Perspective;
	Result->MainShader = MainShader;
	Result->BasicShader = BasicShader;
	Result->Assets = Assets;
	Result->CameraP = CameraP;

	return(Result);
}

internal void
RenderBufferToOutput(render_buffer *RenderBuffer, u32 WindowWidth, u32 WindowHeight)
{
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

	u32 MainShader = RenderBuffer->MainShader;
	u32 BasicShader = RenderBuffer->BasicShader;
	mat4 View = RenderBuffer->View;
	mat4 Perspective = RenderBuffer->Perspective;
	v3 LightDir = V3(1.0f, -1.0f, -0.5f);
	v3 CameraP = RenderBuffer->CameraP;

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
			case RenderBuffer_render_entry_quad_3d:
			{
				render_entry_quad_3d *Entry = (render_entry_quad_3d *)Data;
				glUseProgram(BasicShader);
				UniformMatrixSet(BasicShader, "View", View);
				UniformMatrixSet(BasicShader, "Projection", Perspective);
				UniformBoolSet(BasicShader, "OverRideTexture", false);
				UniformV3Set(BasicShader, "Ambient", V3(0.1f));
				UniformV3Set(BasicShader, "LightDir", LightDir);
				OpenGLDrawQuad(Entry->VA, BasicShader, Entry->Transform, Entry->TextureHandle);
				BaseOffset += sizeof(*Entry);
			} break;

			case RenderBuffer_render_entry_model:
			{
				render_entry_model *Entry = (render_entry_model *)Data;

				if(Entry->Model->HasSkeleton)
				{
					glUseProgram(MainShader);
					UniformMatrixSet(MainShader, "View", View);
					UniformMatrixSet(MainShader, "Projection", Perspective);
					UniformV3Set(MainShader, "CameraP", CameraP);
					UniformV3Set(MainShader, "Ambient", V3(0.1f));
					UniformV3Set(MainShader, "CameraP", CameraP);
					UniformV3Set(MainShader, "LightDir", LightDir);
					OpenGLDrawAnimatedModel(Entry->Model, MainShader, Entry->Transform);
				}
				else
				{
					glUseProgram(BasicShader);
					UniformMatrixSet(BasicShader, "View", View);
					UniformMatrixSet(BasicShader, "Projection", Perspective);
					UniformBoolSet(BasicShader, "OverRideTexture", false);
					UniformV3Set(BasicShader, "Ambient", V3(0.1f));
					UniformV3Set(BasicShader, "LightDir", LightDir);
					UniformV4Set(BasicShader, "Color", V4(1.0f));
					OpenGLDrawModel(Entry->Model, BasicShader, Entry->Transform);
				}

				BaseOffset += sizeof(*Entry);
			} break;
		}
	}
}


