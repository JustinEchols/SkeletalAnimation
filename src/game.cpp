
#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "mesh.cpp"
#include "animation.cpp"
#include "asset.cpp"
#include "opengl.cpp"
#include "font.cpp"

internal entity * 
EntityAdd(game_state *GameState, entity_type Type)
{
	Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
	entity *Entity = GameState->Entities + GameState->EntityCount++;
	Entity->Type = Type;
	return(Entity);
}

internal void
PlayerAdd(game_state *GameState)
{
	entity *Entity = EntityAdd(GameState, EntityType_Player);
	GameState->PlayerEntityIndex = GameState->EntityCount - 1;
	Entity->P = V3(0.0f, 0.0f, -10.0f);
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Theta = -180.0f;
	Entity->dTheta = 0.0f;
	Entity->Orientation = Quaternion(V3(0.0f, 1.0f, 0.0f), Entity->Theta);
	Entity->AnimationState = AnimationState_Idle;
}

internal void
CubeAdd(game_state *GameState, v3 P)
{
	entity *Entity = EntityAdd(GameState, EntityType_Cube);
	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Orientation = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);
}

inline mat4
EntityTransform(entity *Entity, f32 Scale = 1.0f)
{
	mat4 Result = Mat4Identity();

	mat4 R = QuaternionToMat4(Entity->Orientation);
	Result = Mat4(Scale * Mat4ColumnGet(R, 0),
				  Scale * Mat4ColumnGet(R, 1),
				  Scale * Mat4ColumnGet(R, 2),
				  Entity->P);

	return(Result);
}

#if 1
inline void
EntityOrientationUpdate(entity *Entity, f32 dt, f32 AngularSpeed)
{
	quaternion Orientation = Entity->Orientation;
	v3 FacingDirection = Entity->dP;
	FacingDirection.z *= -1.0f;
	f32 Yaw = DirectionToEuler(-1.0f * FacingDirection).yaw;
	quaternion Target = Quaternion(V3(0.0f, 1.0f, 0.0f), Yaw);
	Entity->Orientation = RotateTowards(Orientation, Target, dt, AngularSpeed);
}
#else
#endif

inline void
OrientationUpdate(quaternion *Orientation, v3 FacingDirection, f32 dt, f32 AngularSpeed)
{
	//quaternion Orientation = Entity->Orientation;
	//v3 FacingDirection = Entity->dP;
	FacingDirection.z *= -1.0f;
	f32 Yaw = DirectionToEuler(-1.0f * FacingDirection).yaw;
	quaternion Target = Quaternion(V3(0.0f, 1.0f, 0.0f), Yaw);
	*Orientation = RotateTowards(*Orientation, Target, dt, AngularSpeed);
}

internal quad
QuadDefault(void)
{
	quad Result = {};

	v3 N = V3(0.0f, 1.0f, 0.0f);

	Result.Vertices[0].P = V3(-0.5f, 0.0f, 0.5f);
	Result.Vertices[1].P = V3(0.5f,	 0.0f, 0.5f);
	Result.Vertices[2].P = V3(0.5f,	 0.0f, -0.5f);
	Result.Vertices[3].P = V3(0.5f,	 0.0f, -0.5f);
	Result.Vertices[4].P = V3(-0.5f, 0.0f, -0.5f);
	Result.Vertices[5].P = V3(-0.5f, 0.0f, 0.5f);

	Result.Vertices[0].N = N;
	Result.Vertices[1].N = N;
	Result.Vertices[2].N = N;
	Result.Vertices[3].N = N;
	Result.Vertices[4].N = N;
	Result.Vertices[5].N = N;

	Result.Vertices[0].UV = V2(0.0f);
	Result.Vertices[1].UV = V2(1.0f, 0.0f);
	Result.Vertices[2].UV = V2(1.0f, 1.0f);
	Result.Vertices[3].UV = V2(1.0f, 1.0f);
	Result.Vertices[4].UV = V2(0.0f, 1.0f);
	Result.Vertices[5].UV = V2(0.0f);

	return(Result);
}

