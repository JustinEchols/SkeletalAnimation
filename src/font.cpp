
internal s32 
FontInit(font *Font, char *FileName)
{
	s32 Result = 1;

	FT_Library Ft;
	if(FT_Init_FreeType(&Ft))
	{
		PrintString("ERROR: Unable to load free type library");
		return(-1);
	}

	FT_Face Face;
	if(FT_New_Face(Ft, FileName, 0, &Face))
	{
		PrintString("ERROR: Unable to load font.");
		return(-1);
	}

	Font->Name = String(FileName);
	Font->Ascender = Face->ascender;
	Font->Descender = Face->descender;
	Font->LineHeight = Face->height;

	s32 Width = 0;
	s32 Height = 48;

	FT_Set_Pixel_Sizes(Face, Width, Height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for(char CharIndex = ' '; CharIndex < '~'; ++CharIndex)
	{
		if(FT_Load_Char(Face, CharIndex, FT_LOAD_RENDER))
		{
			PrintString("ERROR: Unable to glyph.");
			return(-1);
		}

		if(!IsSpace(CharIndex))
		{
			Assert(Face->glyph->bitmap.width != 0);
			Assert(Face->glyph->bitmap.rows != 0);
			Assert(Face->glyph->bitmap.buffer);
		}

		u32 GlyphTexture;
		glGenTextures(1, &GlyphTexture);
		glBindTexture(GL_TEXTURE_2D, GlyphTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_R8,
				Face->glyph->bitmap.width,
				Face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				Face->glyph->bitmap.buffer);

		Font->Glyphs[CharIndex].TextureHandle = GlyphTexture;
		Font->Glyphs[CharIndex].Dim = V2I(Face->glyph->bitmap.width, Face->glyph->bitmap.rows);
		Font->Glyphs[CharIndex].Bearing = V2I(Face->glyph->bitmap_left, Face->glyph->bitmap_top);
		Font->Glyphs[CharIndex].Advance = Face->glyph->advance.x;

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
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
