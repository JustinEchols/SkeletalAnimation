#if !defined(MATH_UTIL_H)

//#include "math.h"

union v2
{
	struct
	{
		f32 x, y;
	};
	f32 E[2];
};

union v2i
{
	struct
	{
		s32 x, y;
	};
	s32 E[2];
};


union v3
{
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 pitch, yaw, roll;
	};
	f32 E[3];
};

union v4
{
	struct
	{
		f32 x, y, z, w;
	};
	struct
	{
		union
		{
			v3 xyz;
			struct
			{
				f32 x, y, z;
			};
		};
		f32 w;
	};
	f32 E[4];
};

struct mat3
{
	f32 E[3][3];
};

struct mat4
{
	f32 E[4][4];
};

struct basis
{
	v3 O;
	v3 X;
	v3 Y;
	v3 Z;
};

union quaternion
{
	struct
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
	f32 E[4];
};

struct sqt
{
	v3 Position;
	quaternion Orientation;
	v3 Scale;
};

inline v2
V2(f32 X, f32 Y)
{
	v2 Result = {};

	Result.x = X;
	Result.y = Y;

	return(Result);
}

inline v2
V2(f32 X)
{
	v2 Result = {};

	Result.x = X;
	Result.y = X;

	return(Result);
}

inline v2i
V2I(s32 X, s32 Y)
{
	v2i Result = {};

	Result.x = X;
	Result.y = Y;

	return(Result);
}

inline v2i
V2I(s32 X)
{
	v2i Result = {};

	Result.x = X;
	Result.y = X;

	return(Result);
}

inline v3
V3(f32 X, f32 Y, f32 Z)
{
	v3 Result;

	Result.x = X;
	Result.y = Y;
	Result.z = Z;

	return(Result);
}

inline v4
V4(f32 C)
{
	v4 Result = {};

	Result.x = C;
	Result.y = C;
	Result.z = C;
	Result.w = C;

	return(Result);
}

inline v4
V4(f32 X, f32 Y, f32 Z, f32 W)
{
	v4 Result = {};

	Result.x = X;
	Result.y = Y;
	Result.z = Z;
	Result.w = W;

	return(Result);
}

inline v4
V4(f32 X, f32 Y, f32 Z)
{
	v4 Result = {};

	Result.x = X;
	Result.y = Y;
	Result.z = Z;
	Result.w = 1.0f;

	return(Result);
}

inline v4
V4(v3 V, f32 C)
{
	v4 Result = {};

	Result.xyz = V;
	Result.w = C;

	return(Result);
}

inline sqt
SQT(v3 Scale, quaternion Orientation, v3 Position)
{
	sqt Result;

	Result.Scale = Scale;
	Result.Orientation = Orientation;
	Result.Position = Position;
}

inline f32
Lerp(f32 A, f32 t, f32 B)
{
	f32 Result = (1 - t) * A + t * B;
	return(Result);
}

inline v3
V3(f32 C)
{
	v3 Result = V3(C, C, C);
	return(Result);
}

inline v3
XAxis()
{
	v3 Result = V3(1.0f, 0.0f, 0.0f);
	return(Result);
}

inline v3
YAxis()
{
	v3 Result = V3(0.0f, 1.0f, 0.0f);
	return(Result);
}

inline v3
ZAxis()
{
	v3 Result = V3(0.0f, 0.0f, 1.0f);
	return(Result);
}

//
// NOTE(Justin): Scalar operations
//

inline f32
Square(f32 C)
{
	f32 Result = C * C;
	return(Result);
}

inline f32
Clamp(f32 Min, f32 X, f32 Max)
{
	f32 Result = X;

	if(Result < Min)
	{
		Result = Min;
	}
	else if(Result > Max)
	{
		Result = Max;
	}

	return(Result);
}

inline f32
Clamp01(f32 X)
{
	f32 Result = Clamp(0.0f, X, 1.0f);
	return(Result);
}

inline u32
Clamp(u32 Min, u32 X, u32 Max)
{
	u32 Result = X;

	if(Result < Min)
	{
		Result = Min;
	}
	else if(Result > Max)
	{
		Result = Max;
	}

	return(Result);
}

//
// NOTE(Justin): v3 operations
//

inline v3
operator +(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return(Result);
}