internal void
CameraSet(camera *Camera, v3 P, f32 Yaw, f32 Pitch)
{
	Camera->P = P;
	Camera->Direction.x = Cos(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
}

internal void
CameraTransformUpdate(game_state *GameState)
{
	GameState->CameraTransform = Mat4Camera(GameState->Camera.P,
											GameState->Camera.P + GameState->Camera.Direction);
}

internal void
PerspectiveTransformUpdate(game_state *GameState)
{
	// TODO(Justin): Probably dont want to hide the fact that we are re-calculating aspect here..
	GameState->Aspect = (f32)Win32GlobalWindowWidth / (f32)Win32GlobalWindowHeight;
	GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);
}

internal void
GameUpdateAndRender(game_memory *GameMemory, game_input *GameInput)
{

	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	if(!GameMemory->IsInitialized)
	{

		FontInit(&GameState->FontQuad.Info, "c:/windows/fonts/arial.ttf");

		ArenaInitialize(&GameState->Arena, (u8 *)GameMemory->PermanentStorage + sizeof(game_state),
												 GameMemory->PermanentStorageSize - sizeof(game_state)); 

		ArenaInitialize(&GameState->TempArena, (u8 *)GameMemory->TemporaryStorage,
													 GameMemory->TemporaryStorageSize);

		memory_arena *Arena = &GameState->Arena;

		GameState->Textures[0] = TextureLoad("..\\data\\textures\\tile_gray.bmp");
		GameState->Textures[1] = TextureLoad("..\\data\\textures\\left_arrow.png");
		OpenGLAllocateTexture(&GameState->Textures[0]);
		OpenGLAllocateTexture(&GameState->Textures[1]);

		GameState->XBot = PushStruct(Arena, model);
		*GameState->XBot = ModelLoad(Arena, "..\\data\\XBot.mesh");

		GameState->Cube = PushStruct(Arena, model);
		*GameState->Cube = ModelLoad(Arena, "..\\data\\Cube.mesh");
		GameState->Cube->Meshes[0].TextureHandle = GameState->Textures[0].Handle;

		GameState->Sphere = PushStruct(Arena, model);
		*GameState->Sphere = ModelLoad(Arena, "..\\data\\Sphere.mesh");

		GameState->Arrow = PushStruct(Arena, model);
		*GameState->Arrow = ModelLoad(Arena, "..\\data\\Arrow.mesh");

		GameState->Quad = QuadDefault();
		GameState->Quad.Texture = GameState->Textures[0].Handle;

		PlayerAdd(GameState);

		RandInit(2024);
		for(u32 Index = 0; Index < 10; ++Index)
		{
			v3 P = V3(RandBetween(-100.0f, 100.0f), RandBetween(1.0f, 2.0f), RandBetween(-10.0f, -100.0f));
			CubeAdd(GameState, P);
		}

		model *Model = GameState->XBot;
		AnimationPlayerInitialize(&GameState->AnimationPlayer, Model, Arena);
		GameState->AnimationInfos = PushArray(Arena, ArrayCount(AnimationFiles), animation_info);
		GameState->Animations = PushArray(Arena, ArrayCount(AnimationFiles), animation);
		for(u32 AnimIndex = 0; AnimIndex < ArrayCount(AnimationFiles); ++AnimIndex)
		{
			GameState->AnimationInfos[AnimIndex] = AnimationLoad(Arena, AnimationFiles[AnimIndex]);

			animation_info *Info = GameState->AnimationInfos + AnimIndex;
			animation *Animation = GameState->Animations + AnimIndex;

			if(Info)
			{
				Animation->Name = AnimationFiles[AnimIndex];
				Animation->ID.Value = AnimIndex;
				Animation->Info = Info;
				Animation->BlendedPose = PushStruct(Arena, key_frame);
				key_frame *BlendedPose = Animation->BlendedPose;
				AllocateJointXforms(Arena, BlendedPose, Info->JointCount);
			}

			switch(AnimIndex)
			{
				case Animation_Idle:
				{
					Animation->DefaultFlags = AnimationFlags_Looping;
				} break;
				case Animation_Run:
				{
					Animation->DefaultFlags = AnimationFlags_Looping |
											  AnimationFlags_RemoveLocomotion;
				} break;
				case Animation_Sprint:
				{
					Animation->DefaultFlags = AnimationFlags_Looping |
											  AnimationFlags_RemoveLocomotion;
				} break;
				case Animation_JumpForward:
				{
					Animation->DefaultFlags = AnimationFlags_RemoveLocomotion |
											  AnimationFlags_MustFinish;
				} break;
			}
		}

		CameraSet(&GameState->Camera, V3(0.0f, 15.0f, 20.0f), -90.0f, -10.0f);
		CameraTransformUpdate(GameState);
		GameState->CameraOffsetFromPlayer = V3(0.0f, 5.0f, 10.0f);

		GameState->TimeScale = 1.0f;
		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)Win32GlobalWindowWidth / (f32)Win32GlobalWindowHeight;
		GameState->ZNear = 0.1f;
		GameState->ZFar = 100.0f;
		GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);

		GameState->Shaders[0] = GLProgramCreate(MainVS, MainFS);
		GameState->Shaders[1] = GLProgramCreate(BasicVS, BasicFS);
		GameState->Shaders[2] = GLProgramCreate(FontVS, FontFS);

		u32 MainShader = GameState->Shaders[0];
		u32 BasicShader = GameState->Shaders[1];
		u32 FontShader = GameState->Shaders[2];

		OpenGLAllocateAnimatedModel(GameState->XBot, MainShader);
		OpenGLAllocateModel(GameState->Cube, BasicShader);
		OpenGLAllocateModel(GameState->Sphere, BasicShader);
		OpenGLAllocateModel(GameState->Arrow, BasicShader);
		OpenGLAllocateQuad(&GameState->Quad, BasicShader);
		OpenGLAllocateFontQuad(&GameState->FontQuad, FontShader);

