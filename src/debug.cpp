
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
		case MovementState_InAir:
		{
			sprintf(Buffer, "%s", "MovementState: InAir");
		} break;
	}
}

inline void 
AttackTypeToString(char *Buffer, attack_type Type)
{
	switch(Type)
	{
		case AttackType_Neutral1:
		case AttackType_Neutral2:
		case AttackType_Neutral3:
		{
			sprintf(Buffer, "%s", "Neutral");
		} break;
		case AttackType_Forward:
		{
			sprintf(Buffer, "%s", "Forward");
		} break;
		case AttackType_Strong:
		{
			sprintf(Buffer, "%s", "Strong");
		} break;
		case AttackType_Dash:
		{
			sprintf(Buffer, "%s", "Dash");
		} break;
		case AttackType_Air:
		{
			sprintf(Buffer, "%s", "Air");
		} break;
		case AttackType_None:
		{
			sprintf(Buffer, "%s", "None");
		} break;
	}
}

internal void
DebugDrawString(char *String, v3 Color = V3(1.0f))
{
	string Text = StringCopy(Ui.TempArena, String);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	Ui.P.y -= Ui.LineGap;
}

internal void
DebugDrawString(char *Label, char *String, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %s", Label, String);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	Ui.P.y -= Ui.LineGap;
}

internal void
DebugDrawFloat(char *String, f32 F32, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %.2f", String, F32);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	Ui.P.y -= Ui.LineGap;
}

internal void
DebugDrawVector3(char *String, v3 Vector3, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %.1f %.1f %.1f", String, Vector3.x, Vector3.y, Vector3.z);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	Ui.P.y -= Ui.LineGap;
}

internal void
DebugDrawOBB(render_buffer *RenderBuffer, model *DebugCube, obb OBB, v3 P, v3 Offset, v3 Color)
{
	v3 X = OBB.X;
	v3 Y = OBB.Y;
	v3 Z = OBB.Z;

	mat4 T = Mat4Translate(P + Offset);
	mat4 R = Mat4(X, Y, Z);
	mat4 S = Mat4Scale(OBB.Dim);

	PushDebugVolume(RenderBuffer, DebugCube, T*R*S, Color);
}

internal void
DebugDrawCapsule(entity *Entity)
{
	capsule Capsule = Entity->MovementColliders.Volumes[0].Capsule;
	mat4 T = Mat4Translate(Entity->P + CapsuleCenter(Capsule));
	mat4 R = QuaternionToMat4(Entity->Orientation);
	mat4 S = Mat4Scale(1.0f);
	PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Capsule, T*R*S, V3(1.0f));
}

internal void
DebugDrawGroundArrow(entity *Entity, quad Quad)
{
	v3 P = Entity->P;
	P.y += 0.01f;
	mat4 T = Mat4Translate(P);
	mat4 R = QuaternionToMat4(Entity->AnimationPlayer->OrientationLockedAt);
	mat4 S = Mat4Scale(V3(0.5f));

	asset_entry Entry = LookupTexture(Ui.Assets, "left_arrow");
	PushTexture(Ui.RenderBuffer, Entry.Texture, Entry.Index);
	PushQuad3D(Ui.RenderBuffer, Quad.Vertices, T*R*S, Entry.Index);
}

internal void
DebugDrawCollisionVolumes(entity *Entity)
{
	collision_group *Colliders = &Entity->CombatColliders;
	mat4 T = Mat4Identity();
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(Colliders->Volumes[0].Dim.x);

	for(u32 VolumeIndex = 0; VolumeIndex < Colliders->VolumeCount; ++VolumeIndex)
	{
		collision_volume *Volume = Colliders->Volumes + VolumeIndex;
		v3 P = Entity->P + Volume->Offset;
		T = Mat4Translate(P);
		PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, T*R*S, V3(1.0f));
	}
}

internal void
DebugDrawJoints(entity *Entity)
{
	if(!Entity->AnimationPlayer)
	{
		return;
	}

	model *Model = Entity->AnimationPlayer->Model;
	mat4 Transform	= EntityTransform(Entity, Entity->VisualScale);
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		for(u32 JointIndex = 0; JointIndex < Mesh->JointCount; ++JointIndex)
		{
			mat4 JointTransform = Mesh->JointTransforms[JointIndex];
			JointTransform = Transform * JointTransform;
			PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, JointTransform, V3(1.0f));
		}
	}
}

