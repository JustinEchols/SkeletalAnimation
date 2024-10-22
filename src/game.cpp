
#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "mesh.cpp"
#include "animation.cpp"
#include "asset.cpp"
#include "opengl.cpp"

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
	Entity->Orientation = Quaternion(V3(0.0f, 1.0f, 0.0f), 0.0f);
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
GameUpdateAndRender(game_memory *GameMemory, game_input *GameInput)
{
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	if(!GameMemory->IsInitialized)
	{
		ArenaInitialize(&GameState->Arena, (u8 *)GameMemory->PermanentStorage + sizeof(game_state),
												 GameMemory->PermanentStorageSize - sizeof(game_state)); 

		ArenaInitialize(&GameState->TempArena, (u8 *)GameMemory->TemporaryStorage,
													 GameMemory->TemporaryStorageSize);

		memory_arena *Arena = &GameState->Arena;

		GameState->Textures[0] = TextureLoad("..\\data\\textures\\tile_gray.bmp");
		GameState->Textures[1] = TextureLoad("..\\data\\textures\\left-arrow.png");
		OpenGLAllocateTexture(&GameState->Textures[0]);
		OpenGLAllocateTexture(&GameState->Textures[1]);

		//GameState->Font = FontInit(&GameState->Arena, "c:/Windows/Fonts/arial.ttf");

		GameState->XBot = PushStruct(Arena, model);
		*GameState->XBot = ModelLoad(Arena, "..\\data\\XBot.mesh");

		GameState->Cube = PushStruct(Arena, model);
		*GameState->Cube = ModelLoad(Arena, "..\\data\\Cube.mesh");
		GameState->Cube->Meshes[0].TextureHandle = GameState->Textures[0].Handle;

		GameState->Quad = QuadDefault();
		GameState->Quad.Texture = GameState->Textures[0].Handle;

		PlayerAdd(GameState);

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
				Animation->ID.Value = AnimIndex;
				Animation->Info = Info;
			}
		}

		GameState->CameraP = V3(0.0f, 15.0f, 20.0f);
		f32 Yaw = -90.0f;
		f32 Pitch = -10.0f;
		GameState->CameraDirection.x = Cos(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
		GameState->CameraDirection.y = Sin(DegreeToRad(Pitch));
		GameState->CameraDirection.z = Sin(DegreeToRad(Yaw)) * Cos(DegreeToRad(Pitch));
		
		GameState->CameraTransform = Mat4Camera(GameState->CameraP,
												GameState->CameraP + GameState->CameraDirection);

		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)Win32GlobalWindowWidth / Win32GlobalWindowHeight;
		GameState->ZNear = 0.1f;
		GameState->ZFar = 100.0f;
		GameState->PerspectiveTransform = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);

		GameState->Shaders[0] = GLProgramCreate(MainVS, MainFS);
		GameState->Shaders[1] = GLProgramCreate(QuadVS, QuadFS);
		u32 Shader = GameState->Shaders[0];
		u32 Shader1 = GameState->Shaders[1];

		OpenGLAllocateAnimatedModel(GameState->XBot, Shader);
		OpenGLAllocateModel(GameState->Cube, Shader1);
		OpenGLAllocateQuad(&GameState->Quad, Shader1);