inline v3 &
operator +=(v3 &A, v3 B)
{
	A = A + B;
	return(A);
}

inline v3
operator -(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return(Result);
}

inline v3
operator *(f32 C, v3 V)
{
	v3 Result = {};

	Result.x = C * V.x;
	Result.y = C * V.y;
	Result.z = C * V.z;

	return(Result);
}

inline v3
operator *(v3 V, f32 C)
{
	v3 Result = C * V;
	return(Result);
}

inline v3 &
operator *=(v3 &V, f32 C)
{
	V = C * V;
	return(V);
}

inline f32
Dot(v3 A, v3 B)
{
	f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
	return(Result);
}

inline v3
Cross(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.y * B.z - A.z * B.y;
	Result.y = A.z * B.x - A.x * B.z;
	Result.z = A.x * B.y - A.y * B.x;

	return(Result);
}

inline f32
Length(v3 V)
{
	f32 Result = sqrtf(Dot(V, V));
	return(Result);
}

inline v3
Normalize(v3 V)
{
	v3 Result = {};

	f32 Length = sqrtf(Dot(V, V));
	if(Length != 0.0f)
	{
		Result = (1.0f / Length) * V;
	}

	return(Result);
}

inline v3
Lerp(v3 A, f32 t, v3 B)
{
	v3 Result = (1 - t) * A + t * B;
	return(Result);
}

inline b32
Equal(v3 A, v3 B, f32 Tolerance = SmallNumber)
{
	b32 Result = ((AbsVal(A.x - B.x) <= Tolerance) &&
				  (AbsVal(A.y - B.y) <= Tolerance) &&
				  (AbsVal(A.z - B.z) <= Tolerance));

	return(Result);
}

//
// NOTE(Justin): v4 operations
//

inline v4
operator*(f32 C, v4 A)
{
	v4 Result;

	Result.x = C * A.x;
	Result.y = C * A.y;
	Result.z = C * A.z;
	Result.w = C * A.w;

	return(Result);
}

inline v4
operator*(v4 A, f32 C)
{
	v4 Result = C * A;
	return(Result);
}

inline v4
operator+(v4 A, v4 B)
{
	v4 Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;
	Result.w = A.w + B.w;

	return(Result);

}

inline v4
operator-(v4 A, v4 B)
{
	v4 Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;
	Result.w = A.w - B.w;

	return(Result);
}

inline f32
Dot(v4 A, v4 B)
{
	f32 Result = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
	return(Result);
}

inline v4
Cross(v4 A, v4 B)
{
    f32 X = A.y * B.z - A.z * B.y;
    f32 Y = A.z * B.x - A.x * B.z;
    f32 Z = A.x * B.y - A.y * B.x;
    
    v4 Result = V4(X, Y, Z);
    return(Result);
}

inline void
operator+=(v4 &A, v4 &B)
{
	A = A + B;
}

inline mat3
Mat3(v3 X, v3 Y, v3 Z)
{
	mat4 R =
	{
		{{X.x, Y.x, Z.x},
		{X.y, Y.y, Z.y},
		{X.z, Y.z, Z.z}} 
	};
}

inline f32
Trace(mat3 M)
{
	f32 Result = M.E[0][0] + M.E[1][1] + M.E[2][2];
	return(Result);
}

inline mat4
Mat4Identity()
{
	mat4 R =
	{
		{{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}}
	};

	return(R);
}

inline mat4
Mat4(v3 X, v3 Y, v3 Z)
{
	mat4 R =
	{
		{{X.x, Y.x, Z.x, 0},
		{X.y, Y.y, Z.y, 0},
		{X.z, Y.z, Z.z, 0},
		{0, 0, 0, 1}}
	};

	return(R);
}

inline mat4
Mat4(v3 X, v3 Y, v3 Z, v3 W)
{
	mat4 R =
	{
		{{X.x, Y.x, Z.x, W.x},
		{X.y, Y.y, Z.y, W.y},
		{X.z, Y.z, Z.z, W.z},
		{0, 0, 0, 1}}
	};

	return(R);
}

inline mat4
Mat4Scale(f32 C)
{
	mat4 R =
	{
		{{C, 0, 0, 0},
		{0, C, 0, 0},
		{0, 0, C, 0},
		{0, 0, 0, 1}}
	};

	return(R);
}

