#if !defined(INTRINSICS_H)

inline u32 
F32RoundToU32(f32 X)
{
	u32 Result = (u32)(X + 0.5f);
	return(Result);
}

inline s32
F32TruncateToS32(f32 X)
{
	s32 Result = (s32)X;
	return(Result);
}

inline f32
AbsVal(f32 X)
{
	f32 Result = (f32)fabs(X);
	return(Result);
}

#define INTRINSICS_H
#endif
