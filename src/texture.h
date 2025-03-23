#if !defined(TEXTURE_H)

struct texture
{
	string Path;
	string Name;
	//GLint StoredFormat;
	//GLenum SrcFormat;
	s32 Width, Height;
	s32 ChannelCount;
	u8 *Memory;

	u32 Handle;
};

#define TEXTURE_H
#endif
