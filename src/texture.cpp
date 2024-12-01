
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

internal texture
TextureLoad(char *FileName)
{
	texture Result = {};
	stbi_set_flip_vertically_on_load(true);
	Result.Memory = stbi_load(FileName, &Result.Width, &Result.Height, &Result.ChannelCount, 4);
	if(Result.Memory)
	{
		//Result.StoredFormat = GL_RGBA8;
		//Result.SrcFormat = GL_RGBA;
	}
	else
	{
		Assert(0);
	}

	stbi_set_flip_vertically_on_load(false);

	return(Result);
}

