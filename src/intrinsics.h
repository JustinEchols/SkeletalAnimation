#if !defined(INTRINSICS_H)

#include <math.h>

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

inline s32
Floor(f32 X)
{
	s32 Result = (s32)floorf(X);
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
	f32 Result = fmaxf(X, Y);
	return(Result);
}

inline f32
Max3(f32 X, f32 Y, f32 Z)
{
	f32 Max1 = Max(X, Y);
	f32 Max2 = Max(Y, Z);
	f32 Result = Max(Max1, Max2);
	return(Result);
}

inline f32
Min(f32 X, f32 Y)
{
	f32 Result = fminf(X, Y);
	return(Result);
}

inline f32
Min3(f32 X, f32 Y, f32 Z)
{
	f32 Min1 = Min(X, Y);
	f32 Min2 = Min(Y, Z);
	f32 Result = Min(Min1, Min2);
	return(Result);
}

inline void
RandInit(u32 Seed)
{
	srand(Seed);
}

inline f32
Rand01(void)
{
	f32 Result = rand() / (f32)RAND_MAX;
	return(Result);
}

inline f32
RandBetween(f32 A, f32 B)
{
	f32 Result = (B - A) * Rand01() + A;
	return(Result);
}

inline f32
SignOf(f32 X)
{
	f32 Result = 1.0f;

	if(X < 0.0f)
	{
		Result *= -1.0f;
	}

	return(Result);
}


#define INTRINSICS_H
#endif