inline mat4
Mat4Scale(v3 V)
{
	mat4 R =
	{
		{{V.x, 0, 0, 0},
		{0, V.y, 0, 0},
		{0, 0, V.z, 0},
		{0, 0, 0, 1}}
	};

	return(R);
}

inline mat4
Mat4Translate(v3 V)
{
	mat4 R =
	{
		{{1, 0, 0, V.x},
		{0, 1, 0, V.y},
		{0, 0, 1, V.z},
		{0, 0, 0, 1}}
	};

	return(R);
}

inline v3
Mat4ColumnGet(mat4 M, u32 ColIndex)
{
	v3 Result = {};

	if((ColIndex >= 0) && (ColIndex < 4))
	{
		Result.x = M.E[0][ColIndex];
		Result.y = M.E[1][ColIndex];
		Result.z = M.E[2][ColIndex];
	}

	return(Result);
}

inline mat4
Mat4ColumnSet(mat4 M, v4 V, u32 ColIndex)
{
	mat4 Result = M; 

	if((ColIndex >= 0) && (ColIndex < 4))
	{
		Result.E[0][ColIndex] = V.x;
		Result.E[1][ColIndex] = V.y;
		Result.E[2][ColIndex] = V.z;
		Result.E[3][ColIndex] = V.w;
	}

	return(Result);
}

inline v4
Mat4Transform(mat4 T, v4 V)
{
	v4 Result = {};

	Result.x = T.E[0][0] * V.x + T.E[0][1] * V.y + T.E[0][2] * V.z + T.E[0][3] * V.w;
	Result.y = T.E[1][0] * V.x + T.E[1][1] * V.y + T.E[1][2] * V.z + T.E[1][3] * V.w;
	Result.z = T.E[2][0] * V.x + T.E[2][1] * V.y + T.E[2][2] * V.z + T.E[2][3] * V.w;
	Result.w = T.E[3][0] * V.x + T.E[3][1] * V.y + T.E[3][2] * V.z + T.E[3][3] * V.w;

	return(Result);
}

inline mat4
Mat4TransposeMat3(mat4 T)
{
	mat4 R = T;

	for(s32 i = 0; i < 3; ++i)
	{
		for(s32 j = 0; j < 3; ++j)
		{
			if((i != j) && (i < j))
			{
				f32 Temp =  R.E[j][i];
				R.E[j][i] = R.E[i][j];
				R.E[i][j] = Temp;
			}
		}
	}

	return(R);
}

inline mat4
Mat4Transpose(mat4 T)
{
	mat4 R = T;

	for(s32 i = 0; i < 4; ++i)
	{
		for(s32 j = 0; j < 4; ++j)
		{
			if((i != j) && (i < j))
			{
				f32 Temp =  R.E[j][i];
				R.E[j][i] = R.E[i][j];
				R.E[i][j] = Temp;
			}
		}
	}

	return(R);
}

inline v3
operator*(mat4 T, v3 V)
{
	v3 Result = Mat4Transform(T, V4(V, 1.0)).xyz;
	return(Result);
}

inline mat4
Mat4Add(mat4 A, mat4 B)
{
	mat4 R = {};

	for(s32 i = 0; i <= 3; ++i)
	{
		for(s32 j = 0; j <= 3; ++j)
		{
			R.E[i][j] = A.E[i][j] + B.E[i][j];
		}
	}

	return(R);
}

inline mat4
operator+(mat4 A, mat4 B)
{
	mat4 Result = Mat4Add(A, B);
	return(Result);
}

inline mat4
Mat4Multiply(mat4 A, mat4 B)
{
	mat4 R = {};

	for(s32 i = 0; i <= 3; ++i)
	{
		for(s32 j = 0; j <= 3; ++j)
		{
			for(s32 k = 0; k <= 3; ++k)
			{
				R.E[i][j] += A.E[i][k] * B.E[k][j];
			}
		}
	}

	return(R);
}

inline mat4
operator*(mat4 A, mat4 B)
{
	mat4 R = Mat4Multiply(A, B);
	return(R);
}

inline mat4
Mat4YRotation(f32 Angle)
{
	mat4 R = 
	{
		{{-sinf(Angle), 0.0f, cosf(Angle), 0.0f},
		 {0.0f, 1.0f, 0.0f, 0.0f},
		 {cosf(Angle), 0.0f, sinf(Angle), 0.0f},
		 {0.0f, 0.0f, 0.0f, 1.0f}}
	};

	return(R);
}

