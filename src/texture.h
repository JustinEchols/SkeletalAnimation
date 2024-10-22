#if !defined(TEXTURE_H)

struct texture
{
	char *FileName;
	u32 Handle;
	GLint StoredFormat;
	GLenum SrcFormat;
	s32 Width, Height;
	s32 ChannelCount;
	u8 *Memory;
};

#define TEXTURE_H
#endif
