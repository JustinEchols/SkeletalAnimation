#if !defined(INTRINSICS_H)

inline u32 
F32RoundToU32(f32 X)
{
	u32 Result = (u32)(X + 0.5f);
	return(Result);
}

#define INTRINSICS_H
#endif