inline mat4
Mat4XRotation(f32 Angle)
{
	mat4 R = 
	{
		{{1.0f, 0.0f, 0.0f, 0.0f},
		 {0.0f, cosf(Angle), -sinf(Angle), 0.0f},
		 {0.0f, sinf(Angle), cosf(Angle), 0.0f},
		 {0.0f, 0.0f, 0.0f, 1.0f}}
	};

	return(R);
}

inline mat4
ModelTransformFromBasis(basis *B)
{
	mat4 R =
	{
		{{B->X.x, B->Y.x, B->Z.x, B->O.x},
		 {B->X.y, B->Y.y, B->Z.y, B->O.y},
		 {B->X.z, B->Y.z, B->Z.z, B->O.z},
		 {0,0,0,1}}
	};

	return(R);
}

inline mat4
Mat4Camera(v3 P, v3 Target)
{
	mat4 R;

	v3 Z = Normalize(P - Target);
	v3 X = Normalize(Cross(YAxis(), Z));
	v3 Y = Normalize(Cross(Z, X));

	mat4 CameraFrame = Mat4(X, Y, Z);

	mat4 Translate = Mat4Translate(-1.0f * P);

	R = Mat4TransposeMat3(CameraFrame) * Translate;

	return(R);
}

inline mat4
Mat4Perspective(f32 FOV, f32 AspectRatio, f32 ZNear, f32 ZFar)
{
	f32 HalfFOV = FOV / 2.0f;

	mat4 R =
	{
		{{1.0f / (tanf(HalfFOV) * AspectRatio), 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f / tanf(HalfFOV), 0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f * (ZFar + ZNear) / (ZFar - ZNear), -1.0f},
		{0.0f, 0.0f, -1.0f, 0.0f}}
	};

	return(R);
}

