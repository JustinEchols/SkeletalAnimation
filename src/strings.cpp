inline b32
IsNull(char C)
{
	b32 Result = (C == '\0');
	return(Result);
}

inline b32
IsNull(u8 C)
{
	b32 Result = (C == '\0');
	return(Result);
}

inline b32
IsNull(u8 *C)
{
	b32 Result = IsNull(*C);
	return(Result);
}

inline b32 
IsNewLine(char C)
{
	b32 Result = ((C == '\n') || (C == '\r'));
	return(Result);
}

inline b32
IsNewLine(char *S)
{
	b32 Result = ((*S == '\n') || (*S == '\r'));
	return(Result);
}

inline b32 
IsNewLine(u8 *S)
{
	b32 Result = ((*S == '\n') || (*S == '\r'));
	return(Result);
}

inline b32 
IsSpace(char C)
{
	b32 Result = ((C == ' '));
	return(Result);
}

inline b32 
IsSpace(u8 *S)
{
	b32 Result = ((*S == ' '));
	return(Result);
}

inline b32
IsNumber(char *S)
{
	b32 Result = (((*S) >= '0') && ((*S) <= '9'));
	return(Result);
}

inline b32
IsNumber(u8 *S)
{
	b32 Result = (((*S) >= '0') && ((*S) <= '9'));
	return(Result);
}

inline s32
S32FromASCII(char *S)
{
	s32 Result = atoi(S);
	return(Result);
}

inline s32
S32FromASCII(u8 *S)
{
	s32 Result = S32FromASCII((char *)S);
	return(Result);
}

inline u32
U32FromASCII(char *S)
{
	u32 Result = (u32)atoi(S);
	return(Result);
}

inline u32
U32FromASCII(u8 *S)
{
	u32 Result = (u32)atoi((char *)S);
	return(Result);
}

inline u32
U32FromASCII(string S)
{
	u32 Result = U32FromASCII(S.Data);
	return(Result);
}

inline f32
F32FromASCII(char *S)
{
	f32 Result = (f32)atof(S);
	return(Result);
}

inline f32
F32FromASCII(u8 *S)
{
	f32 Result = (f32)atof((char *)S);
	return(Result);
}

inline f32
F32FromASCII(string S)
{
	f32 Result = F32FromASCII(S.Data);
	return(Result);
}

inline void 
F32ToString(char *Dest, char *Format, f32 F32)
{
	sprintf(Dest, Format, F32);
}


//
// NOTE(Justin): Simple text file format routines
//

inline void
EatSpaces(u8 **Buff)
{
	while(**Buff == ' ')
	{
		(*Buff)++;
	}
}

inline void
EatSpaces(char **Buff)
{
	while(**Buff == ' ')
	{
		(*Buff)++;
	}
}

inline void
EatUntilSpace(u8 **Buff)
{
	while(**Buff != ' ')
	{
		(*Buff)++;
	}
}

// TODO(Justin): This is dangerous as if the arguements get passed in the incorrect order
// then bad things can happen... Also there is no size checking on the buffer
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

inline string
StringFromRange(u8 *First, u8 *Last)
{
	string Result = {First, (u64)(Last - First)};
	return(Result);
}

internal string
String(u8 *Cstr)
{
	u8 *C = Cstr;
	for(; *C; ++C);

	string Result = StringFromRange(Cstr, C);

	return(Result);
}

internal string
String(char *Cstr)
{
	string Result = String((u8 *)Cstr);
	return(Result);
}

internal u32
StringLen(char *S)
{
	u32 Result = 0;

	char *P = S;
	while(*P++)
	{
		Result++;
	}

	return(Result);
}

internal u32
StringLen(u8 *S)
{
	u32 Result = StringLen((char *)S);
	return(Result);
}

internal b32
StringEndsWith(string S, char C)
{
	u8 *Test = S.Data + (S.Size - 1);
	b32 Result = (*Test == (u8 )C);

	return(Result);
}

internal b32
StringsAreSame(string S1, string S2)
{
	b32 Result = (S1.Size == S2.Size);
	if(Result)
	{
		for(u64 Index = 0; Index < S1.Size; ++Index)
		{
			if(S1.Data[Index] != S2.Data[Index])
			{
				Result = false;
				break;
			}
		}
	}

	return(Result);
}

internal b32
StringsAreSame(char *Str1, char *Str2)
{
	string S1 = String((u8 *)Str1);
	string S2 = String((u8 *)Str2);

	b32 Result = StringsAreSame(S1, S2);

	return(Result);
}

internal b32
StringsAreSame(u8 *Str1, u8 *Str2)
{
	string S1 = String(Str1);
	string S2 = String(Str2);

	b32 Result = StringsAreSame(S1, S2);

	return(Result);
}

internal b32
StringsAreSame(string S1, char *Str2)
{
	string S2 = String((u8 *)Str2);

	b32 Result = StringsAreSame(S1, S2);

	return(Result);
}

internal b32
StringsAreSame(u8 *S1, char *S2)
{
	b32 Result = StringsAreSame(String(S1), String((u8 *)S2));
	return(Result);
}

internal b32
StringsAreSame(char *S1, u8 *S2)
{
	b32 Result = StringsAreSame(S2, S1);
	return(Result);
}

