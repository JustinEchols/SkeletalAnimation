#if !defined(GAME_H)


struct game_state
{
	memory_arena Arena;
	memory_arena TempArena;

	model *Model;

	animation_player AnimationPlayer;
	animation_info *AnimationInfos;
	animation *Animations;

	v3 CameraP;
	v3 Direction;
	mat4 CameraTransform;

	f32 FOV;// = DegreeToRad(45.0f);
	f32 Aspect;// = (f32)Win32GlobalWindowWidth / Win32GlobalWindowHeight;
	f32 ZNear;// = 0.1f;
	f32 ZFar;// = 100.0f;
	mat4 PerspectiveTransform;// = Mat4Perspective(FOV, Aspect, ZNear, ZFar);

	f32 Angle;

	u32 Shaders[2];
};

#define GAME_H
#endif
