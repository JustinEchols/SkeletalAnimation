#if !defined(TEXTURE_H)

enum texture_source_format
{
	TextureSrcFormat_RGBA
};

enum texture_gpu_format
{
	TextureGPUFormat_RGBA8
};

struct texture
{
	string Name;
	texture_gpu_format GPUFormat;
	texture_source_format SrcFormat;
	//GLint StoredFormat;
	//GLenum SrcFormat;
	s32 Width, Height;
	s32 ChannelCount;
	u8 *Memory;

	u32 Handle;
};

#define TEXTURE_H
#endif
