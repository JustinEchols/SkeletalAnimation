
#include "game.h"
#include "strings.cpp"
#include "texture.cpp"
#include "font.cpp"
#include "mesh.cpp"
#include "animation.cpp"
#include "asset.cpp"
#include "render.cpp"

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
	Entity->MovementState = MovementState_Idle;
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

inline void 
EntityMovementState(char *Buffer, entity *Entity)
{
	switch(Entity->MovementState)
	{
		case MovementState_Idle:
		{
			sprintf(Buffer, "%s", "MovementState: Idle");
		} break;
		case MovementState_Run:
		{
			sprintf(Buffer, "%s", "MovementState: Running");
		} break;
		case MovementState_Sprint:
		{
			sprintf(Buffer, "%s", "MovementState: Sprint");
		} break;
		case MovementState_Jump:
		{
			sprintf(Buffer, "%s", "MovementState: JumpForward");
		} break;
	}
}

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

inline void
OrientationUpdate(quaternion *Orientation, v3 FacingDirection, f32 dt, f32 AngularSpeed)
{
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
		//
		// NOTE(Justin): Arena.
		//

		ArenaInitialize(&GameState->Arena, (u8 *)GameMemory->PermanentStorage + sizeof(game_state),
												 GameMemory->PermanentStorageSize - sizeof(game_state)); 

		memory_arena *Arena = &GameState->Arena;

		//
		// NOTE(Justin): Assets.
		//

		ArenaSubset(&GameState->Arena, &GameState->AssetManager.Arena, Kilobyte(512));
		AssetManagerInit(&GameState->AssetManager);
		asset_manager *Assets = &GameState->AssetManager;

		GameState->Quad = QuadDefault();
		GameState->Quad.Texture = LookupTexture(Assets, "left_arrow")->Handle;

		PlayerAdd(GameState);
		entity *Player			= GameState->Entities + GameState->PlayerEntityIndex;
		Player->AnimationPlayer = PushStruct(Arena, animation_player);
		Player->AnimationGraph	= PushStruct(Arena, animation_graph);
		model *XBot				= LookupModel(Assets, "XBot");
		AnimationPlayerInitialize(Player->AnimationPlayer, XBot, Arena);
		Player->AnimationGraph	= LookupGraph(Assets, "XBot_AnimationGraph");

		RandInit(2024);
		for(u32 Index = 0; Index < 10; ++Index)
		{
			v3 P = V3(RandBetween(-100.0f, 100.0f), RandBetween(1.0f, 2.0f), RandBetween(-10.0f, -100.0f));
			CubeAdd(GameState, P);
		}

		CameraSet(&GameState->Camera, V3(0.0f, 15.0f, 20.0f), -90.0f, -10.0f);
		CameraTransformUpdate(GameState);
		GameState->CameraOffsetFromPlayer = V3(0.0f, 5.0f, 10.0f);

		GameState->TimeScale = 1.0f;
		GameState->FOV = DegreeToRad(45.0f);
		GameState->Aspect = (f32)Win32GlobalWindowWidth / (f32)Win32GlobalWindowHeight;
		GameState->ZNear = 0.2f;
		GameState->ZFar = 200.0f;
		GameState->Perspective = Mat4Perspective(GameState->FOV, GameState->Aspect, GameState->ZNear, GameState->ZFar);

#if 0
		OpenGLAllocateQuad2d(&GameState->Quad2d.VA, &GameState->Quad2d.VB, FontShader);
		GameState->TextureWidth = 256;
		GameState->TextureHeight = 256;
		OpenGLFrameBufferInit(&GameState->FBO, &GameState->Texture, &GameState->RBO, GameState->TextureWidth, GameState->TextureHeight);
#endif
		GameMemory->IsInitialized = true;
	}

	temp_state *TempState = (temp_state *)GameMemory->TemporaryStorage;
	if(!TempState->IsInitialized)
	{
		ArenaInitialize(&TempState->Arena,
				(u8 *)GameMemory->TemporaryStorage + sizeof(temp_state),
					  GameMemory->TemporaryStorageSize - sizeof(temp_state));

		TempState->IsInitialized = true;
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

				f32 a = 100.0f;
				if(Sprinting)
				{
					a *= 1.5f;
				}

				f32 PlayerDrag = -10.0f;
				f32 AngularSpeed = 10.0f;
				f32 Speed = Length(Entity->dP);
				ddP = a * ddP;
				ddP += PlayerDrag * Entity->dP;

				v3 OldP = Entity->P;
				v3 dP = Entity->dP + dt * ddP;
				v3 P =  Entity->P + 0.5f * dt * dt * ddP + dt * Entity->dP;

				Entity->P = P;
				Entity->dP = dP;

				EntityOrientationUpdate(Entity, dt, AngularSpeed);
				switch(Entity->MovementState)
				{
					case MovementState_Idle:
					{
						if(Equal(OldPlayerddP, V3(0.0f)) &&
						  !Equal(Entity->ddP, OldPlayerddP))
						{
							if(Sprinting)
							{
								Entity->MovementState = MovementState_Sprint;
							}
							else
							{
								Entity->MovementState = MovementState_Run;
							}
						}
					} break;
					case MovementState_Run:
					{
						if(Equal(Entity->ddP, V3(0.0f)))
						{
							Entity->MovementState = MovementState_Idle;
						}
						else
						{
							if(Sprinting)
							{
								Entity->MovementState = MovementState_Sprint;
							}

							if(Jumping)
							{
								Entity->MovementState = MovementState_Jump;
							}
						}

					} break;
					case MovementState_Sprint:
					{
						if(Equal(Entity->ddP, V3(0.0f)))
						{
							Entity->MovementState = MovementState_Idle;
						}
						else
						{
							if(!Sprinting)
							{
								Entity->MovementState = MovementState_Run;
							}
						}
					} break;
					case MovementState_Jump:
					{
						if(!Jumping)
						{
							if(Equal(Entity->ddP, V3(0.0f)))
							{
								Entity->MovementState = MovementState_Idle;
							}
							else if(Sprinting)
							{
								Entity->MovementState = MovementState_Sprint;
							}
							else
							{
								Entity->MovementState = MovementState_Run;
							}
						}
					} break;
					case MovementState_Invalid:
					{
						Entity->MovementState = MovementState_Invalid;
					} break;

				};
			} break;
		}
	}

	asset_manager *Assets = &GameState->AssetManager;

	//
	// NOTE(Justin): Animation update
	//

	entity *Player = GameState->Entities + GameState->PlayerEntityIndex;
	Animate(Player->AnimationGraph, Assets, Player->AnimationPlayer, Player->MovementState);
	AnimationPlayerUpdate(Player->AnimationPlayer, &TempState->Arena, dt);
	ModelJointsUpdate(Player->AnimationPlayer);
	AnimationGraphPerFrameUpdate(Assets, Player->AnimationPlayer, Player->AnimationGraph);

	camera *Camera = &GameState->Camera;
	Camera->P = Player->P + GameState->CameraOffsetFromPlayer;

	//
	// NOTE(Justin): Render.
	//

	//
	// TODO(Justin): Shadow Pass.
	//

	v3 LightDir = V3(1.0f, -1.0f, -0.5f);

	//
	// NOTE(Justin): Final pass.
	//

	PerspectiveTransformUpdate(GameState);
	CameraTransformUpdate(GameState);
	u32 FontShader	= GameState->Shaders[2];
	render_buffer *RenderBuffer = RenderBufferAllocate(&TempState->Arena, Megabyte(512),
														GameState->CameraTransform,
														GameState->Perspective,
														Assets,
														Camera->P);

	PushClear(RenderBuffer, V4(0.3f, 0.4f, 0.4f, 1.0f));

	//
	// NOTE(Justin): Ground quad.
	//

	mat4 T = Mat4Translate(V3(0.0f, 0.0f, -500.0f));
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(1000.0f);

	s32 TextureIndex = StringHashLookup(&Assets->TextureNames, "tile_gray");
	PushTexture(RenderBuffer, LookupTexture(Assets, "tile_gray"), TextureIndex);
	PushQuad3D(RenderBuffer, &GameState->Quad, T*R*S, TextureIndex);

	//
	// NOTE(Justin): Entities.
	//

	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity *Entity = GameState->Entities + EntityIndex;
		switch(Entity->Type)
		{
			case EntityType_Player:
			{
				mat4 Transform = EntityTransform(Entity, 0.025f);
				model *Model = LookupModel(Assets, "XBot");
				PushModel(RenderBuffer, Model, Transform);

				//
				// NOTE(Justin): Debug orientation arrow 
				//

				v3 P = Entity->P;
				P.y += 0.25f;
				T = Mat4Translate(P);
				R = QuaternionToMat4(Entity->Orientation);
				S = Mat4Scale(V3(1.0f));

				TextureIndex = StringHashLookup(&Assets->TextureNames, "left_arrow");
				PushTexture(RenderBuffer, LookupTexture(Assets, "left_arrow"), TextureIndex);
				PushQuad3D(RenderBuffer, &GameState->Quad, T*R*S, TextureIndex);
			} break;
			case EntityType_Cube:
			{
				model *Cube = LookupModel(Assets, "Cube");
				mat4 Transform = EntityTransform(Entity, 1.0f);
				PushTexture(RenderBuffer, Cube->Meshes[0].Texture, StringHashLookup(&Assets->TextureNames, (char *)Cube->Meshes[0].Texture->Name.Data));
				PushModel(RenderBuffer, Cube, Transform);
			} break;
		};
	}