#if 0
		u32 ShadowMapFBO;
		glGenFrameBuffers(1, &ShadowMapFBO);
		u32 ShadowWidth = 1024;
		u32 ShadowHeight = 1024;

		u32 ShadowMap;
		glGenTextures(1, &ShadowMap);
		glBindTexture(GL_TEXTURE_2D, ShadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowWidth, ShadowHeight, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFrameBuffer(GL_FRAMEBUFFER, ShadowMapFBO);
		glFrameBufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFrameBuffer(GL_FRAMEBUFFER, 0);
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

	if(!Equal(ddP, V3(0.0f)))
	{
		ddP = Normalize(ddP);
	}
	else
	{
	}

	f32 dt = GameInput->DtForFrame;

	entity *CameraFollowingEntity = GameState->Entities + GameState->PlayerEntityIndex;
	v3 Offset = V3(0.0f, 10.0f, 20.0f);
	v3 *CameraP = &GameState->CameraP;
	*CameraP = CameraFollowingEntity->P + Offset;

	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				u32 Flags = 0;
				if(!Equal(ddP, V3(0.0f)))
				{
					ddP = 10.0f * ddP;
					Entity->ddP = ddP;

					v3 OldP = Entity->P;

					v3 dP = Entity->dP + dt * ddP;
					v3 P =  Entity->P + 0.5f * dt * dt * ddP + dt * Entity->dP;

					Entity->P = P;
					Entity->dP = dP;

					quaternion Orientation = Entity->Orientation;
					v3 FacingDirection = Entity->dP;
					FacingDirection.z *= -1.0f;
					f32 Yaw = DirectionToEuler(-1.0f * FacingDirection).yaw;
					quaternion Target = Quaternion(V3(0.0f, 1.0f, 0.0f), Yaw);
					Entity->Orientation = RotateTowards(Orientation, Target, dt, 10.0f);

					Flags = AnimationFlags_RemoveLocomotion;
					AnimationPlay(&GameState->AnimationPlayer, &GameState->Animations[2], Flags);
				}
				else
				{
					Entity->dP = V3(0.0f);
					if(Keyboard->Space.EndedDown)
					{
						AnimationPlay(&GameState->AnimationPlayer, &GameState->Animations[4], Flags);
					}
					else
					{
						Flags = AnimationFlags_Looping;
						AnimationPlay(&GameState->AnimationPlayer, &GameState->Animations[0], Flags);
					}
				}
			}
		};
	}

	//
	// NOTE(Justin): Render.
	//
	
	GameState->CameraTransform = Mat4Camera(GameState->CameraP,
											GameState->CameraP + GameState->CameraDirection);

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

	GameState->Angle += dt;
	f32 Angle = GameState->Angle;
	v3 LightDir = V3(0.0f, -1.0f, 0.0f);

	//
	// NOTE(Justin): Ground quad.
	//

	u32 Shader1 = GameState->Shaders[1];
	glUseProgram(Shader1);
	mat4 T = Mat4Translate(V3(0.0f, 0.0f, -500.0f));
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(1000.0f);

	UniformMatrixSet(Shader1, "View", GameState->CameraTransform);
	UniformMatrixSet(Shader1, "Projection", GameState->PerspectiveTransform);
	UniformBoolSet(Shader1, "OverRideTexture", false);
	UniformV3Set(Shader1, "Ambient", V3(0.1f));
	UniformV3Set(Shader1, "LightDir", LightDir);
	OpenGLDrawQuad(&GameState->Quad, Shader1, T*R*S, GameState->Textures[0].Handle);
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				u32 Shader = GameState->Shaders[0];
				glUseProgram(Shader);
				UniformMatrixSet(Shader, "View", GameState->CameraTransform);
				UniformMatrixSet(Shader, "Projection", GameState->PerspectiveTransform);
				UniformV3Set(Shader, "CameraP", GameState->CameraP);
				UniformV3Set(Shader, "Ambient", V3(0.1f));
				UniformV3Set(Shader, "CameraP", GameState->CameraP);
				UniformV3Set(Shader, "LightDir", LightDir);

				AnimationPlayerUpdate(&GameState->AnimationPlayer, &GameState->TempArena, dt);
				ModelUpdate(&GameState->AnimationPlayer);

				mat4 Transform = EntityTransform(Entity, 0.05f);
				model *Model = GameState->XBot;
				OpenGLDrawAnimatedModel(Model, Shader, Transform);

				//
				// NOTE(Justin): Debug orientation arrow 
				//

				glUseProgram(Shader1);
				v3 P = Entity->P;
				P.y += 0.25f;
				T = Mat4Translate(P);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(V3(1.0f));

				UniformMatrixSet(Shader1, "View", GameState->CameraTransform);
				UniformMatrixSet(Shader1, "Projection", GameState->PerspectiveTransform);
				UniformBoolSet(Shader1, "OverRideTexture", true);
				UniformV3Set(Shader1, "Ambient", V3(0.1f));
				UniformV3Set(Shader1, "LightDir", LightDir);
				UniformV4Set(Shader1, "Color", V4(1.0f, 0.0f, 0.0f));
				OpenGLDrawQuad(&GameState->Quad, Shader1, T*R*S, GameState->Textures[1].Handle);

			} break;
			case EntityType_Cube:
			{
				u32 Shader = GameState->Shaders[1];

				glUseProgram(Shader);
				UniformMatrixSet(Shader, "View", GameState->CameraTransform);
				UniformMatrixSet(Shader, "Projection", GameState->PerspectiveTransform);
				UniformV3Set(Shader, "CameraP", GameState->CameraP);

				UniformV3Set(Shader, "Ambient", V3(0.1f));
				UniformV3Set(Shader, "LightDir", LightDir);
				model *Cube = GameState->Cube;
				mat4 Transform = EntityTransform(Entity);
				OpenGLDrawModel(Cube, Shader, Transform);
			} break;
		};
	}
}
