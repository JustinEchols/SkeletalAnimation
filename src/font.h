#if !defined(FONT_H)

struct glyph
{
	u32 TextureHandle;
	v2i Dim;
	v2i Bearing;
	u32 Advance;
	u8 *Memory;
};

// TODO(Justin): Glyph atlas
// TODO(Justin): Kerning 
struct font
{
	u32 VA;
	u32 VB;
	b32 UploadedToGPU;

	string Name;
	glyph Glyphs[256];
	s32 Ascender;
	s32 Descender;
	s32 LineHeight;
};

#define FONT_H
#endif
