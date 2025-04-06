#if !defined(TEXT_FILE_HANDLER_H)

//
// NOTE(Justin): Text file handler experiment. This is only to be used for simple text file formats
// that have short lines that look like:
//
// name value

struct text_file_handler
{
	u8 *FileContent;

	u8 LineBuffer_[4096];
	u8 *LineBuffer;
	u8 *Line;

	u8 Word_[4096];
	u8 *Word;

	u32 LineNumber;
};

inline void BufferNextWord(u8 **Content, u8 *Buffer);
inline void BufferNumber(u8 **Content, u8 *Buffer);
inline void BufferLine(u8 **Content, u8 *Buffer);
inline void AdvanceLine(u8 **Content);
inline void ParseInt(u8 **AddressOfLinePtr, u8 *Buffer, s32 *Dest);
inline void ParseUnsignedInt(u8 **AddressOfLinePtr, u8 *Buffer, u32 *Dest);
inline void ParseFloat(u8 **AddressOfLinePtr, u8 *Buffer, f32 *Dest);
inline void ParseV2(u8 **AddressOfLinePtr, u8 *Buffer, v2 *Dest);
inline void ParseV3(u8 **AddressOfLinePtr, u8 *Buffer, v3 *Dest);
inline void ParseV4(u8 **AddressOfLinePtr, u8 *Buffer, v4 *Dest);
inline void ParseQuaternion(u8 **AddressOfLinePtr, u8 *Buffer, quaternion *Dest);

#define TEXT_FILE_HANDLER_H
#endif
