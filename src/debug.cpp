
inline void 
MovementStateToString(char *Buffer, movement_state State)
{
	switch(State)
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
		case MovementState_Crouch:
		{
			sprintf(Buffer, "%s", "MovementState: Crouch");
		} break;
		case MovementState_Sliding:
		{
			sprintf(Buffer, "%s", "MovementState: Sliding");
		} break;
		case MovementState_Attack:
		{
			sprintf(Buffer, "%s", "MovementState: Attack");
		} break;
	}
}

internal void
DebugDrawEntity(char *Label, entity *Entity)
{
	char Buff[256];
	string Text;

	if(!Ui.DebugEntityView)
	{
		sprintf(Buff, "+Player");
		Text = StringCopy(Ui.TempArena, Buff);
		PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
		Ui.P.y -= Ui.LineGap;
		return;
	}

	sprintf(Buff, Label);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.HoverColor);
	Ui.P.y -= Ui.LineGap;

	Ui.P.x += 20.0f;

	sprintf(Buff, "p: %.1f %.1f %.1f", Entity->P.x, Entity->P.y, Entity->P.z);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	sprintf(Buff, "dP: %.1f %.1f %.1f", Entity->dP.x, Entity->dP.y, Entity->dP.z);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	sprintf(Buff, "ddP: %.1f %.1f %.1f", Entity->ddP.x, Entity->ddP.y, Entity->ddP.z);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	sprintf(Buff, "Theta: %.2f", Entity->Theta);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	sprintf(Buff, "ThetaTarget: %.2f", Entity->ThetaTarget);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	sprintf(Buff, "dTheta: %.2f", Entity->dTheta);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	// NOTE(Justin): 
	// dTheta > 0 -> CCW turning left
	// dTheta < 0 -> CW turning right 

	b32 TurningLeft = (Entity->dTheta > 0.0f);
	b32 TurningRight = (Entity->dTheta < 0.0f);
	if(TurningRight)
	{
		sprintf(Buff, "turning: Right");
	}
	else if(TurningLeft)
	{
		sprintf(Buff, "turning: Left");
	}
	else
	{
		sprintf(Buff, "turning: Still");
	}

	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	if(FlagIsSet(Entity, EntityFlag_YSupported))
	{
		sprintf(Buff, "y supported: true");
	}
	else
	{
		sprintf(Buff, "y supported: false");
	}

	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;
	Ui.P.x -= 20.0f;
}

internal void
DebugDrawAnimationPlayer(char *Label, animation_player *AnimationPlayer)
{
	char Buff[256];
	string Text;

	if(!Ui.DebugAnimationPlayerView)
	{
		sprintf(Buff, "+AnimationPlayer");
		Text = StringCopy(Ui.TempArena, Buff);
		PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
		Ui.P.y -= Ui.LineGap;
		return;
	}

	sprintf(Buff, Label);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.HoverColor);
	Ui.P.y -= Ui.LineGap;

	Ui.P.x += 20.0f;

	sprintf(Buff, "RootMotionAccumulator: %.1f %.1f %.1f",	AnimationPlayer->RootMotionAccumulator.x,
															AnimationPlayer->RootMotionAccumulator.y,
															AnimationPlayer->RootMotionAccumulator.z);
	Text = StringCopy(Ui.TempArena, Buff);

	v3 Color = V3(1.0f);
	if(AnimationPlayer->ControlsPosition) Color = Ui.HoverColor;
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	Ui.P.y -= Ui.LineGap;

	sprintf(Buff, "RootTurningAccumulator: %f", AnimationPlayer->RootTurningAccumulator);
	Text = StringCopy(Ui.TempArena, Buff);
	Color = V3(1.0f);
	if(AnimationPlayer->ControlsTurning) Color = Ui.HoverColor;
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	Ui.P.y -= Ui.LineGap;

	MovementStateToString(Buff, AnimationPlayer->MovementState);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
	Ui.P.y -= Ui.LineGap;

	for(animation *Animation = AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		if(Animation)
		{
			sprintf(Buff, "Name: %s", Animation->Name.Data);
			Text = StringCopy(Ui.TempArena, Buff);
			PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
			Ui.P.y -= Ui.LineGap;

			sprintf(Buff, "%s %.2f", "Duration: ", Animation->Duration);
			Text = StringCopy(Ui.TempArena, Buff);
			PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
			Ui.P.y -= Ui.LineGap;

			sprintf(Buff, "%s %.2f", "t: ", Animation->CurrentTime);
			Text = StringCopy(Ui.TempArena, Buff);
			PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
			Ui.P.y -= Ui.LineGap;

			sprintf(Buff, "%s %.2f", "blend duration: ", Animation->BlendDuration);
			Text = StringCopy(Ui.TempArena, Buff);
			PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
			Ui.P.y -= Ui.LineGap;

			sprintf(Buff, "%s %.2f", "blend_t: ", Animation->BlendCurrentTime);
			Text = StringCopy(Ui.TempArena, Buff);
			PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
			Ui.P.y -= Ui.LineGap;

			sprintf(Buff, "%s %.2f", "blend: ", Animation->BlendFactor);
			Text = StringCopy(Ui.TempArena, Buff);
			PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
			Ui.P.y -= Ui.LineGap;
		}
		Ui.P.y -= Ui.LineGap;
	}
	Ui.P.x -= 20.0f;
}

