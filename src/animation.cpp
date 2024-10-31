
inline void
FlagAdd(animation *Animation, u32 Flag)
{
	Animation->Flags |= Flag;
}

inline b32 
FlagIsSet(animation *Animation, u32 Flag)
{
	b32 Result = ((Animation->Flags & Flag) != 0);
	return(Result);
}

inline b32
Finished(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_Finished);
	return(Result);
}

inline b32
Looping(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_Looping);
	return(Result);
}

inline b32
CrossFading(animation *Animation)
{
#if 0
	b32 Result = ((FlagIsSet(Animation, AnimationFlags_CrossFadeIn)) ||
				  (FlagIsSet(Animation, AnimationFlags_CrossFadeOut)));
#else
	b32 Result = (Animation->BlendingIn || Animation->BlendingOut);
#endif

	return(Result);
}

inline b32
RemoveLocomotion(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_RemoveLocomotion);
	return(Result);
}

inline mat4
JointTransformFromSQT(sqt SQT)
{
	mat4 Result = Mat4Identity();

	v3 P = SQT.Position;
	mat4 R = QuaternionToMat4(SQT.Orientation);
	v3 Rx = SQT.Scale.x * Mat4ColumnGet(R, 0);
	v3 Ry = SQT.Scale.y * Mat4ColumnGet(R, 1);
	v3 Rz = SQT.Scale.z * Mat4ColumnGet(R, 2);

	Result = Mat4(Rx, Ry, Rz, P);

	return(Result);
}

inline sqt
JointTransformInterpolatedSQT(key_frame *Current, f32 t, key_frame *Next, u32 JointIndex)
{
	sqt Result;

	v3 P1 = Current->Positions[JointIndex];
	quaternion Q1 = Current->Orientations[JointIndex];
	v3 Scale1 = Current->Scales[JointIndex];

	v3 P2 = Next->Positions[JointIndex];
	quaternion Q2 = Next->Orientations[JointIndex];
	v3 Scale2 = Next->Scales[JointIndex];

	v3 P = Lerp(P1, t, P2);
	quaternion Q = LerpShortest(Q1, t, Q2);
	v3 Scale = Lerp(Scale1, t, Scale2);

	Result.Position = P;
	Result.Orientation = Q;
	Result.Scale = Scale;

	return(Result);
}

inline void 
AllocateJointXforms(memory_arena *Arena, key_frame *KeyFrame, u32 JointCount)
{
	KeyFrame->Positions		= PushArray(Arena, JointCount, v3);
	KeyFrame->Orientations	= PushArray(Arena, JointCount, quaternion);
	KeyFrame->Scales		= PushArray(Arena, JointCount, v3);
}

internal void
AnimationPlayerInitialize(animation_player *AnimationPlayer, model *Model, memory_arena *Arena)
{
	if(Model)
	{
		AnimationPlayer->State = AnimationState_Idle;
		AnimationPlayer->Arena = Arena;
		AnimationPlayer->Channels = 0;
		AnimationPlayer->FreeChannels = 0;

		AnimationPlayer->CurrentTime = 0.0f;
		AnimationPlayer->dt = 0.0f;

		// TODO(Justin): Skeleton reference...
		// TODO(Justin): Final pose should just be one key frame with joint count number of P, Q, and S...
		// The blended animations array is an array of key_frames. One key_frame for each mesh. Each key_frame is the final pose
		// , which is a blend of all the currently playing animations, for that mesh.
		AnimationPlayer->FinalPose = PushArray(AnimationPlayer->Arena, Model->MeshCount, key_frame);
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *Animation = AnimationPlayer->FinalPose + MeshIndex;
			mesh *Mesh = Model->Meshes + MeshIndex;
			AllocateJointXforms(AnimationPlayer->Arena, Animation, Mesh->JointCount);
		}

		AnimationPlayer->Model = Model;
		AnimationPlayer->IsInitialized = true;
	}
}

