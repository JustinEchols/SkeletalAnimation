#if !defined(RENDER_H)

enum render_buffer_entry_type
{
	RenderBuffer_render_entry_clear,
	RenderBuffer_render_entry_texture,
	RenderBuffer_render_entry_quad_3d,
	RenderBuffer_render_entry_model,
	RenderBuffer_render_entry_text,
};

struct render_buffer_entry_header
{
	render_buffer_entry_type Type;
};

struct render_entry_clear
{
	v4 Color;
};

struct render_entry_texture
{
	s32 Index;
};

struct render_entry_quad_3d
{
	quad *Quad;
	mat4 Transform;
	u32 TextureIndex;
};

struct render_entry_model
{
	mat4 Transform;
	model *Model;
};

struct render_entry_text
{
	char *Text;
	font *Font;
	v2 P;
	f32 Scale;
	v3 Color;
};

struct render_buffer
{
	u8 *Base;
	u32 Size;
	u32 MaxSize;

	mat4 View;
	mat4 Perspective;
	v3 CameraP;

	u32 TextureCount;
	u32 MaxTextureCount;
	texture **Textures;

	asset_manager *Assets;
};

#define RENDER_H
#endif
