#if !defined(FONT_H)

struct glyph
{
	u32 TextureHandle;
	v2i Dim;
	v2i Bearing;
	u32 Advance;
};

struct font_info
{
	string Name;
	glyph Glyphs[256];
	s32 Ascender;
	s32 Descender;
	s32 LineHeight;
};

struct font_quad
{
	u32 VA;
	u32 VB;
	font_info Info;
};

#define FONT_H
#endif
