
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
		case AttackType_Sprint:
		{
			sprintf(Buffer, "%s", "Sprint");
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
	UiAdvanceLine();
}

internal void
DebugDrawString(char *Label, char *String, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %s", Label, String);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	UiAdvanceLine();
}

internal void
DebugDrawFloat(char *String, f32 F32, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %.2f", String, F32);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	UiAdvanceLine();
}

internal void
DebugDrawVector2(char *String, v2 Vector2, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %.1f %.1f", String, Vector2.x, Vector2.y);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	UiAdvanceLine();
}

internal void
DebugDrawVector3(char *String, v3 Vector3, v3 Color = V3(1.0f))
{
	char Buff[256];
	sprintf(Buff, "%s %.1f %.1f %.1f", String, Vector3.x, Vector3.y, Vector3.z);
	string Text = StringCopy(Ui.TempArena, Buff);
	PushText(Ui.RenderBuffer, Text, Ui.Font, Ui.P, Ui.Font->Scale, Color);
	UiAdvanceLine();
}

#if 0
internal void
DebugDrawAxes(entity *Entity)
{
	mat4 T = Mat4Identity();
	mat4 R = QuaternionToMat4(Entity->Orientation);
	mat4 S = Mat4Scale(0.5f);

	model *ZArrow = FindModel(Ui.Assets, "Arrow").Model;
	model *XArrow = FindModel(Ui.Assets, "XArrow").Model;
	model *YArrow = FindModel(Ui.Assets, "YArrow").Model;

	obb OBB = Entity->MovementColliders.Volumes[0].OBB;
	v3 Offset = Entity->MovementColliders.Volumes[0].Offset;

	T = Mat4Translate(Entity->P + Offset + 0.5f*OBB.X);
	PushDebugVolume(Ui.RenderBuffer, XArrow, T*R*S, V3(1.0f, 0.0f, 0.0f));

	T = Mat4Translate(Entity->P + Offset + 0.5f*OBB.Y);
	PushDebugVolume(Ui.RenderBuffer, YArrow, T*R*S, V3(0.0f, 1.0f, 0.0f));

	T = Mat4Translate(Entity->P + Offset + 0.5f*OBB.Z);
	PushDebugVolume(Ui.RenderBuffer, ZArrow, T*R*S, V3(0.0f, 0.0f, 1.0f));
}
#endif

internal void
DebugDrawAABB(render_buffer *RenderBuffer, model *DebugCube, v3 P, v3 Offset, v3 Dim, v3 Color)
{
	mat4 T = Mat4Translate(P + Offset);
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(Dim);
	PushDebugVolume(RenderBuffer, DebugCube, T*R*S, Color);
}

internal void
DebugDrawOBB(render_buffer *RenderBuffer, model *DebugCube, obb OBB, v3 P, v3 Offset, v3 Color)
{
	mat4 T = Mat4Translate(P + Offset);
	mat4 R = Mat4(OBB.X, OBB.Y, OBB.Z);
	mat4 S = Mat4Scale(OBB.Dim);
	PushDebugVolume(RenderBuffer, DebugCube, T*R*S, Color);
}



internal void
DebugDrawCapsule(v3 P, v3 Offset, mat4 R, mat4 S, capsule Capsule)
{
	mat4 T = Mat4Translate(P + Offset);

	model *DebugCapsule = DebugModelCapsuleInitialize2(Ui.TempArena, Capsule.Min, Capsule.Max, Capsule.Radius);
	for(u32 MeshIndex = 0; MeshIndex < DebugCapsule->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = DebugCapsule->Meshes + MeshIndex;
		PushImmediateDebugVolume(Ui.RenderBuffer, Mesh->Vertices,
												  Mesh->VertexCount,
												  Mesh->Indices,
												  Mesh->IndicesCount,
												  T*R*S,
												  V3(1.0f));
	}
}

internal void
DebugDrawGroundArrow(entity *Entity, quad Quad)
{
	v3 P = Entity->P;
	P.y += 0.01f;
	mat4 T = Mat4Translate(P);
	mat4 R = QuaternionToMat4(Entity->Orientation);
	mat4 S = Mat4Scale(V3(0.5f));

	asset_entry Entry = FindTexture(Ui.Assets, "left_arrow");
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
	mat4 Transform	= ModelToWorldTransform(Entity->P, Entity->Orientation, Entity->VisualScale);
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
	UiIndentAdd();

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
	MovementStateToString(Buffer, Entity->MovementState);
	DebugDrawString("", Buffer);

	MemoryZero(Buffer, sizeof(Buffer));
	AttackTypeToString(Buffer, Entity->AttackType);
	DebugDrawString("AttackType: ", Buffer);
	DebugDrawFloat("t: ", Entity->Attacks[Entity->AttackType].CurrentTime);

	UiIndentAdd(-Ui.DefaultIndent);
}

