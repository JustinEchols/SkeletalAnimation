
char *AnimationFiles[] =
{
	"..\\data\\XBot_IdleToSprint.animation",
	"..\\data\\XBot_Running.animation",
	"..\\data\\XBot_ActionIdle.animation",
	"..\\data\\XBot_IdleLookAround.animation",
	"..\\data\\XBot_RightTurn.animation",
	"..\\data\\XBot_LeftTurn.animation",
	"..\\data\\XBot_PushingStart.animation",
	"..\\data\\XBot_Pushing.animation",
	"..\\data\\XBot_PushingStop.animation",
	"..\\data\\XBot_ActionIdleToStandingIdle.animation",
	"..\\data\\XBot_RunningToTurn.animation",
	"..\\data\\XBot_RunningChangeDirection.animation",
	"..\\data\\XBot_FemaleWalk.animation",
};

internal void
GameUpdateAndRender(game_memory *GameMemory, f32 DtForFrame)
{
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	if(!GameMemory->IsInitialized)
	{
		ArenaInitialize(&GameState->Arena, (u8 *)GameMemory->PermanentStorage + sizeof(game_state),
												 GameMemory->PermanentStorageSize - sizeof(game_state)); 

		ArenaInitialize(&GameState->TempArena, (u8 *)GameMemory->TemporaryStorage, GameMemory->TemporaryStorageSize);

		memory_arena *Arena = &GameState->Arena;
		GameState->Model = PushStruct(Arena, model);
		*GameState->Model = ModelLoad(Arena, "..\\data\\XBot.mesh");

		model *Model = GameState->Model;
		Model->Basis.O = V3(0.0f, -80.0f, -400.0f);
		Model->Basis.X = XAxis();
		Model->Basis.Y = YAxis();
		Model->Basis.Z = ZAxis();
		mat4 Scale = Mat4Identity();

		AnimationPlayerInitialize(&GameState->AnimationPlayer, Model, Arena);
		GameState->AnimationInfos = PushArray(Arena, ArrayCount(AnimationFiles), animation_info);
		GameState->Animations = PushArray(Arena, ArrayCount(AnimationFiles), animation);

		for(u32 AnimIndex = 0; AnimIndex < ArrayCount(AnimationFiles); ++AnimIndex)
		{
			GameState->AnimationInfos[AnimIndex] = AnimationInfoLoad(Arena, AnimationFiles[AnimIndex]);

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

		GameState->Shaders[0] = GLProgramCreate(BasicVsSrc, BasicFsSrc);
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


		GameMemory->IsInitialized;
	}

	model *Model = GameState->Model;
	if(Model)
	{
		if(Model->HasSkeleton)
		{
			AnimationPlay(&GameState->AnimationPlayer, &GameState->Animations[0], true, false);
			AnimationPlayerUpdate(&GameState->AnimationPlayer, &GameState->TempArena, DtForFrame);
			ModelUpdate(&GameState->AnimationPlayer);
		}
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

	f32 *Theta = &GameState->Angle;
	*Theta += DtForFrame;
	f32 Angle = *Theta;

	//model *Model = &GameState->Model;//[ModelIndex];
	mat4 Scale = Mat4Identity();
	if(Model)
	{
		if(Model->HasSkeleton)
		{
			glUseProgram(GameState->Shaders[0]);
			UniformV3Set(GameState->Shaders[0], "LightDir", V3(2.0f * cosf(Angle), 0.0f, 2.0f * sinf(Angle)));
			OpenGLDrawAnimatedModel(Model, GameState->Shaders[0]);
		}
		else
		{
			glUseProgram(GameState->Shaders[1]);
			f32 A = 20.0f * Angle;
			mat4 R = Mat4YRotation(DegreeToRad(A));
			UniformMatrixSet(GameState->Shaders[1], "Model", Mat4Translate(Model->Basis.O) * R * Scale);
			OpenGLDrawModel(Model, GameState->Shaders[1]);
		}
	}
}