internal void
AnimationPlay(animation_player *AnimationPlayer, animation *NewAnimation, u32 Flags)
{
	Assert(AnimationPlayer->IsInitialized);
	for(animation *Current = AnimationPlayer->Channels;
			Current;
			Current = Current->Next)
	{
		if(Current->ID.Value == NewAnimation->ID.Value)
		{
			// Animation is already playing, so return.
			return;
		}
	}

	// Blend out currently playing animations
	for(animation *Current = AnimationPlayer->Channels;
			Current;
			Current = Current->Next)
	{
		Current->BlendDuration = 0.2f;
		Current->BlendingOut = true;
		Current->BlendCurrentTime = 0.0f;
	}

	if(!AnimationPlayer->FreeChannels)
	{
		AnimationPlayer->FreeChannels = PushStruct(AnimationPlayer->Arena, animation);
		AnimationPlayer->FreeChannels->Next = 0;
	}

	animation *Animation = AnimationPlayer->FreeChannels;
	AnimationPlayer->FreeChannels = Animation->Next;

	Animation->Flags = 0;
	FlagAdd(Animation, Flags);

	Animation->CurrentTime = 0.0f;
	Animation->OldTime = 0.0f;
	Animation->TimeScale = 1.0f;

	Animation->BlendFactor = 1.0f;
	Animation->BlendDuration = 0.2f;
	Animation->BlendCurrentTime = 0.0f;
	//Animation->BlendingIn = true;
	Animation->BlendingIn = false;
	Animation->BlendingOut = false;

	Animation->ID = NewAnimation->ID;
	Animation->Info = NewAnimation->Info;
	Animation->BlendedPose = NewAnimation->BlendedPose;
	Animation->Next = AnimationPlayer->Channels;
	AnimationPlayer->Channels = Animation;
	AnimationPlayer->PlayingCount++;

}

internal void
AnimationUpdate(animation *Animation, f32 dt)
{
	animation_info *Info = Animation->Info;
	if(!Info)
	{
		// TODO(Justin): Stream in animation.
		return;
	}

	Animation->OldTime = Animation->CurrentTime;
	Animation->CurrentTime += dt * Animation->TimeScale;

	if((Animation->OldTime != Animation->CurrentTime) &&
	   (Animation->CurrentTime >= Info->Duration))
	{
		if(!Looping(Animation))
		{
			FlagAdd(Animation, AnimationFlags_Finished);
			Animation->CurrentTime = Info->Duration;
		}

		Animation->CurrentTime -= Info->Duration;
	}

	if(CrossFading(Animation))
	{
		Animation->BlendCurrentTime += dt;
		if((Animation->BlendCurrentTime > Animation->BlendDuration) && Animation->BlendingOut)
		{
			FlagAdd(Animation, AnimationFlags_Finished);
		}
	}

	if(!Finished(Animation))
	{
		key_frame *BlendedPose = Animation->BlendedPose;

		f32 tNormalized = Animation->CurrentTime / Info->Duration;
		u32 LastKeyFrameIndex = Info->KeyFrameCount - 1;
		u32 KeyFrameIndex = F32TruncateToS32(tNormalized * (f32)LastKeyFrameIndex);

		f32 DtPerKeyFrame = Info->Duration / (f32)LastKeyFrameIndex;
		f32 KeyFrameTime = KeyFrameIndex * DtPerKeyFrame;
		f32 t = (Animation->CurrentTime - KeyFrameTime) / DtPerKeyFrame;
		t = Clamp01(t);

		key_frame *KeyFrame = Info->KeyFrames + KeyFrameIndex;
		key_frame *NextKeyFrame = Info->KeyFrames + (KeyFrameIndex + 1);

		sqt RootTransform;
		if(RemoveLocomotion(Animation))
		{
			RootTransform = JointTransformInterpolatedSQT(KeyFrame, t, NextKeyFrame, 0);

			v3 RootStartP = Info->KeyFrames[0].Positions[0];

			RootTransform.Position.x = RootStartP.x;
			RootTransform.Position.z = RootStartP.z;
		}
		else
		{
			RootTransform = JointTransformInterpolatedSQT(KeyFrame, t, NextKeyFrame, 0);
		}

		BlendedPose->Positions[0] = RootTransform.Position;
		BlendedPose->Orientations[0] = RootTransform.Orientation;
		BlendedPose->Scales[0] = RootTransform.Scale;

		for(u32 JointIndex = 1; JointIndex < Info->JointCount; ++JointIndex)
		{
			sqt Transform;
			Transform = JointTransformInterpolatedSQT(KeyFrame, t, NextKeyFrame, JointIndex);
			BlendedPose->Positions[JointIndex] = Transform.Position;
			BlendedPose->Orientations[JointIndex] = Transform.Orientation;
			BlendedPose->Scales[JointIndex] = Transform.Scale;
		}
	}

	f32 BlendFactorLast = 1.0f;
	if(Animation->BlendingOut)
	{
		BlendFactorLast = 1.0f - (Animation->BlendCurrentTime / Animation->BlendDuration);
	}
	else if(Animation->BlendingIn)
	{
		BlendFactorLast = (Animation->BlendCurrentTime / Animation->BlendDuration);
	}
	else if(Animation->BlendingComposite)
	{
		BlendFactorLast = Animation->BlendFactor;
	}

	Animation->BlendFactor = Clamp01(BlendFactorLast);
}