#if 1
	//
	// NOTE(Justin): Test font/ui.
	//

	entity *Entity = GameState->Entities + GameState->PlayerEntityIndex;

	font *FontInfo =  &Assets->Font;
	f32 Scale = 0.35f;
	f32 Gap = Scale * (f32)FontInfo->LineHeight / 64.0f;
	f32 X = 0.0f;
	f32 Y = (f32)Win32GlobalWindowHeight - Gap;
	f32 dY = 5.0f;
	s32 WindowWidth = GameInput->BackBufferWidth;
	s32 WindowHeight = GameInput->BackBufferHeight;
	v2 MouseP = V2(GameInput->MouseX, GameInput->MouseY);
	v2 P = V2(X, Y);
	v3 HoverColor = V3(1.0f, 1.0f, 0.0f);
	v3 DefaultColor = V3(1.0f);

	char Buff[256];
	sprintf(Buff, "%s", "Controls: wasd to move, shift to sprint, +- to scale time");
	string Text = StringCopy(&TempState->Arena, Buff);

	PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
	P.y -= (Gap + dY);

	sprintf(Buff, "%s %.2f", "time scale: ", GameState->TimeScale);
	Text = StringCopy(&TempState->Arena, Buff);
	rect Rect = RectMinDim(P, TextDim(FontInfo, Scale, Buff));
	if(InRect(Rect, MouseP))
	{
		PushText(RenderBuffer, Text, FontInfo, P, Scale, HoverColor);
	}
	else
	{
		PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);
	}

	//
	// NOTE(Jusitn): Animation information.
	//

	P.y -= (Gap + dY);
	sprintf(Buff, "%s", "Animation Control");
	Text = StringCopy(&TempState->Arena, Buff);
	PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

	P.x += 20.0f;
	EntityMovementState(Buff, Entity);
	OpenGLDrawText(Buff, FontShader, &Assets->Font, P, Scale, DefaultColor, WindowWidth, WindowHeight);

	for(animation *Animation = Player->AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		if(Animation)
		{
			P.y -= (Gap + dY);
			sprintf(Buff, "Name: %s", Animation->Name.Data);
			Text = StringCopy(&TempState->Arena, Buff);
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

			P.y -= (Gap + dY);
			sprintf(Buff, "%s %.2f", "Duration: ", Animation->Duration);
			Text = StringCopy(&TempState->Arena, Buff);
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

			P.y -= (Gap + dY);
			sprintf(Buff, "%s %.2f", "t: ", Animation->CurrentTime);
			Text = StringCopy(&TempState->Arena, Buff);
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

			P.y -= (Gap + dY);
			sprintf(Buff, "%s %.2f", "blend duration: ", Animation->BlendDuration);
			Text = StringCopy(&TempState->Arena, Buff);
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

			P.y -= (Gap + dY);
			sprintf(Buff, "%s %.2f", "blend_t: ", Animation->BlendCurrentTime);
			Text = StringCopy(&TempState->Arena, Buff);
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

			P.y -= (Gap + dY);
			sprintf(Buff, "%s %.2f", "blend: ", Animation->BlendFactor);
			Text = StringCopy(&TempState->Arena, Buff);
			PushText(RenderBuffer, Text, FontInfo, P, Scale, DefaultColor);

			P.y -= (Gap + dY);
		}
	}