inline mat4
Mat4OrthographicProjection(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
	mat4 Result = {};

    Result.E[0][0] = 2.0f / (Right - Left);
    Result.E[1][1] = 2.0f / (Top - Bottom);
    Result.E[2][2] = 2.0f / (Far - Near);
    Result.E[3][0] = (Left + Right) / (Left - Right);
    Result.E[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.E[3][2] = (Far + Near) / (Near - Far);
    Result.E[3][3] = 1.0f;

	return(Result);
}


struct affine_decomposition
{
	v3 P;

	f32 Cx;
	f32 Cy;
	f32 Cz;

	mat4 R;
};

inline affine_decomposition
Mat4AffineDecomposition(mat4 M)
{
	affine_decomposition Result = {};

	Result.P = Mat4ColumnGet(M, 3);

	M = Mat4ColumnSet(M, V4(0.0f, 0.0f, 0.0f, 1.0f), 3);

	v3 X = Mat4ColumnGet(M, 0);
	v3 Y = Mat4ColumnGet(M, 1);
	v3 Z = Mat4ColumnGet(M, 2);

	Result.Cx = Length(X);
	Result.Cy = Length(Y);
	Result.Cz = Length(Z);

	Result.R = Mat4(Normalize(X), Normalize(Y), Normalize(Z), V3(0.0f, 0.0f, 0.0f));

	return(Result);
}

inline quaternion
Quaternion()
{
	quaternion Result = {};
	Result.w = 1.0f;
	return(Result);
}

inline quaternion
Quaternion(v3 Axis, f32 Angle)
{
	quaternion Result;

	Axis = Normalize(Axis);
	Result.x = sinf(0.5f * Angle) * Axis.x;
	Result.y = sinf(0.5f * Angle) * Axis.y;
	Result.z = sinf(0.5f * Angle) * Axis.z;
	Result.w = cosf(0.5f * Angle);

	return(Result);
}

inline quaternion
Conjugate(quaternion Q)
{
	quaternion Result = Q;

	Result.x = -Q.x;
	Result.y = -Q.y;
	Result.z = -Q.z;

	return(Result);
}

inline quaternion
operator+(quaternion Q1, quaternion Q2)
{
	quaternion Result;

	Result.x = Q1.x + Q2.x;
	Result.y = Q1.y + Q2.y;
	Result.z = Q1.z + Q2.z;
	Result.w = Q1.w + Q2.w;

	return(Result);
}

inline void 
operator+=(quaternion &Q1, quaternion Q2)
{
	Q1 = Q1 + Q2;
}

inline quaternion
operator*(f32 C, quaternion Q)
{
	quaternion Result;

	Result.x = C * Q.x;
	Result.y = C * Q.y;
	Result.z = C * Q.z;
	Result.w = C * Q.w;

	return(Result);
}

inline void 
operator*=(quaternion &Q, f32 C)
{
	Q = C * Q;
}

inline quaternion
operator*(quaternion Q1, quaternion Q2)
{
	quaternion Result;

	v4 V1 = V4(Q1.x, Q1.y, Q1.z);
	v4 V2 = V4(Q2.x, Q2.y, Q2.z);

	v4 V = Q1.w * V2 + Q2.w * V1 - Cross(V1, V2);
	f32 C = Q1.w * Q2.w - Dot(V1, V2);

	Result.x = V.x;
	Result.y = V.y;
	Result.z = V.z;
	Result.w = C;

	return(Result);
}

inline void
operator*=(quaternion &Q1, quaternion Q2)
{
	Q1 = Q1 * Q2;
}

inline quaternion
Quaternion(v3 V)
{
	quaternion Result;

	Result.x = V.x;
	Result.y = V.y;
	Result.z = V.z;
	Result.w = 0.0f;

	return(Result);
}

inline v3
Rotate(v3 V, quaternion Q)
{
	v3 Result;

	quaternion VAsQ = Quaternion(V);
	quaternion ResultAsQ = Q * VAsQ * Conjugate(Q);

	Result.x = ResultAsQ.x;
	Result.y = ResultAsQ.y;
	Result.z = ResultAsQ.z;

	return(Result);
}

inline v4
Rotate(v4 V, quaternion Q)
{
	v4 Result;

	quaternion VAsQ;
	VAsQ.x = V.x;
	VAsQ.y = V.y;
	VAsQ.z = V.z;
	VAsQ.w = 1.0f;

	quaternion ResultAsQ = Q * VAsQ * Conjugate(Q);

	Result.x = ResultAsQ.x;
	Result.y = ResultAsQ.y;
	Result.z = ResultAsQ.z;
	Result.w = V.w;

	return(Result);
}

inline mat4
QuaternionToMat4(quaternion Q)
{
	mat4 Result = Mat4Identity();

	f32 XSquared = Square(Q.x);
	f32 YSquared = Square(Q.y);
	f32 ZSquared = Square(Q.z);

	f32 XY = Q.x * Q.y;
	f32 XZ = Q.x * Q.z;
	f32 XW = Q.x * Q.w;
	f32 YZ = Q.y * Q.z;
	f32 YW = Q.y * Q.w;
	f32 ZW = Q.z * Q.w;

	Result.E[0][0] = 1.0f - 2.0f * YSquared - 2.0f * ZSquared;
	Result.E[0][1] = 2.0f * XY + 2.0f * ZW;
	Result.E[0][2] = 2.0f * XZ - 2.0f * YW;

	Result.E[1][0] = 2.0f * XY - 2.0f * ZW;
	Result.E[1][1] = 1.0f - 2.0f * XSquared - 2.0f * ZSquared;
	Result.E[1][2] = 2.0f * YZ + 2.0f * XW;

	Result.E[2][0] = 2.0f * XZ + 2.0f * YW;
	Result.E[2][1] = 2.0f * YZ - 2.0f * XW;
	Result.E[2][2] = 1.0f - 2.0f * XSquared - 2.0f * YSquared;

	return(Result);
}

inline quaternion
Normalize(quaternion Q)
{
	f32 Length = sqrtf(Square(Q.x) + Square(Q.y) + Square(Q.z) + Square(Q.w));

	Q.x /= Length;
	Q.y /= Length;
	Q.z /= Length;
	Q.w /= Length;

	return(Q);
}

inline quaternion
NormalizeOrIdentity(quaternion Q)
{
	quaternion Result;

	f32 Length = sqrtf(Square(Q.x) + Square(Q.y) + Square(Q.z) + Square(Q.w));
	if(Length > 0.0f)
	{
		Result = (1.0f / Length) * Q;
	}
	else
	{
		Result = Quaternion();
	}

	return(Result);
}

inline quaternion
RotationToQuaternion(mat4 Mat)
{
    quaternion Quat = {};
    
    float  tr, s, q[4];
    int    i, j, k;
    int nxt[3] = {1, 2, 0};
    tr = Mat.E[0][0] + Mat.E[1][1] + Mat.E[2][2];
    // check the diagonal
    if (tr > 0.0) {
        s = (f32)sqrtf(tr + 1.0f);
        Quat.w = s / 2.0f;
        s = 0.5f / s;
        Quat.x = (Mat.E[1][2] - Mat.E[2][1]) * s;
        Quat.y = (Mat.E[2][0] - Mat.E[0][2]) * s;
        Quat.z = (Mat.E[0][1] - Mat.E[1][0]) * s;
    } else {		
        // diagonal is negative
        i = 0;
        if (Mat.E[1][1] > Mat.E[0][0]) i = 1;
        if (Mat.E[2][2] > Mat.E[i][i]) i = 2;
        j = nxt[i];
        k = nxt[j];
        s = (f32)sqrtf((Mat.E[i][i] - (Mat.E[j][j] + Mat.E[k][k])) + 1.0f);
        q[i] = s * 0.5f;
        if (s != 0.0) s = 0.5f / s;
        q[3] = (Mat.E[j][k] - Mat.E[k][j]) * s;
        q[j] = (Mat.E[i][j] + Mat.E[j][i]) * s;
        q[k] = (Mat.E[i][k] + Mat.E[k][i]) * s;
        Quat.x = q[0];
        Quat.y = q[1];
        Quat.z = q[2];
        Quat.w = q[3];
    }
    
    return Quat;
}

inline f32
Dot(quaternion Q1, quaternion Q2)
{
	f32 Result = (Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z + Q1.w * Q2.w);
	return(Result);
}


inline quaternion
Lerp(quaternion A, f32 t, quaternion B)
{
	quaternion Result;

	Result.x = (1.0f - t) * A.x + t * B.x;
	Result.y = (1.0f - t) * A.y + t * B.y;
	Result.z = (1.0f - t) * A.z + t * B.z;
	Result.w = (1.0f - t) * A.w + t * B.w;

	Result = Normalize(Result);

	return(Result);
}

inline quaternion
LerpShortest(quaternion A, f32 t, quaternion B)
{
	if(Dot(A, B) <= 0.0f)
	{
		A = -1.0f * A;
	}

	quaternion Result = Lerp(A, t, B);
	return(Result);
}

inline v3
DirectionToEuler(v3 V)
{
	v3 Result;

    Result.pitch = ATan2(V.yaw, Sqrt(V.roll*V.roll + V.pitch*V.pitch));
    Result.yaw   = ATan2(V.pitch, V.roll);
    Result.roll  = 0.0f;

	return(Result);

}

// NOTE(Justin): Angular distance between two quaternions
inline f32
AngleBetween(quaternion A, quaternion B)
{
	f32 Inner = Dot(A, B);
	f32 Theta = Clamp(-1.0f, ((2.0f * Inner * Inner) - 1.0f), 1.0f);
	f32 Result = ACos(Theta);

	return(Result);
}

inline b32
Equal(quaternion A, quaternion B, f32 Tolerance = SmallNumber)
{
	b32 Result = ((AbsVal(A.x - B.x) <= Tolerance) &&
				  (AbsVal(A.y - B.y) <= Tolerance) &&
				  (AbsVal(A.z - B.z) <= Tolerance) &&
				  (AbsVal(A.w - B.w) <= Tolerance));

	return(Result);
}

inline quaternion
RotateTowards(quaternion Current, quaternion Target, f32 dt, f32 AngularSpeed)
{
	if(AngularSpeed <= 0.0f)
	{
		return(Target);
	}

	if(Equal(Current, Target))
	{
		return(Target);
	}

	f32 dSpeed = Clamp01(dt * AngularSpeed);
	f32 dTheta = Max(AngleBetween(Current, Target), SmallNumber);
	f32 tAlpha = Clamp01(dSpeed / dTheta);
	
	// TODO(Justin): Slerp?
	quaternion Result = LerpShortest(Current, tAlpha, Target);

	return(Result);
}


#define MATH_UTIL_H 
#endif