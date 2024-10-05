
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

internal void
AnimationPlayerInitialize(animation_player *AnimationPlayer, model *Model, memory_arena *PermArena)
{
	if(Model)
	{
		AnimationPlayer->State = AnimationState_Idle;
		AnimationPlayer->PermArena = PermArena;
		AnimationPlayer->AnimationPreviouslyAdded = 0;
		AnimationPlayer->AnimationPreviouslyFreed = 0;

		AnimationPlayer->CurrentTime = 0.0f;
		AnimationPlayer->dt = 0.0f;

		// TODO(Justin): Skeleton reference...
		// The blended animations array is an array of key_frames. One key_frame for each mesh. Each key_frame is a blend of all the currently playing
		// animations for that mesh.
		AnimationPlayer->BlendedAnimations = PushArray(AnimationPlayer->PermArena, Model->MeshCount, key_frame);
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *Animation = AnimationPlayer->BlendedAnimations + MeshIndex;
			mesh *Mesh = Model->Meshes + MeshIndex;
			Animation->Positions = PushArray(AnimationPlayer->PermArena, Mesh->JointCount, v3);
			Animation->Orientations = PushArray(AnimationPlayer->PermArena, Mesh->JointCount, quaternion);
			Animation->Scales = PushArray(AnimationPlayer->PermArena, Mesh->JointCount, v3);
		}

		AnimationPlayer->Model = Model;
		AnimationPlayer->IsInitialized = true;
	}
}

