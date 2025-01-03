internal s32 
FontInitialize(memory_arena *Arena, font *Font, char *FileName)
{
	s32 Result = 1;

	FT_Library Ft;
	if(FT_Init_FreeType(&Ft))
	{
		return(-1);
	}

	FT_Face Face;
	if(FT_New_Face(Ft, FileName, 0, &Face))
	{
		return(-1);
	}

	char Buffer[64];
	FileNameFromFullPath(FileName, Buffer);
	Font->Name = StringCopy(Arena, Buffer);
	Font->Ascender = Face->ascender;
	Font->Descender = Face->descender;
	Font->LineHeight = Face->height;
	Font->Scale = 0.35f;
	Font->LineGap = Font->Scale * (f32)Font->LineHeight / 64.0f;

	s32 Width = 0;
	s32 Height = 48;

	FT_Set_Pixel_Sizes(Face, Width, Height);
	for(char CharIndex = ' '; CharIndex < '~'; ++CharIndex)
	{
		if(FT_Load_Char(Face, CharIndex, FT_LOAD_RENDER))
		{
			return(-1);
		}

		if(!IsSpace(CharIndex))
		{
			Assert(Face->glyph->bitmap.width != 0);
			Assert(Face->glyph->bitmap.rows != 0);
			Assert(Face->glyph->bitmap.buffer);
		}

		glyph *Glyph = Font->Glyphs + CharIndex;
		Glyph->Dim = V2I(Face->glyph->bitmap.width, Face->glyph->bitmap.rows);
		Glyph->Bearing = V2I(Face->glyph->bitmap_left, Face->glyph->bitmap_top);
		Glyph->Advance = Face->glyph->advance.x;
		Glyph->Memory = (u8 *)PushSize_(Arena, Glyph->Dim.x * Glyph->Dim.y * sizeof(u8));
		MemoryCopy(Glyph->Dim.x * Glyph->Dim.y * sizeof(u8), Face->glyph->bitmap.buffer, Glyph->Memory);
	}

	FT_Done_Face(Face);
	FT_Done_FreeType(Ft);

	return(Result);
}

internal v2 
TextDim(font *Font, f32 Scale, char *String)
{	
	v2 Result = {};

	f32 Width = 0.0f;
	f32 MaxHeight = 0.0f;
	for(char *C = String; *C; ++C)
	{
		glyph Glyph = Font->Glyphs[*C];

		f32 Advance = (f32)(Glyph.Advance >> 6);
		f32 W = (f32)Glyph.Dim.x;
		f32 dX = Advance - W;

		Width += W + dX;

		if(Glyph.Dim.y > MaxHeight)
		{
			MaxHeight = (f32)Glyph.Dim.y;
		}
	}

	Result = Scale * V2(Width, MaxHeight);

	return(Result);
}

