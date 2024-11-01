
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
	b32 Result = (Animation->BlendingIn || Animation->BlendingOut);
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
		AnimationPlayer->State = AnimationState_Invalid;
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
			key_frame *FinalPose = AnimationPlayer->FinalPose + MeshIndex;
			mesh *Mesh = Model->Meshes + MeshIndex;
			AllocateJointXforms(AnimationPlayer->Arena, FinalPose, Mesh->JointCount);
		}

		AnimationPlayer->Model = Model;
		AnimationPlayer->IsInitialized = true;
	}
}

internal void
//AnimationPlay(animation_player *AnimationPlayer, animation *NewAnimation, u32 Flags = 0, f32 BlendDuration = 0.0f)
AnimationPlay(animation_player *AnimationPlayer, animation *NewAnimation, f32 BlendDuration = 0.0f)
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

	if(BlendDuration != 0.0f)
	{
		// Blend out currently playing animations
		for(animation *Current = AnimationPlayer->Channels;
				Current;
				Current = Current->Next)
		{
			//Current->BlendDuration = 0.2f;
			Current->BlendDuration = BlendDuration;
			Current->BlendingOut = true;
			Current->BlendCurrentTime = 0.0f;
		}
	}

	if(!AnimationPlayer->FreeChannels)
	{
		AnimationPlayer->FreeChannels = PushStruct(AnimationPlayer->Arena, animation);
		AnimationPlayer->FreeChannels->Next = 0;
	}

	animation *Animation = AnimationPlayer->FreeChannels;
	AnimationPlayer->FreeChannels = Animation->Next;

	Animation->Name = NewAnimation->Name;
	Animation->Flags = NewAnimation->DefaultFlags | AnimationFlags_Playing;
	Animation->CurrentTime = 0.0f;
	Animation->OldTime = 0.0f;
	Animation->TimeScale = 1.0f;

	Animation->BlendFactor = 1.0f;
	Animation->BlendDuration = BlendDuration;
	Animation->BlendCurrentTime = 0.0f;

	if(BlendDuration != 0.0f)
	{
		Animation->BlendingIn = true;
	}
	else
	{
		Animation->BlendingIn = false;
	}

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
			// Done blending out playing animations 
			FlagAdd(Animation, AnimationFlags_Finished);
		}

		if((Animation->BlendCurrentTime > Animation->BlendDuration) && Animation->BlendingIn)
		{
			// Done blending in new animation
			Animation->BlendingIn = false;
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

		sqt RootTransform = JointTransformInterpolatedSQT(KeyFrame, t, NextKeyFrame, 0);
		if(RemoveLocomotion(Animation))
		{
			v3 RootStartP = Info->KeyFrames[0].Positions[0];
			RootTransform.Position.x = RootStartP.x;
			RootTransform.Position.z = RootStartP.z;
		}

		BlendedPose->Positions[0]	 = RootTransform.Position;
		BlendedPose->Orientations[0] = RootTransform.Orientation;
		BlendedPose->Scales[0]		 = RootTransform.Scale;

		for(u32 JointIndex = 1; JointIndex < Info->JointCount; ++JointIndex)
		{
			sqt Transform = JointTransformInterpolatedSQT(KeyFrame, t, NextKeyFrame, JointIndex);
			BlendedPose->Positions[JointIndex]	  = Transform.Position;
			BlendedPose->Orientations[JointIndex] = Transform.Orientation;
			BlendedPose->Scales[JointIndex]		  = Transform.Scale;
		}
	}

	f32 BlendFactor= 1.0f;
	if(Animation->BlendingOut)
	{
		BlendFactor = 1.0f - (Animation->BlendCurrentTime / Animation->BlendDuration);
	}
	else if(Animation->BlendingIn)
	{
		BlendFactor = (Animation->BlendCurrentTime / Animation->BlendDuration);
	}
	else
	{
		BlendFactor = Animation->BlendFactor;
	}

	Animation->BlendFactor = Clamp01(BlendFactor);
}


inline animation *
AnimationGet(game_state *GameState, animation_name Name)
{
	animation *Result = &GameState->Animations[Name];
	return(Result);
}