internal void
DebugDrawHandAndFoot(char *Label, entity *Entity, model *Sphere)
{
	char Buff[256];
	string Text;

	if(!Ui.DebugDrawHandAndFoot)
	{
		sprintf(Buff, "+HandAndFoot");
		Text = StringCopy(Ui.TempArena, Buff);
		PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
		Ui.P.y -= Ui.LineGap;
		return;
	}

	sprintf(Buff, Label);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.HoverColor);
	Ui.P.y -= Ui.LineGap;

	mat4 T = EntityTransform(Entity, Entity->VisualScale);
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(0.2f);

	T = Mat4Translate(Entity->LeftFootP);
	PushAABB(Ui.RenderBuffer, Sphere, T*R*S, V3(1.0f));

	T = Mat4Translate(Entity->RightFootP);
	PushAABB(Ui.RenderBuffer, Sphere, T*R*S, V3(1.0f));

	T = Mat4Translate(Entity->LeftHandP);
	PushAABB(Ui.RenderBuffer, Sphere, T*R*S, V3(1.0f));

	T = Mat4Translate(Entity->RightHandP);
	PushAABB(Ui.RenderBuffer, Sphere, T*R*S, V3(1.0f));
}

internal void
DebugDrawTexture(char *Label, game_state *GameState)
{
	char Buff[256];
	string Text;

	if(!Ui.DebugDrawTexture)
	{
		sprintf(Buff, "+Texture");
		Text = StringCopy(Ui.TempArena, Buff);
		PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.DefaultColor);
		Ui.P.y -= Ui.LineGap;
		return;
	}

	sprintf(Buff, Label);
	Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Ui.HoverColor);
	Ui.P.y -= Ui.LineGap;

	//
	// NOTE(Justin): Render to texture
	//

	// TODO(Justin); Cleanup
	mat4 Persp = Mat4Perspective(GameState->FOV, 256.0f/256.0f, GameState->ZNear, GameState->ZFar);
	render_buffer *RenderToTextureBuffer = RenderBufferAllocate(Ui.TempArena, Megabyte(64),
			GameState->CameraTransform,
			Persp,
			Mat4Identity(),
			Ui.Assets,
			GameState->Camera.P,
			V3(0.0f),
			2);

	PushClear(RenderToTextureBuffer, V4(1.0f));
	Platform.RenderToOpenGL(RenderToTextureBuffer, GameState->Texture.Width, GameState->Texture.Height);

	//
	// NOTE(Justin): Render to default frame buffer
	//

	f32 Width = 256.0f;
	f32 Height = 256.0f;
	v2 P = V2(0.0f, Ui.P.y - Height);

	rect Rect = RectMinDim(P, V2(Width, Height));
	f32 Vertices[6][4] =
	{
		{Rect.Min.x, Rect.Min.y, 0.0f, 0.0f},
		{Rect.Max.x, Rect.Min.y, 1.0f, 0.0f},
		{Rect.Max.x, Rect.Max.y, 1.0f, 1.0f},

		{Rect.Max.x, Rect.Max.y, 1.0f, 1.0f},
		{Rect.Min.x, Rect.Max.y, 0.0f, 1.0f},
		{Rect.Min.x, Rect.Min.y, 0.0f, 0.0f},
	};

	PushRenderToTexture(Ui.RenderBuffer, (f32 *)Vertices);
}

