
internal text_file_handler
TextFileHandlerInitialize(void *FileContent)
{
	text_file_handler Result = {};

	Result.FileContent = (u8 *)FileContent;
	MemoryZero(Result.LineBuffer_, sizeof(Result.LineBuffer_));
	MemoryZero(Result.Word_, sizeof(Result.Word_));

	Result.LineBuffer = &Result.LineBuffer_[0];
	Result.Word = &Result.Word_[0];
	Result.LineNumber = 0;

	return(Result);
}

inline void 
TextFileHandlerReset(text_file_handler *Handler, void *FileContent)
{
	Handler->FileContent = (u8 *)FileContent;
	MemoryZero(Handler->LineBuffer_, sizeof(Handler->LineBuffer_));
	MemoryZero(Handler->Word_, sizeof(Handler->Word_));
	Handler->LineNumber = 0;
}

inline b32
IsValid(text_file_handler *Handler)
{
	b32 Result = (*Handler->FileContent != 0);
	return(Result);
}

inline b32
LineIsValid(text_file_handler *Handler)
{
	b32 Result = (*Handler->Line != 0);
	return(Result);
}

inline void
BufferAndAdvanceALine(text_file_handler *Handler)
{
	BufferLine(&Handler->FileContent, Handler->LineBuffer);
	Handler->Line = Handler->LineBuffer;
	BufferNextWord(&Handler->Line, Handler->Word);
	AdvanceLine(&Handler->FileContent);
	Handler->LineNumber++;
}

inline void
BufferAndAdvanceAWord(text_file_handler *Handler)
{
	EatSpaces(&Handler->Line);
	BufferNextWord(&Handler->Line, Handler->Word);
}

inline void
BufferNextWord(u8 **Content, u8 *Buffer)
{
	u32 At = 0;
	while(!IsNewLine(*Content) && !IsSpace(*Content) && !IsNull(*Content))
	{
		Buffer[At++] = **Content;
		(*Content)++;

	}
	Buffer[At] = '\0';
}

inline void
BufferNumber(u8 **Content, u8 *Buffer)
{
	u32 At = 0;
	while(IsNumber(*Content))
	{
		Buffer[At++] = **Content;
		(*Content)++;

	}
	Buffer[At] = '\0';
}

inline void
BufferLine(u8 **Content, u8 *Buffer)
{
	u32 At = 0;
	while((**Content) && !IsNewLine(**Content))
	{
		Buffer[At++] = **Content;
		(*Content)++;
	}
	Buffer[At] = '\0';
}

inline void
AdvanceLine(u8 **Content)
{
	while(IsNewLine(**Content))
	{
		(*Content)++;
	}
}

inline void
ParseUnsignedInt(u8 **AddressOfLinePtr, u8 *Buffer, u32 *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	*Dest = U32FromASCII(Buffer); 
}

inline void
ParseInt(u8 **AddressOfLinePtr, u8 *Buffer, s32 *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	*Dest = S32FromASCII(Buffer); 
}

inline void
ParseFloat(u8 **AddressOfLinePtr, u8 *Buffer, f32 *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	*Dest = F32FromASCII(Buffer); 
}

inline void
ParseV2(u8 **AddressOfLinePtr, u8 *Buffer, v2 *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->x = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->y = F32FromASCII(Buffer);
}

inline void
ParseV3(u8 **AddressOfLinePtr, u8 *Buffer, v3 *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->x = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->y = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->z = F32FromASCII(Buffer);
}

inline void
ParseV4(u8 **AddressOfLinePtr, u8 *Buffer, v4 *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->x = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->y = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->z = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	Dest->w = F32FromASCII(Buffer);
}

inline void
ParseQuaternion(u8 **AddressOfLinePtr, u8 *Buffer, quaternion *Dest)
{
	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	f32 X = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	f32 Y = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	f32 Z = F32FromASCII(Buffer);

	EatSpaces(AddressOfLinePtr);
	BufferNextWord(AddressOfLinePtr, Buffer);
	f32 Angle = F32FromASCII(Buffer);

	*Dest = Quaternion(V3(X, Y , Z), DegreeToRad(Angle));
}