internal void
AnimationPlayerUpdate(animation_player *AnimationPlayer, memory_arena *TempArena, f32 dt)
{
	Assert(AnimationPlayer->IsInitialized);

	AnimationPlayer->CurrentTime += dt;
	AnimationPlayer->TimeInCurrentState += dt;
	AnimationPlayer->dt = dt;

	for(animation **AnimationPtr = &AnimationPlayer->Channels; *AnimationPtr;)
	{
		animation *Animation = *AnimationPtr;
		AnimationUpdate(Animation, AnimationPlayer->dt);
		if(Finished(Animation))
		{
			*AnimationPtr = Animation->Next;
			Animation->Next = AnimationPlayer->FreeChannels;
			AnimationPlayer->FreeChannels = Animation;
			AnimationPlayer->RetiredCount++;
			AnimationPlayer->PlayingCount--;
		}
		else
		{
			AnimationPtr = &Animation->Next;
		}
	}

	// TODO(Justin): Clean this up..
	// NOTE(Justin): Animation cross fading.
#if 1
	b32 First = true;
	for(animation *Animation = AnimationPlayer->Channels;
			Animation;
			Animation = Animation->Next)
	{
		key_frame *BlendedPose = Animation->BlendedPose;
		animation_info *Info = Animation->Info;
		if(Info)
		{
			model *Model = AnimationPlayer->Model;
			for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
			{
				key_frame *FinalPose = AnimationPlayer->FinalPose + MeshIndex;
				mesh *Mesh = Model->Meshes + MeshIndex;
				for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
				{
					joint *Joint = Mesh->Joints + Index;
					s32 JointIndex = JointIndexGet(Info->JointNames, Info->JointCount, Joint->Name);

					if(JointIndex == -1)
					{
						FinalPose->Positions[Index] = V3(0.0f);
						FinalPose->Orientations[Index] = Quaternion();
						FinalPose->Scales[Index] = V3(0.0f);
					}
					else
					{
						if(First)
						{
							FinalPose->Positions[Index] = BlendedPose->Positions[JointIndex];
							FinalPose->Orientations[Index] = BlendedPose->Orientations[JointIndex];
							FinalPose->Scales[Index] = BlendedPose->Scales[JointIndex];
						}
						else
						{
							f32 t = Animation->BlendFactor;
							if(t != 1.0f)
							{
								sqt A;
								A.Position = Lerp(FinalPose->Positions[Index], t, BlendedPose->Positions[JointIndex]);
								A.Orientation = LerpShortest(FinalPose->Orientations[Index], t, BlendedPose->Orientations[JointIndex]);
								A.Scale = Lerp(FinalPose->Scales[Index], t, BlendedPose->Scales[JointIndex]);

								FinalPose->Positions[Index] = A.Position;
								FinalPose->Orientations[Index] = A.Orientation;
								FinalPose->Scales[Index] = A.Scale;
							}
							else
							{
								FinalPose->Positions[Index] = BlendedPose->Positions[JointIndex];
								FinalPose->Orientations[Index] = BlendedPose->Orientations[JointIndex];
								FinalPose->Scales[Index] = BlendedPose->Scales[JointIndex];
							}
						}
					}
				}
			}

			First = false;
		}
	}
#else
	// TODO(Justin): Doing the accumulation we either need to clear the final pose
	// each frame, or use scratch space to do the accumulation then copy to the final pose.

	b32 First = true;
	model *Model = AnimationPlayer->Model;

	TemporaryMemoryBegin(TempArena);
	key_frame *FinalPoseScratch = PushArray(TempArena, Model->MeshCount, key_frame);
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		key_frame *Pose = FinalPoseScratch + MeshIndex;
		mesh *Mesh = Model->Meshes + MeshIndex;
		Pose->Positions = PushArray(TempArena, Mesh->JointCount, v3);
		Pose->Orientations = PushArray(TempArena, Mesh->JointCount, v3);
		Pose->Scales = PushArray(TempArena, Mesh->JointCount, v3);
	}


	f32 FactorSum = 0.0f;
	for(animation *Animation = AnimationPlayer->Channels;
			Animation;
			Animation = Animation->Next)
	{
		key_frame *BlendedPose = Animation->BlendedPose;
		animation_info *Info = Animation->Info;
		Assert(Info);

		f32 Factor = Animation->BlendFactor;
		FactorSum += Factor;
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *FinalPose = AnimationPlayer->FinalPose + MeshIndex;
			mesh *Mesh = Model->Meshes + MeshIndex;
			for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
			{
				joint *Joint = Mesh->Joints + Index;

				s32 JointIndex = JointIndexGet(Info->JointNames, Info->JointCount, Joint->Name);
				if(JointIndex == -1)
				{
					FinalPose->Positions[Index]		+= V3(0.0f);
					FinalPose->Orientations[Index]	+= Quaternion();
					FinalPose->Scales[Index]		+= V3(0.0f);
				}
				else
				{
					FinalPose->Positions[Index]		+= Factor * BlendedPose->Positions[JointIndex];
					FinalPose->Orientations[Index]	+= Factor * BlendedPose->Orientations[JointIndex];
					FinalPose->Scales[Index]		+= Factor * BlendedPose->Scales[JointIndex];
				}
			}
		}
	}

	f32 Scale = 1.0f;
	if(FactorSum)
	{
		Scale = 1.0f / FactorSum;
	}

	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		key_frame *FinalPose = AnimationPlayer->FinalPose + MeshIndex;
		mesh *Mesh = Model->Meshes + MeshIndex;
		for(u32 JointIndex = 0; JointIndex < Mesh->JointCount; ++JointIndex)
		{
			FinalPose->Positions[JointIndex]	*= Scale;
			FinalPose->Orientations[JointIndex]	*= Scale;
			FinalPose->Scales[JointIndex]		*= Scale;

			FinalPose->Orientations[JointIndex] = NormalizeOrIdentity(FinalPose->Orientations[JointIndex]);
		}
	}