internal void
DebugDrawEntity(entity *Entity)
{
	Ui.P.x += 20.0f;

	DebugDrawVector3("p: ", Entity->P);
	DebugDrawVector3("dP: ", Entity->dP);
	DebugDrawVector3("ddP: ", Entity->ddP);
	DebugDrawFloat("Theta: ", Entity->Theta);
	DebugDrawFloat("ThetaTarget: ", Entity->ThetaTarget);

	f32 dTheta = Entity->ThetaTarget - Entity->Theta;
	DebugDrawFloat("dTheta: ", dTheta);

	// NOTE(Justin): 
	// dTheta > 0 -> CCW turning left
	// dTheta < 0 -> CW turning right 

	b32 TurningLeft = (dTheta > 0.0f);
	b32 TurningRight = (dTheta < 0.0f);
	char *Turning = "";
	if(TurningRight)
	{
		Turning = "turning: Right";
	}
	else if(TurningLeft)
	{
		Turning = "turning: Left";
	}
	else
	{
		Turning = "turning: Still";
	}

	DebugDrawString(Turning);

	char *YSupported = "";
	if(FlagIsSet(Entity, EntityFlag_YSupported))
	{
		YSupported = "y supported: true";
	}
	else
	{
		YSupported = "y supported: false";
	}

	DebugDrawString(YSupported);

	char Buffer[256];
	AttackTypeToString(Buffer, Entity->AttackType);
	DebugDrawString("AttackType: ", Buffer);
	DebugDrawFloat("t: ", Entity->Attacks[Entity->AttackType].CurrentTime);

	Ui.P.x -= 20.0f;
}

internal void
DebugDrawAnimationPlayer(animation_player *AnimationPlayer)
{
	if(!AnimationPlayer)
	{
		return;
	}

	Ui.P.x += 20.0f;

	if(AnimationPlayer->ControlsPosition)
	{
		DebugDrawVector3("RootMotionAccumulator: ", AnimationPlayer->RootMotionAccumulator, Ui.HoverColor);
	}
	else
	{
		DebugDrawVector3("RootMotionAccumulator: ", AnimationPlayer->RootMotionAccumulator);
	}

	if(AnimationPlayer->ControlsTurning)
	{
		DebugDrawFloat("RootTurningAccumulator: ", AnimationPlayer->RootTurningAccumulator, Ui.HoverColor);
	}
	else
	{
		DebugDrawFloat("RootTurningAccumulator: ", AnimationPlayer->RootTurningAccumulator);
	}

	char Buff[256];
	MovementStateToString(Buff, AnimationPlayer->MovementState);
	DebugDrawString(Buff);

	for(animation *Animation = AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		DebugDrawString(CString(Animation->Name));
		DebugDrawFloat("Duration: ", Animation->Duration);
		DebugDrawFloat("t: ", Animation->CurrentTime);
		DebugDrawFloat("blend duration: ", Animation->BlendDuration);
		DebugDrawFloat("t_blend: ", Animation->BlendCurrentTime);
		DebugDrawFloat("t_scale: ", Animation->TimeScale);
		DebugDrawFloat("factor: ", Animation->BlendFactor);

		Ui.P.y -= Ui.LineGap;
	}
	Ui.P.x -= 20.0f;
}

internal void
DebugDrawHandAndFoot(entity *Entity)
{
	mat4 T = EntityTransform(Entity, Entity->VisualScale);
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(0.2f);

	T = Mat4Translate(Entity->LeftFootP);
	PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, T*R*S, V3(1.0f));

	T = Mat4Translate(Entity->RightFootP);
	PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, T*R*S, V3(1.0f));

	T = Mat4Translate(Entity->LeftHandP);
	PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, T*R*S, V3(1.0f));

	T = Mat4Translate(Entity->RightHandP);
	PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, T*R*S, V3(1.0f));
}

internal void
DebugDrawTexture(game_state *GameState)
{
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
