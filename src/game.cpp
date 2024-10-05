
#include "game.h"
#include "strings.cpp"
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
	Entity->P = V3(0.0f, -80.0f, -400.0f);
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Orientation = Quaternion();
}

internal void
CubeAdd(game_state *GameState, v3 P)
{
	entity *Entity = EntityAdd(GameState, EntityType_Cube);
	Entity->P = P;
	Entity->dP = V3(0.0f);
	Entity->ddP = V3(0.0f);
	Entity->Orientation = Quaternion();
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
		GameState->XBot = PushStruct(Arena, model);
		GameState->Cube = PushStruct(Arena, model);
		*GameState->XBot = ModelLoad(Arena, "..\\data\\XBot.mesh");
		*GameState->Cube = ModelLoad(Arena, "..\\data\\Cube.mesh");

		PlayerAdd(GameState);
		CubeAdd(GameState, V3(-50.0f, 0.0f, -100.0f));

		model *Model = GameState->XBot;
		Model->Basis.X = XAxis();
		Model->Basis.Y = YAxis();
		Model->Basis.Z = ZAxis();

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

		GameState->CameraP = V3(0.0f, 5.0f, 3.0f);
		GameState->Direction = V3(0.0f, 0.0f, -1.0f);
		GameState->CameraTransform = Mat4Camera(GameState->CameraP, GameState->CameraP + GameState->Direction);

		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)Win32GlobalWindowWidth / Win32GlobalWindowHeight;
		GameState->ZNear = 0.1f;
		GameState->ZFar = 100.0f;
		GameState->PerspectiveTransform = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);

		GameState->Shaders[0] = GLProgramCreate(MainVS, MainFS);
		u32 Shader = GameState->Shaders[0];
		u32 Shader1 = GameState->Shaders[1];
		if(Model)
		{
			if(Model->HasSkeleton)
			{
				OpenGLAllocateAnimatedModel(Model, Shader);
				glUseProgram(Shader);
				UniformMatrixSet(Shader, "View", GameState->CameraTransform);
				UniformMatrixSet(Shader, "Projection", GameState->PerspectiveTransform);
				UniformV3Set(Shader, "CameraP", GameState->CameraP);
			}
			else
			{
				OpenGLAllocateModel(Model, Shader1);
				glUseProgram(Shader1);
				UniformMatrixSet(Shader1, "View", GameState->CameraTransform);
				UniformMatrixSet(Shader1, "Projection", GameState->PerspectiveTransform);
			}
		}

		GameMemory->IsInitialized = true;
	}

	v3 ddP = {};
	game_keyboard *Keyboard = &GameInput->Keyboard;
	if(Keyboard->W.IsDown)
	{
		ddP += V3(0.0f, 0.0f, -1.0f);
	}
	if(Keyboard->A.IsDown)
	{
		ddP += V3(-1.0f, 0.0f, 0.0f);
	}
	if(Keyboard->S.IsDown)
	{
		ddP += V3(0.0f, 0.0f, 1.0f);
	}
	if(Keyboard->D.IsDown)
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

	f32 DtForFrame = GameInput->DtForFrame;
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				Entity->P += ddP;
			} break;
		};
	}

	//
	// NOTE(Justin): Render.
	//

	glViewport(0, 0, (u32)Win32GlobalWindowWidth, (u32)Win32GlobalWindowHeight);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	f32 Angle = 0.0f;
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				AnimationPlay(&GameState->AnimationPlayer, &GameState->Animations[0], true, false);
				AnimationPlayerUpdate(&GameState->AnimationPlayer, &GameState->TempArena, DtForFrame);
				ModelUpdate(&GameState->AnimationPlayer);
				model *Model = GameState->XBot;
				Model->Basis.O = Entity->P;
				glUseProgram(GameState->Shaders[0]);
				UniformV3Set(GameState->Shaders[0], "LightDir", V3(2.0f * cosf(Angle), 0.0f, 2.0f * sinf(Angle)));
				OpenGLDrawAnimatedModel(Model, GameState->Shaders[0]);
			} break;
			case EntityType_Cube:
			{
#if 0
				model *Model = GameState->XBot;
				Model->Basis.O = Entity->P;
				Model->Basis.X = XAxis();
				Model->Basis.Y = YAxis();
				Model->Basis.Z = ZAxis();

				glUseProgram(GameState->Shaders[1]);
				mat4 R = Mat4YRotation(DegreeToRad(Angle));
				mat4 Scale = Mat4Identity();
				UniformMatrixSet(GameState->Shaders[0], "Model", Mat4Translate(Model->Basis.O) * R * Scale);
				OpenGLDrawModel(Model, GameState->Shaders[0]);
#endif
			} break;
		};
	}
}