internal void
Animate(game_state *GameState, animation_player *AnimationPlayer, animation_state State)
{
	// NOTE(Justin): Calling animation play everytime is what allows
	// the animation system to "work" currently. If we only allow an animation to play during a 
	// state change then if a sudden state change happens such as idle -> run -> idle
	// what ends up happening is that the idle and run animation do not complete the blend.
	// Since the blend is not complete both animations are still active. Since both are still active
	// the idle animation is active. if the idle animation is still active and we try adn play another idle 
	// animation then it will return immedialtey. The result is that the original blend between idle and run
	// will complete the blend. When this happens the idle animation drops and we are left with a running animation
	// that keeps looping even though from the game perspective the player is not moving. How do we fix this
	// without having to call animation play everytime? Or is calling animation play everytime an ok solution?
	//
	// Q: Do we force the blend to complete before moving to another animation?
	//  If we play animation that is currently blending with another then we already force the blend to complete before
	//  playing the animation 
	//if(AnimationPlayer->State == State)
	//{
	//	return;
	//}

	animation_state OldState = AnimationPlayer->State;
	AnimationPlayer->State = State;

	switch(AnimationPlayer->State)
	{
		case AnimationState_Idle:
		{
			if(OldState == AnimationState_Invalid)
			{
				AnimationPlay(AnimationPlayer, AnimationGet(GameState, Animation_Idle));
			}
			else
			{
				AnimationPlay(AnimationPlayer, AnimationGet(GameState, Animation_Idle), 0.2f);
			}
		} break;
		case AnimationState_Running:
		{
			AnimationPlay(AnimationPlayer, AnimationGet(GameState, Animation_Run), 0.2f);
		} break;
		case AnimationState_Sprint:
		{
			AnimationPlay(AnimationPlayer, AnimationGet(GameState, Animation_Sprint), 0.2f);
		} break;
	}
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

	// NOTE(Justin): Use scratch pose to do the animation mixing then copy to the final pose.

	model *Model = AnimationPlayer->Model;
	TemporaryMemoryBegin(TempArena);
	key_frame *TempPose = PushArray(TempArena, Model->MeshCount, key_frame);

	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		key_frame *Pose = TempPose + MeshIndex;
		mesh *Mesh = Model->Meshes + MeshIndex;
		AllocateJointXforms(TempArena, Pose, Mesh->JointCount);
		MemoryZero(Mesh->JointCount * sizeof(v3), Pose->Positions);
		MemoryZero(Mesh->JointCount * sizeof(quaternion), Pose->Orientations);
		MemoryZero(Mesh->JointCount * sizeof(v3), Pose->Scales);
		
	}

	f32 FactorSum = 0.0f;
	for(animation *Animation = AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		key_frame *BlendedPose = Animation->BlendedPose;
		animation_info *Info = Animation->Info;
		Assert(Info);

		f32 Factor = Animation->BlendFactor;
		FactorSum += Factor;
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *FinalPose = TempPose + MeshIndex;
			mesh *Mesh = Model->Meshes + MeshIndex;
			for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
			{
				joint *Joint = Mesh->Joints + Index;

				s32 JointIndex = JointIndexGet(Info->JointNames, Info->JointCount, Joint->Name);
				if(JointIndex != -1)
				{
					FinalPose->Positions[Index]		+= Factor * BlendedPose->Positions[JointIndex];

					// TODO(Justin): Pre-process the aniamtions so that the orientations are in the known correct neighborhood.
					quaternion Scaled = Factor * BlendedPose->Orientations[JointIndex];
					if(Dot(Scaled, FinalPose->Orientations[Index]) < 0.0f)
					{
						Scaled *= -1.0f;
					}

					FinalPose->Orientations[Index]	+= Scaled;
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
		key_frame *SrcPose = TempPose + MeshIndex;
		key_frame *DestPose = AnimationPlayer->FinalPose + MeshIndex;
		mesh *Mesh = Model->Meshes + MeshIndex;
		for(u32 JointIndex = 0; JointIndex < Mesh->JointCount; ++JointIndex)
		{
			SrcPose->Positions[JointIndex]		*= Scale;
			SrcPose->Orientations[JointIndex]	*= Scale;
			SrcPose->Scales[JointIndex]			*= Scale;
			SrcPose->Orientations[JointIndex]	= NormalizeOrIdentity(SrcPose->Orientations[JointIndex]);

			DestPose->Positions[JointIndex]		= SrcPose->Positions[JointIndex];
			DestPose->Orientations[JointIndex]	= SrcPose->Orientations[JointIndex];
			DestPose->Scales[JointIndex]		= SrcPose->Scales[JointIndex];
		}
	}
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