#endif
}

internal void
ModelUpdate(animation_player *AnimationPlayer)
{
	model *Model = AnimationPlayer->Model;
	if(Model)
	{
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *FinalPose = AnimationPlayer->FinalPose + MeshIndex;

			mesh *Mesh = Model->Meshes + MeshIndex;
			joint RootJoint = Mesh->Joints[0];
			mat4 RootJointT = RootJoint.Transform;
			mat4 RootInvBind = Mesh->InvBindTransforms[0];

			sqt Xform;
			Xform.Position = FinalPose->Positions[0];
			Xform.Orientation = FinalPose->Orientations[0];
			Xform.Scale = FinalPose->Scales[0];

			if(!Equal(Xform.Position, V3(0.0f)) &&
			   !Equal(Xform.Scale, V3(0.0f)))
			{
				RootJointT = JointTransformFromSQT(Xform);
			}

			Mesh->JointTransforms[0] = RootJointT;
			Mesh->ModelSpaceTransforms[0] = RootJointT * RootInvBind;

			for(u32 JointIndex = 1; JointIndex < Mesh->JointCount; ++JointIndex)
			{
				joint *Joint = Mesh->Joints + JointIndex;
				mat4 JointTransform = Joint->Transform;

				Xform.Position = FinalPose->Positions[JointIndex];
				Xform.Orientation = FinalPose->Orientations[JointIndex];
				Xform.Scale = FinalPose->Scales[JointIndex];

				if(!Equal(Xform.Position, V3(0.0f)) &&
				   !Equal(Xform.Scale, V3(0.0f)))
				{
					JointTransform = JointTransformFromSQT(Xform);
				}

				mat4 ParentTransform = Mesh->JointTransforms[Joint->ParentIndex];
				JointTransform = ParentTransform * JointTransform;
				mat4 InvBind = Mesh->InvBindTransforms[JointIndex];

				Mesh->JointTransforms[JointIndex] = JointTransform;
				Mesh->ModelSpaceTransforms[JointIndex] = JointTransform * InvBind;
			}
		}
	}
}