#if 0
		u32 FBO;
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);


		u32 FrameBufferTexture;
		glGenTextures(1, &FrameBufferTexture);
		glBindTexture(GL_TEXTURE_2D, FrameBufferTexture);
		s32 TextureWidth = 256;
		s32 TextureHeight = 256;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA8,
				TextureWidth,
				TextureHeight,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				0);

		u32 RenderBO;
		glGenRenderbuffers(1, &RenderBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RenderBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, TextureWidth, TextureHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebuggerRenderbugger(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBO);

		if(glCheckFramebuggerStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			PrintString("ERROR: Framebugger is not complete");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

#endif


		GameMemory->IsInitialized = true;
	}

	v3 ddP = {};
	game_keyboard *Keyboard = &GameInput->Keyboard;
	if(Keyboard->W.EndedDown)
	{
		ddP += V3(0.0f, 0.0f, -1.0f);
	}
	if(Keyboard->A.EndedDown)
	{
		ddP += V3(-1.0f, 0.0f, 0.0f);
	}
	if(Keyboard->S.EndedDown)
	{
		ddP += V3(0.0f, 0.0f, 1.0f);
	}
	if(Keyboard->D.EndedDown)
	{
		ddP += V3(1.0f, 0.0f, 0.0f);
	}

	b32 Sprinting = false;
	if(Keyboard->Shift.EndedDown)
	{
		Sprinting = true;
	}
	
	b32 Jumping = false;
	if(Keyboard->Space.EndedDown)
	{
		Jumping = true;
	}

	if(Keyboard->Add.EndedDown)
	{
		GameState->TimeScale *= 1.1f;
	}

	if(Keyboard->Subtract.EndedDown)
	{
		GameState->TimeScale *= 0.9f;
	}


	if(!Equal(ddP, V3(0.0f)))
	{
		ddP = Normalize(ddP);
	}
	else
	{
	}

	f32 dt = GameInput->DtForFrame;
	dt *= GameState->TimeScale;
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				v3 OldPlayerddP = Entity->ddP;
				Entity->ddP = ddP;

				f32 PlayerSpeed = 100.0f;
				//f32 PlayerSpeed = 50.0f;
				if(Sprinting)
				{
					PlayerSpeed *= 1.5f;
				}

				f32 PlayerDrag = -9.0f;
				f32 AngularSpeed = 10.0f;
				ddP = PlayerSpeed * ddP;
				ddP += PlayerDrag * Entity->dP;

				v3 OldP = Entity->P;
				v3 dP = Entity->dP + dt * ddP;
				v3 P =  Entity->P + 0.5f * dt * dt * ddP + dt * Entity->dP;

				Entity->P = P;
				Entity->dP = dP;

				EntityOrientationUpdate(Entity, dt, AngularSpeed);

				switch(Entity->AnimationState)
				{
					case AnimationState_Idle:
					{
						if(Equal(OldPlayerddP, V3(0.0f)) &&
						  !Equal(Entity->ddP, OldPlayerddP))
						{
							if(Sprinting)
							{
								Entity->AnimationState = AnimationState_Sprint;
							}
							else
							{
								Entity->AnimationState = AnimationState_Running;
							}
						}
					} break;
					case AnimationState_Running:
					{
						if(Equal(Entity->ddP, V3(0.0f)))
						{
							Entity->AnimationState = AnimationState_Idle;
						}
						else
						{
							if(Sprinting)
							{
								Entity->AnimationState = AnimationState_Sprint;
							}

							if(Jumping)
							{
								Entity->AnimationState = AnimationState_JumpForward;
							}
						}

					} break;
					case AnimationState_Sprint:
					{
						if(Equal(Entity->ddP, V3(0.0f)))
						{
							Entity->AnimationState = AnimationState_Idle;
						}
						else
						{
							if(!Sprinting)
							{
								Entity->AnimationState = AnimationState_Running;
							}
						}
					} break;
					case AnimationState_JumpForward:
					{
						if(!Jumping)
						{
							if(Equal(Entity->ddP, V3(0.0f)))
							{
								Entity->AnimationState = AnimationState_Idle;
							}
							else if(Sprinting)
							{
								Entity->AnimationState = AnimationState_Sprint;
							}
							else
							{
								Entity->AnimationState = AnimationState_Running;
							}
						}
					} break;
					case AnimationState_Invalid:
					{
						// NOTE(Justin): AnimationState is initalized to invalid.
						// First time this is hit, we set the state to idle.
						Entity->AnimationState = AnimationState_Idle;
					} break;

				};
			} break;
		}
	}

	entity *CameraFollowingEntity = GameState->Entities + GameState->PlayerEntityIndex;
	camera *Camera = &GameState->Camera;
	Camera->P = CameraFollowingEntity->P + GameState->CameraOffsetFromPlayer;



	//
	// NOTE(Justin): Render.
	//

	//
	// TODO(Justin): Shadow Pass.
	//

	v3 LightDir = V3(1.0f, -1.0f, 0.0f);

	//
	// NOTE(Justin): Final pass.
	//
	
	PerspectiveTransformUpdate(GameState);
	CameraTransformUpdate(GameState);

	glViewport(0, 0, (u32)Win32GlobalWindowWidth, (u32)Win32GlobalWindowHeight);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	u32 MainShader = GameState->Shaders[0];
	u32 BasicShader = GameState->Shaders[1];
	u32 FontShader = GameState->Shaders[2];

	//
	// NOTE(Justin): Ground quad.
	//

	glUseProgram(BasicShader);
	mat4 T = Mat4Translate(V3(0.0f, 0.0f, -500.0f));
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(1000.0f);

	UniformMatrixSet(BasicShader, "View", GameState->CameraTransform);
	UniformMatrixSet(BasicShader, "Projection", GameState->Perspective);
	UniformBoolSet(BasicShader, "OverRideTexture", false);
	UniformV3Set(BasicShader, "Ambient", V3(0.1f));
	UniformV3Set(BasicShader, "LightDir", LightDir);
	OpenGLDrawQuad(&GameState->Quad, BasicShader, T*R*S, GameState->Textures[0].Handle);
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				animation_player *AnimationPlayer = &GameState->AnimationPlayer;
				Animate(GameState, AnimationPlayer, Entity->AnimationState);
				AnimationPlayerUpdate(AnimationPlayer, &GameState->TempArena, dt);
				ModelJointsUpdate(AnimationPlayer);

				mat4 Transform = EntityTransform(Entity, 0.025f);
				model *Model = GameState->XBot;

				glUseProgram(MainShader);
				UniformMatrixSet(MainShader, "View", GameState->CameraTransform);
				UniformMatrixSet(MainShader, "Projection", GameState->Perspective);
				UniformV3Set(MainShader, "CameraP", Camera->P);
				UniformV3Set(MainShader, "Ambient", V3(0.1f));
				UniformV3Set(MainShader, "CameraP", Camera->P);
				UniformV3Set(MainShader, "LightDir", LightDir);
				OpenGLDrawAnimatedModel(Model, MainShader, Transform);

				//
				// NOTE(Justin): Debug orientation arrow 
				//

				v3 P = Entity->P;
				P.y += 0.25f;
				T = Mat4Translate(P);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(V3(1.0f));

				glUseProgram(BasicShader);
				UniformMatrixSet(BasicShader, "View", GameState->CameraTransform);
				UniformMatrixSet(BasicShader, "Projection", GameState->Perspective);
				UniformBoolSet(BasicShader, "OverRideTexture", true);
				UniformV3Set(BasicShader, "Ambient", V3(0.1f));
				UniformV3Set(BasicShader, "LightDir", LightDir);
				UniformV4Set(BasicShader, "Color", V4(1.0f, 0.0f, 0.0f));
				OpenGLDrawQuad(&GameState->Quad, BasicShader, T*R*S, GameState->Textures[1].Handle);

				//
				// NOTE(Justin): Debug sphere 
				//

				Transform = EntityTransform(Entity, 0.5f);
				UniformV4Set(BasicShader, "Color", V4(1.0f));
				//OpenGLDrawModel(GameState->Sphere, BasicShader, Transform);

			} break;
			case EntityType_Cube:
			{
				glUseProgram(BasicShader);
				UniformMatrixSet(BasicShader, "View", GameState->CameraTransform);
				UniformMatrixSet(BasicShader, "Projection", GameState->Perspective);
				UniformBoolSet(BasicShader, "OverRideTexture", true);
				UniformV3Set(BasicShader, "Ambient", V3(1.0f));
				UniformV3Set(BasicShader, "LightDir", LightDir);
				UniformV4Set(BasicShader, "Color", V4(0.5f));
				model *Cube = GameState->Cube;
				mat4 Transform = EntityTransform(Entity, 1.0f);
				OpenGLDrawModel(Cube, BasicShader, Transform);
			} break;
		};
	}

	//
	// NOTE(Justin): Test font/ui.
	//

	entity *Entity = GameState->Entities + GameState->PlayerEntityIndex;

	glUseProgram(FontShader);

	char Buff[64];
	sprintf(Buff, "%s %f", "time scale: ", GameState->TimeScale);
	OpenGLDrawText(Buff, FontShader, &GameState->FontQuad, V2(0.0f, (f32)Win32GlobalWindowHeight - 20.0f), 0.35f, V3(1.0f), Win32GlobalWindowWidth, Win32GlobalWindowHeight);

	MemoryZero(Buff, sizeof(Buff));
	sprintf(Buff, "%s %f %f %f", "p: ", Entity->P.x, Entity->P.y, Entity->P.z);
	OpenGLDrawText(Buff, FontShader, &GameState->FontQuad, V2(0.0f, (f32)Win32GlobalWindowHeight - 40.0f), 0.35f, V3(1.0f), Win32GlobalWindowWidth, Win32GlobalWindowHeight);

	MemoryZero(Buff, sizeof(Buff));
	f32 Angle = DirectionToEuler(-1.0f * Entity->dP).yaw;
	sprintf(Buff, "%s %f", "yaw: ", Angle);
	OpenGLDrawText(Buff, FontShader, &GameState->FontQuad, V2(0.0f, (f32)Win32GlobalWindowHeight - 60.0f), 0.35f, V3(1.0f), Win32GlobalWindowWidth, Win32GlobalWindowHeight);

	MemoryZero(Buff, sizeof(Buff));
	f32 Speed = Length(Entity->dP);
	sprintf(Buff, "%s %f", "speed: ", Speed);
	OpenGLDrawText(Buff, FontShader, &GameState->FontQuad, V2(0.0f, (f32)Win32GlobalWindowHeight - 80.0f), 0.35f, V3(1.0f), Win32GlobalWindowWidth, Win32GlobalWindowHeight);

	ArenaClear(&GameState->TempArena);
}
