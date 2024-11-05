
internal s32 
FontInit(font_info *FontInfo, char *FileName)
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

	FontInfo->Name = String(FileName);
	FontInfo->Ascender = Face->ascender;
	FontInfo->Descender = Face->descender;
	FontInfo->LineHeight = Face->height;

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

		FontInfo->Glyphs[CharIndex].TextureHandle = GlyphTexture;
		FontInfo->Glyphs[CharIndex].Dim = V2I(Face->glyph->bitmap.width, Face->glyph->bitmap.rows);
		FontInfo->Glyphs[CharIndex].Bearing = V2I(Face->glyph->bitmap_left, Face->glyph->bitmap_top);
		FontInfo->Glyphs[CharIndex].Advance = Face->glyph->advance.x;

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	FT_Done_Face(Face);
	FT_Done_FreeType(Ft);

	return(Result);
}
