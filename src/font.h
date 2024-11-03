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
	char *Name;
	glyph Glyphs[256];
};

struct font_quad_vertex
{
	v4 XYUV;
};

struct font_quad
{
	u32 VA;
	u32 VB;
	font_quad_vertex Vertices[6];
	font_info Info;
};

#define FONT_H
#endif
