#if !defined(RENDER_H)

// TODO(Justin): quad_3d_vertex?
struct quad_vertex
{
	v3 P;
	v3 N;
	v2 UV;
};

// TODO(Justin): quad_3d?
struct quad
{
	quad_vertex Vertices[6];
};

struct quad_2d
{
	u32 VA;
	u32 VB;
	u32 Texture;
};

enum render_buffer_entry_type
{
	RenderBuffer_render_entry_clear,
	RenderBuffer_render_entry_texture,
	RenderBuffer_render_entry_quad_3d,
	RenderBuffer_render_entry_quad_2d,
	RenderBuffer_render_entry_model,
	RenderBuffer_render_entry_text,
	RenderBuffer_render_entry_aabb,
	RenderBuffer_render_entry_render_to_texture,
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
	//quad *Quad;
	quad_vertex Vertices[6];
	mat4 Transform;
	u32 TextureIndex;
};

// TODO(justin): What else should be in here?
struct render_entry_quad_2d
{
	f32 Vertices[6][4];
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

struct render_entry_aabb
{
	mat4 Transform;
	model *Model;
	//v3 Dim;
	v3 Color;
};

struct render_entry_render_to_texture
{
	f32 Vertices[6][4];
};


struct render_buffer
{
	u8 *Base;
	u32 Size;
	u32 MaxSize;
	u32 OutputTargetIndex;

	mat4 View;
	mat4 Perspective;
	mat4 LightTransform;
	v3 CameraP;
	v3 LightDir;

	u32 TextureCount;
	u32 MaxTextureCount;
	texture **Textures;

	struct asset_manager *Assets;
};

#define RENDER_H
#endif
