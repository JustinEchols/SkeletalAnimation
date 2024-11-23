#if !defined(RENDER_H)

enum render_buffer_entry_type
{
	RenderBuffer_render_entry_clear,
	RenderBuffer_render_entry_quad_3d,
	RenderBuffer_render_entry_model,
};

struct render_buffer_entry_header
{
	render_buffer_entry_type Type;
};

struct render_entry_clear
{
	v4 Color;
};

struct render_entry_quad_3d
{
	u32 VA;
	mat4 Transform;
	u32 TextureHandle;
};

struct render_entry_model
{
	b32 Animated;
	mat4 Transform;
	model *Model;
};

struct render_buffer
{
	u8 *Base;
	u32 Size;
	u32 MaxSize;

	mat4 View;
	mat4 Perspective;
	v3 CameraP;

	u32 MainShader;
	u32 BasicShader;

	asset_manager *Assets;
};

#define RENDER_H
#endif
