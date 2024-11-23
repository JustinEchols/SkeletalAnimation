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
	}
}

inline void
PushQuad3D(render_buffer *RenderBuffer, quad *Quad, mat4 Transform, u32 TextureIndex)
{
	render_entry_quad_3d *Entry = PushRenderElement(RenderBuffer, render_entry_quad_3d);
	if(Entry)
	{
		Entry->Quad = Quad;
		Entry->Transform = Transform;
		Entry->TextureIndex = TextureIndex;
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
		asset_manager *Assets,
		v3 CameraP)
{
	render_buffer *Result = (render_buffer *)PushStruct(Arena, render_buffer);

	Result->Base = (u8 *)PushSize_(Arena, MaxSize);
	Result->Size = 0;
	Result->MaxSize = MaxSize;
	Result->View = View;
	Result->Perspective = Perspective;
	Result->Assets = Assets;
	Result->CameraP = CameraP;

	Result->TextureCount = 0;
	Result->MaxTextureCount = 32;
	Result->Textures = PushArray(Arena, Result->MaxTextureCount, texture *);

	return(Result);
}