internal void
DebugDrawWeaponCollider(entity *Entity)
{
	model *Model = Entity->AnimationPlayer->Model;

	mat4 T = Mat4Translate(Entity->P);
	mat4 R = QuaternionToMat4(Entity->Orientation);
	mat4 S = Mat4Scale(Entity->VisualScale);
	mat4 ModelToWorld = T*R*S;
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		if(Mesh->Flags & MeshFlag_Weapon)
		{
			// NOTE(Justin): Get the length and radius of the capsule in world space.
			v3 Dim = AABBDim(Mesh->BoundingBox);
			f32 Length = Max3(Dim);
			f32 Radius = 0.0f;
			if(Length == Dim.x)
			{
				Radius = 0.5f*Max(Dim.y, Dim.z);
			}
			else if(Length == Dim.y)
			{
				Radius = 0.5f*Max(Dim.x, Dim.z);
			}
			else
			{
				Radius = 0.5f*Max(Dim.x, Dim.y);
			}

			Length = Entity->VisualScale.x * Length;
			Radius = Entity->VisualScale.x * Radius;

			// NOTE(Justin): The collider's position starts at the world position of the weapon joint. We need to offset this position by some amount
			// in the z direction because the joint position is not the same position as the collider's position. In order to do this, get the Z basis
			// vector of the _capsule_ in the world and do the linear combination P + cZ where P is the weapon joint position in the world
			mat4 WeaponTransform = Mesh->JointTransforms[Model->WeaponJointIndex];
			v3 JointP = Mat4ColumnGet(WeaponTransform, 3);
			v3 JointWorldP = ModelToWorld*(JointP);
			
			// TODO(Justin): Compute z coordinate
			affine_decomposition D = Mat4AffineDecomposition(WeaponTransform);
			R = R*D.R;
			v3 Z = Mat4ColumnGet(R, 2);
			v3 CapsuleCenter = JointWorldP + 0.4f*Z;

			v3 Min = V3(0.0f, Radius, 0.0f);
			v3 Max = V3(0.0f, Length - Radius, 0.0f);
			capsule Capsule = CapsuleMinMaxRadius(Min, Max, Radius);

			// NOTE(Justin): The debug capsule is an upright capsule. The orientation of the capsule of a weapon in tpose is lying flat. So
			// the first rotation applied is a 90 degree CCW turn about the x-axis so that the capsule is lying flat. From there the joint rotation 
			// is applied and the final rotation rotates the capsule so that it is orientated towrds the entities facing direction.
			R = QuaternionToMat4(Entity->Orientation)*D.R*Mat4XRotation(DegreeToRad(-90.0f));

			DebugDrawCapsule(CapsuleCenter, V3(0.0f), R, Mat4Scale(1.0f), Capsule);
		}
	}
}

internal void
DebugDrawAnimationPlayer(animation_player *AnimationPlayer)
{
	if(!AnimationPlayer)
	{
		return;
	}

	UiIndentAdd();

	char Buff[256];
	MovementStateToString(Buff, AnimationPlayer->MovementState);
	DebugDrawString(Buff);

	mat4 T = Mat4Translate(AnimationPlayer->EntityPLockedAt);
	mat4 R = Mat4Identity();
	mat4 S = Mat4Scale(0.5f);
	PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Sphere, T*R*S, V3(1.0f, 0.0f, 0.0f));

	for(animation *Animation = AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		DebugDrawString(CString(Animation->Name));
		DebugDrawFloat("FrameRate: ", Animation->Info->FrameRate);
		DebugDrawFloat("Duration: ", Animation->Duration);
		DebugDrawFloat("BlendDuration: ", Animation->BlendDuration);
		DebugDrawFloat("t: ", Animation->CurrentTime);
		DebugDrawFloat("t_blend: ", Animation->BlendCurrentTime);
		DebugDrawFloat("t_scale: ", Animation->TimeScale);
		DebugDrawFloat("weight: ", Animation->BlendFactor);
		DebugDrawVector3("LockedP: ", AnimationPlayer->EntityPLockedAt);

		v3 Color = V3(1.0f);
		if(AnimationPlayer->ControlsPosition)
		{
			Color = V3(1.0f, 1.0f, 0.0f);
		}

		DebugDrawVector3("RootMotion: ", AnimationPlayer->RootMotionAccumulator, Color);
		DebugDrawVector3("RootVelocity: ", AnimationPlayer->RootVelocityAccumulator, Color);

		UiAdvanceLine();
	}

	UiIndentAdd(-Ui.DefaultIndent);
}

internal void
DebugDrawHandAndFoot(entity *Entity)
{
	mat4 T = ModelToWorldTransform(Entity->P, Entity->Orientation, Entity->VisualScale);
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
DebugRenderToTexture(game_state *GameState)
{
	//
	// NOTE(Justin): Render to texture
	//

	// TODO(Justin); Cleanup
	mat4 Persp = Mat4Perspective(GameState->FOV, 256.0f/256.0f, GameState->ZNear, GameState->ZFar);
	render_buffer *RenderToTextureBuffer = RenderBufferAllocate(Ui.TempArena, Megabyte(64),
			GameState->CameraTransform,
			Persp,
			Ui.Assets,
			GameState->Camera.P,
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

internal void
DebugDrawTexture(char *TextureName, f32 Width = 128.0f, f32 Height = 128.0f)
{
	asset_entry Entry = FindTexture(Ui.Assets, TextureName);
	if(!Entry.Texture)
	{
		return;
	}

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

	PushTexture(Ui.RenderBuffer, Entry.Texture, Entry.Index);
	PushQuad2D(Ui.RenderBuffer, (f32 *)Vertices, Entry.Index);
}

internal void
DebugDrawMeshTPoseAABB(entity *Entity)
{
	model *Model = Entity->AnimationPlayer->Model;
	ModelTPose(Model);

	f32 C = Entity->VisualScale.x;
	mat4 T = Mat4Translate(Entity->P);
	mat4 R = QuaternionToMat4(Entity->Orientation);
	mat4 S = Mat4Scale(Entity->VisualScale);
	mat4 ModelToWorld = T*R*S;

	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		aabb BoundingBox = Mesh->BoundingBox;
		v3 Center = AABBCenter(BoundingBox);
		Center = ModelToWorld*Center;
		T = Mat4Translate(Center);

		v3 Dim = AABBDim(BoundingBox);
		v3 Scale = C * Dim;
		S = Mat4Scale(Scale);

		PushDebugVolume(Ui.RenderBuffer, &Ui.Assets->Cube, T*R*S, V3(1.0f));
	}
}


