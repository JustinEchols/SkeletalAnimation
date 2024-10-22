#if !defined(FONT_H)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct glyph_box
{
	v2 Offset;
	v2 Dim;
};

struct font
{
	b32 Initialized;
	u32 TextureHandle;
	f32 TopToBaseLine;
	f32 BaseLineToNextTop;
	glyph_box Glyph[128];
	f32 Advance[128];
};

#define FONT_H
#endif
