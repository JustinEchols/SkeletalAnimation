
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
PushTexture(render_buffer *RenderBuffer, texture *Texture, s32 Index)
{
	render_entry_texture *Entry = PushRenderElement(RenderBuffer, render_entry_texture);
	if(Entry)
	{
		texture **Tex = RenderBuffer->Textures + Index;
		*Tex = Texture;
		Entry->Index = Index;
		RenderBuffer->TextureCount++;
	}
}

inline void
PushQuad3D(render_buffer *RenderBuffer, quad_vertex *Vertices, mat4 Transform, u32 TextureIndex)
{
	render_entry_quad_3d *Entry = PushRenderElement(RenderBuffer, render_entry_quad_3d);
	if(Entry)
	{
		MemoryCopy(sizeof(Entry->Vertices), Vertices, Entry->Vertices);
		Entry->Transform = Transform;
		Entry->TextureIndex = TextureIndex;
	}
}

inline void
PushQuad2D(render_buffer *RenderBuffer, f32 *Vertices, u32 TextureIndex)
{
	render_entry_quad_2d *Entry = PushRenderElement(RenderBuffer, render_entry_quad_2d);
	if(Entry)
	{
		// TODO(justin): Should we use a pointer instead of f32 A[6][4]?
		MemoryCopy(sizeof(Entry->Vertices), Vertices, Entry->Vertices);
		Entry->TextureIndex = TextureIndex;
	}
}

inline void
PushQuad2D(render_buffer *RenderBuffer, f32 *Vertices, v4 Color)
{
	render_entry_quad_2d *Entry = PushRenderElement(RenderBuffer, render_entry_quad_2d);
	if(Entry)
	{
		// TODO(justin): Should we use a pointer instead of f32 A[6][4]?
		MemoryCopy(sizeof(Entry->Vertices), Vertices, Entry->Vertices);
		Entry->TextureIndex = 0;
		Entry->Color = Color;
	}
}

inline void
PushModel(render_buffer *RenderBuffer, char *ModelName, mat4 Transform)
{
	model *Model = LookupModel(RenderBuffer->Assets, ModelName).Model;
	if(Model)
	{
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			mesh *Mesh = Model->Meshes + MeshIndex;

			if(Mesh->DiffuseTexture)
			{
				texture *Texture = RenderBuffer->Assets->Textures + Mesh->DiffuseTexture;
				if(Texture)
				{
					PushTexture(RenderBuffer, Texture, Mesh->DiffuseTexture);
				}
			}

			if(Mesh->SpecularTexture)
			{
				texture *Texture = RenderBuffer->Assets->Textures + Mesh->SpecularTexture;
				if(Texture)
				{
					PushTexture(RenderBuffer, Texture, Mesh->SpecularTexture);
				}
			}
		}

		render_entry_model *Entry = PushRenderElement(RenderBuffer, render_entry_model);
		if(Entry)
		{
			Entry->Model = Model;
			Entry->Transform = Transform;
		}
	}
}

// NOTE(Justin): Testing a different push buffer call
inline void
PushMesh(render_buffer *RenderBuffer, mesh *Mesh, mat4 Transform, u32 TextureIndex)
{
	render_entry_mesh *Entry = PushRenderElement(RenderBuffer, render_entry_mesh);
	if(Entry)
	{
		Entry->Mesh = Mesh;
		Entry->Transform = Transform;
		Entry->TextureIndex = TextureIndex;
	}
}

inline void
PushText(render_buffer *RenderBuffer, string Text, font *Font, v2 P, f32 Scale, v3 Color)
{
	render_entry_text *Entry = (render_entry_text *)PushRenderElement(RenderBuffer, render_entry_text);
	if(Entry)
	{
		Entry->Text = CString(Text);
		Entry->Font = Font;
		Entry->P = P;
		Entry->Scale = Scale;
		Entry->Color = Color;
	}
}

inline void
PushDebugVolume(render_buffer *RenderBuffer, model *Model, mat4 Transform, v3 Color)
{
	render_entry_debug_volume *Entry = (render_entry_debug_volume *)PushRenderElement(RenderBuffer, render_entry_debug_volume);
	if(Entry)
	{
		Entry->VolumeType = DebugVolumeType_AABB;
		Entry->Model = Model;
		Entry->Transform = Transform;
		Entry->Color = Color;
	}
}

inline void
PushRenderToTexture(render_buffer *RenderBuffer, f32 *Vertices)
{
	render_entry_render_to_texture *Entry = PushRenderElement(RenderBuffer, render_entry_render_to_texture);
	if(Entry)
	{
		// TODO(justin): Should we use a pointer instead of f32 A[6][4]?
		MemoryCopy(sizeof(Entry->Vertices), Vertices, Entry->Vertices);
	}
}

internal render_buffer * 
RenderBufferAllocate(memory_arena *Arena, u32 MaxSize,
		mat4 View, mat4 Perspective, mat4 LightTransform,
		asset_manager *Assets,
		v3 CameraP, v3 LightDir,
		u32 OutputTargetIndex = 0)
{
	render_buffer *Result = (render_buffer *)PushStruct(Arena, render_buffer);

	Result->Base = (u8 *)PushSize_(Arena, MaxSize);
	Result->Size = 0;
	Result->MaxSize = MaxSize;
	Result->OutputTargetIndex = OutputTargetIndex;
	Result->View = View;
	Result->Perspective = Perspective;
	Result->LightTransform = LightTransform;
	Result->Assets = Assets;
	Result->CameraP = CameraP;
	Result->LightDir = LightDir;

	Result->TextureCount = 0;
	Result->MaxTextureCount = 32;
	Result->Textures = PushArray(Arena, Result->MaxTextureCount, texture *);

	return(Result);
}