#if 0
	//
	// NOTE(Justin): Render to texture
	//

	glViewport(0, 0, GameState->TextureWidth, GameState->TextureHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, GameState->FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	mat4 Transform = EntityTransform(Entity, 0.025f);
	model *Model = LookupModel(Assets, "XBot");
	glUseProgram(MainShader);
	UniformMatrixSet(MainShader, "View", GameState->CameraTransform);

	mat4 Persp = Mat4Perspective(GameState->FOV, ((f32)GameState->TextureWidth / (f32)GameState->TextureHeight), GameState->ZNear, GameState->ZFar);
	UniformMatrixSet(MainShader, "Projection", Persp);
	UniformV3Set(MainShader, "CameraP", Camera->P);
	UniformV3Set(MainShader, "Ambient", V3(0.1f));
	UniformV3Set(MainShader, "CameraP", Camera->P);
	UniformV3Set(MainShader, "LightDir", LightDir);
	OpenGLDrawAnimatedModel(Model, MainShader, Transform);

	//
	// NOTE(Justin): Display texture in default framebuffer
	//

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	u32 ScreenShader = GameState->Shaders[3];
	quad_2d *Quad2d = &GameState->Quad2d; 

	P = V2(0.0f);
	f32 Width = (f32)GameState->TextureWidth;
	f32 Height = (f32)GameState->TextureHeight;
	Rect = RectMinDim(P, V2(Width, Height));
	f32 Vertices[6][4] =
	{
		{Rect.Min.x, Rect.Min.y, 0.0f, 0.0f},
		{Rect.Max.x, Rect.Min.y, 1.0f, 0.0f},
		{Rect.Max.x, Rect.Max.y, 1.0f, 1.0f},

		{Rect.Max.x, Rect.Max.y, 1.0f, 1.0f},
		{Rect.Min.x, Rect.Max.y, 0.0f, 1.0f},
		{Rect.Min.x, Rect.Min.y, 0.0f, 0.0f},
	};

	glUseProgram(ScreenShader);
	glActiveTexture(GL_TEXTURE0);
	UniformBoolSet(ScreenShader, "Texture", 0);
	UniformF32Set(ScreenShader, "WindowWidth", (f32)WindowWidth);
	UniformF32Set(ScreenShader, "WindowHeight", (f32)WindowHeight);
	glBindTexture(GL_TEXTURE_2D, GameState->Texture);
	glBindVertexArray(Quad2d->VA);
	glBindBuffer(GL_ARRAY_BUFFER, Quad2d->VB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
#endif

	RenderBufferToOutput(RenderBuffer, (u32)Win32GlobalWindowWidth, (u32)Win32GlobalWindowHeight);

	ArenaClear(&TempState->Arena);
}