internal void
AnimationPlay(animation_player *AnimationPlayer, animation *NewAnimation, u32 Flags)
{
	Assert(AnimationPlayer->IsInitialized);
	for(animation *Current = AnimationPlayer->AnimationPreviouslyAdded;
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
	for(animation *Current = AnimationPlayer->AnimationPreviouslyAdded;
			Current;
			Current = Current->Next)
	{
		Current->BlendDuration = 0.2f;
		Current->BlendingOut = true;
		Current->BlendCurrentTime = 0.0f;
	}

	if(!AnimationPlayer->AnimationPreviouslyFreed)
	{
		AnimationPlayer->AnimationPreviouslyFreed = PushStruct(AnimationPlayer->PermArena, animation);
		AnimationPlayer->AnimationPreviouslyFreed->Next = 0;
	}

	animation *Animation = AnimationPlayer->AnimationPreviouslyFreed;
	AnimationPlayer->AnimationPreviouslyFreed = Animation->Next;

	Animation->Flags = 0;
	FlagAdd(Animation, Flags);

	Animation->CurrentTime = 0.0f;
	Animation->OldTime = 0.0f;
	Animation->TimeScale = 1.0f;

	Animation->BlendFactorLast = 0.0f;
	Animation->BlendDuration = 0.2f;
	Animation->BlendCurrentTime = 0.0f;
	Animation->BlendingIn = true;
	Animation->BlendingOut = false;

	Animation->ID = NewAnimation->ID;
	Animation->Info = NewAnimation->Info;
	Animation->Next = AnimationPlayer->AnimationPreviouslyAdded;
	AnimationPlayer->AnimationPreviouslyAdded = Animation;
	AnimationPlayer->PlayingCount++;
}

internal void
AnimationPlayerUpdate(animation_player *AnimationPlayer, memory_arena *TempArena, f32 dt)
{
	Assert(AnimationPlayer->IsInitialized);

	// Init temporary memory. Used for an array of interpolated poses for each key frame.
	temporary_memory AnimationMemory = TemporaryMemoryBegin(TempArena);
	key_frame *BlendedPoses = PushArray(TempArena, AnimationPlayer->PlayingCount, key_frame);
	u32 BlendedPoseIndex = 0;

	for(animation **AnimationPtr = &AnimationPlayer->AnimationPreviouslyAdded; *AnimationPtr;)
	{
		animation *Animation = *AnimationPtr;
		animation_info *Info = Animation->Info;
		if(Info)
		{
			// Update current animation timeline.
			Animation->OldTime = Animation->CurrentTime;
			Animation->CurrentTime += dt;

			if((Animation->OldTime != Animation->CurrentTime) &&
			   (Animation->CurrentTime >= Info->Duration))
			{
				if(!FlagIsSet(Animation, AnimationFlags_Looping))
				{
					FlagAdd(Animation, AnimationFlags_Finished);
					Animation->CurrentTime = Info->Duration;
				}

				Animation->CurrentTime -= Info->Duration;
			}

			if(Animation->BlendingIn || Animation->BlendingOut)
			{
				Animation->BlendCurrentTime += dt;
				if((Animation->BlendCurrentTime > Animation->BlendDuration) && (Animation->BlendingOut))
				{
					FlagAdd(Animation, AnimationFlags_Finished);
				}
			}

			AnimationPlayer->CurrentTime += dt;
			AnimationPlayer->dt = dt;

			if(!FlagIsSet(Animation, AnimationFlags_Finished))
			{
				// Allocate JointCount number of elements for the blended pose.
				key_frame *BlendedPose = BlendedPoses + BlendedPoseIndex++;
				BlendedPose->Positions = PushArray(TempArena, Info->JointCount, v3);
				BlendedPose->Orientations = PushArray(TempArena, Info->JointCount, quaternion);
				BlendedPose->Scales = PushArray(TempArena, Info->JointCount, v3);

				f32 tNormalized = Animation->CurrentTime / Info->Duration;
				u32 LastKeyFrameIndex = Info->KeyFrameCount - 1;
				u32 KeyFrameIndex = F32TruncateToS32(tNormalized * (f32)LastKeyFrameIndex);
				//Assert(KeyFrameIndex < LastKeyFrameIndex);

				f32 DtPerKeyFrame = Info->Duration / (f32)LastKeyFrameIndex;
				f32 KeyFrameTime = KeyFrameIndex * DtPerKeyFrame;
				f32 t = (Animation->CurrentTime - KeyFrameTime) / DtPerKeyFrame;
				t = Clamp01(t);

				key_frame *KeyFrame = Info->KeyFrames + KeyFrameIndex;
				key_frame *NextKeyFrame = Info->KeyFrames + (KeyFrameIndex + 1);

				sqt RootTransform;
				if(FlagIsSet(Animation, AnimationFlags_RemoveLocomotion))
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
		}
		else
		{
			// TODO(Justin): Stream in animation.
		}

		if(FlagIsSet(Animation, AnimationFlags_Finished))
		{
			*AnimationPtr = Animation->Next;
			Animation->Next = AnimationPlayer->AnimationPreviouslyFreed;
			AnimationPlayer->AnimationPreviouslyFreed = Animation;
			AnimationPlayer->RetiredCount++;
			AnimationPlayer->PlayingCount--;
		}
		else
		{
			AnimationPtr = &Animation->Next;
		}
	}

	Assert(BlendedPoseIndex == AnimationPlayer->PlayingCount);

	// TODO(Justin): Can we not just do this in the loop above???
	// NOTE(Justin): Update blend coefficients.
	for(animation *Animation = AnimationPlayer->AnimationPreviouslyAdded;
			Animation;
			Animation = Animation->Next)
	{
		f32 BlendFactorLast = 1.0f;
		if(Animation->BlendingOut)
		{
			BlendFactorLast = 1.0f - (Animation->BlendCurrentTime / Animation->BlendDuration);
		}
		else if(Animation->BlendingIn)
		{
			BlendFactorLast = (Animation->BlendCurrentTime / Animation->BlendDuration);
		}

		Animation->BlendFactorLast = Clamp01(BlendFactorLast);
	}

	// TODO(Justin): Clean this up..
	// NOTE(Justin): Animation cross fading.

	BlendedPoseIndex = 0;
	b32 First = true;
	for(animation *Animation = AnimationPlayer->AnimationPreviouslyAdded;
			Animation;
			Animation = Animation->Next,
			BlendedPoseIndex++)
	{
		key_frame *BlendedPose = BlendedPoses + BlendedPoseIndex;
		//animation_info *Info = AnimationGet(AssetManager, Animation->ID);
		animation_info *Info = Animation->Info;//Get(AssetManager, Animation->ID);
		if(Info)
		{
			for(u32 MeshIndex = 0; MeshIndex < AnimationPlayer->Model->MeshCount; ++MeshIndex)
			{
				key_frame *BlendedAnimation = AnimationPlayer->BlendedAnimations + MeshIndex;
				mesh *Mesh = AnimationPlayer->Model->Meshes + MeshIndex;
				for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
				{
					joint *Joint = Mesh->Joints + Index;
					s32 JointIndex = JointIndexGet(Info->JointNames, Info->JointCount, Joint->Name);

					if(First)
					{
						if(JointIndex != -1)
						{
							BlendedAnimation->Positions[Index] = BlendedPose->Positions[JointIndex];
							BlendedAnimation->Orientations[Index] = BlendedPose->Orientations[JointIndex];
							BlendedAnimation->Scales[Index] = BlendedPose->Scales[JointIndex];
						}
						else
						{
							// TODO(Justin): Put original joint transform here?
							BlendedAnimation->Positions[Index] = V3(0.0f);
							BlendedAnimation->Orientations[Index] = Quaternion();
							BlendedAnimation->Scales[Index] = V3(0.0f);
						}
					}
					else
					{
						if(JointIndex != -1)
						{
							f32 t = Animation->BlendFactorLast;
							if(t != 1)
							{
								sqt A;
								A.Position = Lerp(BlendedAnimation->Positions[Index], t, BlendedPose->Positions[JointIndex]);
								A.Orientation = LerpShortest(BlendedAnimation->Orientations[Index], t, BlendedPose->Orientations[JointIndex]);
								A.Scale = Lerp(BlendedAnimation->Scales[Index], t, BlendedPose->Scales[JointIndex]);

								BlendedAnimation->Positions[Index] = A.Position;
								BlendedAnimation->Orientations[Index] = A.Orientation;
								BlendedAnimation->Scales[Index] = A.Scale;
							}
							else
							{
								BlendedAnimation->Positions[Index] = BlendedPose->Positions[JointIndex];
								BlendedAnimation->Orientations[Index] = BlendedPose->Orientations[JointIndex];
								BlendedAnimation->Scales[Index] = BlendedPose->Scales[JointIndex];
							}
						}
						else
						{
							// TODO(Justin): Put original joint transform here?
							BlendedAnimation->Positions[Index] = V3(0.0f);
							BlendedAnimation->Orientations[Index] = Quaternion();
							BlendedAnimation->Scales[Index] = V3(0.0f);
						}
					}
				}
			}

			First = false;
		}
	}

	TemporaryMemoryEnd(AnimationMemory);

}

internal void
ModelUpdate(animation_player *AnimationPlayer)
{
	model *Model = AnimationPlayer->Model;
	//animation *Animation = AnimationPlayer->AnimationPreviouslyAdded;
	//animation_info *Info = AnimationGet(Manager, Animation->ID);
	if(Model)
	{
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *BlendedAnimation = AnimationPlayer->BlendedAnimations + MeshIndex;

			mesh *Mesh = Model->Meshes + MeshIndex;
			joint RootJoint = Mesh->Joints[0];
			mat4 RootJointT = RootJoint.Transform;
			mat4 RootInvBind = Mesh->InvBindTransforms[0];

			sqt Xform;
			Xform.Position = BlendedAnimation->Positions[0];
			Xform.Orientation = BlendedAnimation->Orientations[0];
			Xform.Scale = BlendedAnimation->Scales[0];

			if(!Equal(Xform.Position, V3(0.0f)) &&
			   !Equal(Xform.Scale, V3(0.0f)))
			{
				RootJointT = JointTransformFromSQT(Xform);
			}

			Mesh->JointTransforms[0] = RootJointT;
			Mesh->ModelSpaceTransforms[0] = RootJointT * RootInvBind;

			for(u32 Index = 1; Index < Mesh->JointCount; ++Index)
			{
				joint *Joint = Mesh->Joints + Index;
				mat4 JointTransform = Joint->Transform;

				Xform.Position = BlendedAnimation->Positions[Index];
				Xform.Orientation = BlendedAnimation->Orientations[Index];
				Xform.Scale = BlendedAnimation->Scales[Index];

				if(!Equal(Xform.Position, V3(0.0f)) &&
				   !Equal(Xform.Scale, V3(0.0f)))
				{
					JointTransform = JointTransformFromSQT(Xform);
				}

				mat4 ParentTransform = Mesh->JointTransforms[Joint->ParentIndex];
				JointTransform = ParentTransform * JointTransform;
				mat4 InvBind = Mesh->InvBindTransforms[Index];

				Mesh->JointTransforms[Index] = JointTransform;
				Mesh->ModelSpaceTransforms[Index] = JointTransform * InvBind;
			}
		}
	}
}
