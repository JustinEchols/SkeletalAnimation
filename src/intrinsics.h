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

inline f32
Sqrt(f32 X)
{
	f32 Result = sqrtf(X);
	return(Result);
}

inline f32
Cos(f32 X)
{
	f32 Result = cosf(X);
	return(Result);
}

inline f32
Sin(f32 X)
{
	f32 Result = sinf(X);
	return(Result);
}

inline f32
ATan2(f32 Y, f32 X)
{
	f32 Result = (f32)atan2(Y, X);
	return(Result);
}

inline f32
ACos(f32 X)
{
	f32 Result = (f32)acos(X);
	return(Result);

}

inline f32
Max(f32 X, f32 Y)
{
	f32 Result = max(X, Y);
	return(Result);
}

#define INTRINSICS_H
#endif