internal b32
SubStringExists(char *HayStack, char *Needle)
{
	b32 Result = false;
	if(HayStack && Needle)
	{
		char *S = strstr(HayStack, Needle);
		if(S)
		{
			Result = true;
		}
	}

	return(Result);
}

internal b32
SubStringExists(string HayStack, char *Needle)
{
	b32 Result = SubStringExists(CString(HayStack), Needle);
	return(Result);
}

internal string
StringSearchFor(string S, char C)
{
	u8 *P = S.Data;
	u8 PC = (u8)C;
	for(u32 Index = 0; Index < S.Size; Index++)
	{
		P++;
		if(*P == PC)
		{
			break;
		}
	}

	string Result = StringFromRange(P, S.Data + S.Size);

	return(Result);

}

internal string
StringCopy(memory_arena *Arena, string Str)
{
	Assert(Str.Data);

	string Result = {};
	
	Result.Size = Str.Size;
	Result.Data = PushArray(Arena, Result.Size + 1, u8);

	ArrayCopy(Result.Size, Str.Data, Result.Data);
	Result.Data[Result.Size] = '\0';

	return(Result);
}

internal string
StringCopy(memory_arena *Arena, char *Cstr)
{
	string Result = StringCopy(Arena, String((u8 *)Cstr));
	return(Result);
}

internal string
StringCopy(memory_arena *Arena, u8 *Str)
{
	string Result = StringCopy(Arena, String(Str));
	return(Result);
}


internal void
ParseStringArray(memory_arena *Arena, string *Dest, u32 DestCount, string Str)
{
	char *Context;
	char *Tok = strtok_s((char *)Str.Data, " \n\r", &Context);

	u32 Index = 0;
	if(!IsNewLine(Tok))
	{
		Dest[Index++] = StringCopy(Arena, Tok);
	}

	while(Tok)
	{
		Tok = strtok_s(0, " \n\r", &Context);
		if(Tok)
		{
			if(!IsNewLine(Tok))
			{
				Dest[Index++] = StringCopy(Arena, Tok);
			}
		}
	}
}

internal u32
StringHashIndex(char *String, u32 EntryCount)
{
	u32 Hash = 5381;
	char *C = String;
	while(*C++)
	{
		Hash = (((Hash << 5) + Hash) + *C);
	}

	Hash = Hash % (EntryCount - 1);

	return(Hash);
}

#define INVALID_DEFAULT_VALUE -1
internal void
StringHashInitialize(string_hash *StringHash)
{
	StringHash->Count = 0;
	for(u32 Index = 0; Index < ArrayCount(StringHash->Entries); ++Index)
	{
		string_hash_entry *Entry = StringHash->Entries + Index;
		Entry->Index = INVALID_DEFAULT_VALUE;
	}
}

internal void
StringHashAdd(string_hash *Hash, char *String, u32 Index)
{
	u32 HashIndex = StringHashIndex(String, ArrayCount(Hash->Entries));
	Assert(HashIndex < ArrayCount(Hash->Entries));
	string_hash_entry *Entry = Hash->Entries + HashIndex;
	if(Entry->Index == INVALID_DEFAULT_VALUE)
	{
		Entry->Index = Index;
		Entry->Key = StringCopy(&Hash->Arena, String);
		Hash->Count++;
	}
	else
	{
		for(string_hash_entry *E = Entry;; E = E->Next)
		{
			if(E)
			{
				if(E->Index == INVALID_DEFAULT_VALUE)
				{
					E->Index = Index;
					E->Key = StringCopy(&Hash->Arena, String);
					Hash->Count++;
					break;
				}
			}

			if(!E->Next)
			{
				E->Next = PushStruct(&Hash->Arena, string_hash_entry);
				E = E->Next;
				E->Index = Index;
				E->Key = StringCopy(&Hash->Arena, String);
				Hash->Count++;
				break;
			}
		}
	}
}

internal s32
StringHashLookup(string_hash *Hash, char *String)
{
	s32 Result = -1;

	u32 HashIndex = StringHashIndex(String, ArrayCount(Hash->Entries));
	Assert(HashIndex < ArrayCount(Hash->Entries));
	string_hash_entry *Entry = Hash->Entries + HashIndex;
	if(StringsAreSame(Entry->Key, String))
	{
		Result = Entry->Index;
	}
	else
	{
		for(string_hash_entry *E = Entry; E; E = E->Next)
		{
			if(StringsAreSame(E->Key, String))
			{
				Result = E->Index;
			}
		}
	}

	return(Result);
}

internal void 
FileNameFromFullPath(char *FullPath, char *Dest)
{
	u64 OPLSlash = 0;
	for(char *C = FullPath; *C; ++C)
	{
		if((*C == '/') || (*C == '\\'))
		{
			OPLSlash = (C - FullPath) + 1;
		}
	}

	u32 At = 0;
	for(char *C = (FullPath + OPLSlash); *C; ++C)
	{
		if(*C == '.')
		{
			break;
		}

		Dest[At++] = *C;
	}
	Dest[At] = '\0';
}

internal void 
ExtFromFullPath(char *FullPath, char *Dest)
{
	u64 OPExt = 0;
	for(char *C = FullPath; *C; ++C)
	{
		if(*C == '.')
		{
			OPExt = (C - FullPath) + 1;
		}
	}

	u32 At = 0;
	for(char *C = (FullPath + OPExt); *C; ++C)
	{
		Dest[At++] = *C;
	}
	Dest[At] = '\0';
}
