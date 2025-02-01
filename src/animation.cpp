
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
Playing(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_Playing);
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

inline b32
MaskingJoints(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_JointMask);
	return(Result);
}

inline b32
ControlsPosition(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_ControlsPosition);
	return(Result);
}

inline b32
ControlsTurning(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlags_ControlsTurning);
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
InterpolatedSQT(key_frame *Current, f32 t, key_frame *Next, u32 JointIndex)
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
	if(!AnimationPlayer || !Model)
	{
		return;
	}

	AnimationPlayer->MovementState = MovementState_Idle;
	AnimationPlayer->Arena = Arena;
	AnimationPlayer->Channels = 0;
	AnimationPlayer->FreeChannels = 0;
	AnimationPlayer->CurrentTime = 0.0f;
	AnimationPlayer->dt = 0.0f;
	AnimationPlayer->RootMotionAccumulator = {};
	AnimationPlayer->RootTurningAccumulator = {};

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

// TODO(Justin): Really think through the correct way to handle trying to play the same animation multiple times
// If the current animation is being blended in or out we probably dont want to 
// want to start playing the same animation..

// TODO(Justin): How do we blend if current is blending in?
// TODO(Justin): How do we blend if current is blending out?

internal void
AnimationPlay(animation_player *AnimationPlayer, animation_info *SampledAnimation, u32 ID, u32 AnimationFlags,
		f32 BlendDuration = 0.0f,
		f32 StartTime = 0.0f,
		f32 TimeScale = 1.0f)
{
	Assert(AnimationPlayer->IsInitialized);

	for(animation *Current = AnimationPlayer->Channels; Current; Current = Current->Next)
	{
		if(Current->ID.Value == ID)
		{
			return;
		}
	}

	if(!AnimationPlayer->FreeChannels)
	{
		AnimationPlayer->FreeChannels = PushStruct(AnimationPlayer->Arena, animation);
		AnimationPlayer->FreeChannels->Next = 0;
	}

	animation *Animation = AnimationPlayer->FreeChannels;
	AnimationPlayer->FreeChannels = Animation->Next;

	for(animation *Current = AnimationPlayer->Channels; Current; Current = Current->Next)
	{

		if(!Current->BlendingOut)
		{
			Current->BlendDuration = BlendDuration;
			Current->BlendCurrentTime = 0.0f;
			Current->BlendingOut = true;
			Current->BlendingIn = false;
			Current->TimeScale = TimeScale;
		}
	}

	Animation->Name = SampledAnimation->Name;
	Animation->Flags = AnimationFlags | AnimationFlags_Playing;
	Animation->Duration = SampledAnimation->Duration;
	Animation->CurrentTime = StartTime;
	Animation->OldTime = 0.0f;
	Animation->TimeScale = TimeScale;
	Animation->BlendFactor = 1.0f;
	Animation->BlendDuration = BlendDuration;
	Animation->BlendCurrentTime = 0.0f;
	Animation->BlendingIn = true;
	Animation->BlendingOut = false;

	Animation->ID.Value = ID;
	Animation->Info = SampledAnimation;
	Animation->BlendedPose = SampledAnimation->ReservedForChannel;
	Animation->Next = AnimationPlayer->Channels;

	Animation->MotionDeltaPerFrame = {};
	Animation->TurningDeltaPerFrame = {};

	AnimationPlayer->Channels = Animation;
	AnimationPlayer->PlayingCount++;
}

inline f32
SmoothStep(f32 CurrentTime, f32 Duration)
{
	f32 t01 = CurrentTime / Duration;
	f32 Result = 3.0f * Square(t01) - 2.0f * Cube(t01);
	return(Result);
}

internal void
AnimationUpdate(animation *Animation, f32 dt)
{
	animation_info *Info = Animation->Info;
	Assert(Info);

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
		Animation->BlendCurrentTime += dt * Animation->TimeScale;
		if((Animation->BlendCurrentTime > Animation->BlendDuration) && Animation->BlendingOut)
		{
			FlagAdd(Animation, AnimationFlags_Finished);
		}

		if((Animation->BlendCurrentTime > Animation->BlendDuration) && Animation->BlendingIn)
		{
			Animation->BlendingIn = false;
		}
	}

	if(!Finished(Animation))
	{
		key_frame *BlendedPose = Animation->BlendedPose;

		f32 t01 = Animation->CurrentTime / Info->Duration;
		u32 LastKeyFrameIndex = Info->KeyFrameCount - 1;
		u32 KeyFrameIndex = F32TruncateToS32(t01 * (f32)LastKeyFrameIndex);

		f32 DtPerKeyFrame = Info->Duration / (f32)LastKeyFrameIndex;
		f32 KeyFrameTime = KeyFrameIndex * DtPerKeyFrame;
		f32 t = (Animation->CurrentTime - KeyFrameTime) / DtPerKeyFrame;
		t = Clamp01(t);

		key_frame *KeyFrame		= Info->KeyFrames + KeyFrameIndex;
		key_frame *NextKeyFrame = Info->KeyFrames + (KeyFrameIndex + 1);

		sqt RootTransform = InterpolatedSQT(KeyFrame, t, NextKeyFrame, 0);
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
			sqt Transform = InterpolatedSQT(KeyFrame, t, NextKeyFrame, JointIndex);
			BlendedPose->Positions[JointIndex]	  = Transform.Position;
			BlendedPose->Orientations[JointIndex] = Transform.Orientation;
			BlendedPose->Scales[JointIndex]		  = Transform.Scale;
		}
	}

	f32 BlendFactor = 1.0f;
	if(Animation->BlendingOut)
	{
		BlendFactor = 1.0f - SmoothStep(Animation->BlendCurrentTime, Animation->BlendDuration);
	}
	else if(Animation->BlendingIn)
	{
		BlendFactor = SmoothStep(Animation->BlendCurrentTime, Animation->BlendDuration);
	}
	else
	{
		BlendFactor = Animation->BlendFactor;
	}

	Animation->BlendFactor = Clamp01(BlendFactor);
}

// NOTE(Justin): Nodes of the graph represent the animation states
// NOTE(Justin): BlendDuration and TimeOffset are control parameters of the arc that connects
// the current node to the node that is being switched too.
// NOTE(Justin): Animation flags are now defined at the Node level. When we switch to the current node
// the node itself contains the animation flags and these are passed as arguements when calling AnimationPlay()
// TODO(Justin): Use the animation state name as a tag. Right now the tag is the actual name of the animation...
// TODO(Justin): Decide when, where, and how the new state should be accepted. Since Animate() is the interface 
// between the game and animation system it should probably be done at that level instead of here where it is hidden...
// Moreoever there can be many animation states associated to one movement state??

internal void
SwitchToNode(asset_manager *AssetManager, animation_player *AnimationPlayer, animation_graph *Graph, string Dest, f32 ArcBlendDuration, f32 ArcTimeOffset)
{
	for(u32 Index = 0; Index < Graph->NodeCount; ++Index)
	{
		animation_graph_node *Node = Graph->Nodes + Index;
		if(StringsAreSame(Node->Name, Dest))
		{
			Graph->CurrentNode = *Node;
			Graph->Index = Index;
			break;
		}
	}

	animation_graph_node *Node = &Graph->CurrentNode;
	asset_entry Entry = LookupSampledAnimation(AssetManager, CString(Node->Tag));
	if(Entry.SampledAnimation)
	{
		AnimationPlay(AnimationPlayer, Entry.SampledAnimation, Entry.Index, Node->AnimationFlags, ArcBlendDuration, ArcTimeOffset, Node->TimeScale);
	}
}

// NOTE(Justin): We send a message and do work to determine what animation should be played
// and how to start playiang it. Therefore at this step the new animation has not been
// determined and is not playing. This means that we should look at the most recent animation
// to determine what the next animation should be. This animation is
// the animation at the top of the linked list.

internal void
MessageSend(asset_manager *AssetManager, animation_player *AnimationPlayer, animation_graph *Graph, char *Message)
{
	string Dest = {};

	animation_graph_node *Node = &Graph->CurrentNode;

	f32 DefaultBlendDuration = 0.2f;
	f32 DefaultTimeOffset = 0.0f;
	for(u32 ArcIndex = 0; ArcIndex < Node->ArcCount; ++ArcIndex)
	{
		animation_graph_arc *Arc = Node->Arcs + ArcIndex;
		if(StringsAreSame(Arc->Message, Message))
		{
			b32 ShouldSend = true;
			if(Arc->Type == ArcType_TimeInterval)
			{
				animation *Animation = AnimationPlayer->Channels;
				f32 t = Animation->CurrentTime;
				if((t < Arc->t0) || (t > Arc->t1))
				{
					ShouldSend = false;
				}

				if(Arc->RemainingTimeBeforeCrossFade != 0.0f)
				{
					f32 RemainingTime = Animation->Duration - t;
					if(RemainingTime > Arc->RemainingTimeBeforeCrossFade)
					{
						ShouldSend = false;
					}
				}
			}

			if(ShouldSend)
			{
				Dest = Arc->Destination;
				DefaultTimeOffset = Arc->StartTime;
				if(Arc->BlendDurationSet)
				{
					DefaultBlendDuration = Arc->BlendDuration;
				}
				break;
			}
		}
	}

	if(Dest.Size != 0)
	{
		AnimationPlayer->MovementState = AnimationPlayer->NewState;
		SwitchToNode(AssetManager, AnimationPlayer, Graph, Dest, DefaultBlendDuration, DefaultTimeOffset);
	}
}

internal void
Animate(entity *Entity, asset_manager *AssetManager)
{
	animation_graph *Graph = Entity->AnimationGraph;
	animation_player *AnimationPlayer = Entity->AnimationPlayer;

	if(!Graph || !AnimationPlayer)
	{
		return;
	}

	// Right now the animation player lags behind the gameplay state. Meaning if the gameplay
	// state says sprint and the player was running. The animation state stays in running
	// for some time until it can transition to the sprint state. Only when the animation
	// state has switched do we acceps the new state. Not sure if this is a good idea or not.

	AnimationPlayer->NewState = Entity->MovementState;
	movement_state OldState = AnimationPlayer->MovementState;
	movement_state State	= AnimationPlayer->NewState;
	switch(State)
	{
		case MovementState_Idle:
		{
			// TODO(Justin): Transition to idle!
			char *Message = "go_state_idle";
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		case MovementState_Crouch:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_crouch");
		} break;
		case MovementState_Run:
		{
			char *Message = "go_state_run";

			f32 dTheta = Entity->ThetaTarget - Entity->Theta;
			if(dTheta < -30.0f)
			{
				Message = "go_state_turn_90_right_to_run";
			}
			if(dTheta > 30.0f)
			{
				Message = "go_state_turn_90_left_to_run";
			}

			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		case MovementState_Sprint:
		{
			char *Message = "go_state_sprint";
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		case MovementState_Jump:
		{
			//MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_jump");
		} break;
		case MovementState_Sliding:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_slide");
		} break;
		case MovementState_Attack:
		{
			char *Message = "go_state_neutral_attack";
			if(Entity->Attack.Type == AttackType_Forward)
			{
				Message = "go_state_forward_attack";
			}
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
	}
}

internal void
AnimationPlayerUpdate(animation_player *AnimationPlayer, memory_arena *TempArena, f32 dt)
{
	if(!AnimationPlayer || !AnimationPlayer->IsInitialized)
	{
		return;
	}

	AnimationPlayer->CurrentTime += dt;
	AnimationPlayer->dt = dt;

	//
	// NOTE(Justin): Update each channel.
	//

	AnimationPlayer->ControlsPosition = false;
	AnimationPlayer->ControlsTurning = false;
	for(animation **AnimationPtr = &AnimationPlayer->Channels; *AnimationPtr;)
	{
		animation *Animation = *AnimationPtr;

		if(ControlsTurning(Animation))
		{
			if(!Animation->BlendingOut)
			{
				AnimationPlayer->ControlsTurning = true;
			}
		}

		if(ControlsPosition(Animation))
		{
			AnimationPlayer->ControlsPosition = true;
		}


		Animation->MotionDeltaPerFrame = {};
		Animation->TurningDeltaPerFrame = 0.0f;

		v3 OldP = Animation->BlendedPose->Positions[0];
		quaternion OldQ = Animation->BlendedPose->Orientations[0];

		AnimationUpdate(Animation, AnimationPlayer->dt);

		v3 NewP = Animation->BlendedPose->Positions[0];
		quaternion NewQ = Animation->BlendedPose->Orientations[0];

		Animation->MotionDeltaPerFrame += NewP - OldP;
		Animation->TurningDeltaPerFrame = AngleBetween(NewQ, OldQ);

		// TODO(Justin): Is this the correct location to remove finished 
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

	//
	// NOTE(Justin): Use scratch arena to mix all channels, then copy to the final pose.
	//

	model *Model = AnimationPlayer->Model;
	key_frame *TempPose = PushArray(TempArena, Model->MeshCount, key_frame);

	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		key_frame *Pose = TempPose + MeshIndex;
		mesh *Mesh = Model->Meshes + MeshIndex;
		AllocateJointXforms(TempArena, Pose, Mesh->JointCount);
		MemoryZero(Pose->Positions, Mesh->JointCount * sizeof(v3));
		MemoryZero(Pose->Orientations, Mesh->JointCount * sizeof(quaternion));
		MemoryZero(Pose->Scales, Mesh->JointCount * sizeof(v3));
	}

	f32 FactorSum = 0.0f;
	for(animation *Animation = AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		Assert(!Finished(Animation));

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
					FinalPose->Positions[Index]	+= Factor * BlendedPose->Positions[JointIndex];
					FinalPose->Scales[Index]	+= Factor * BlendedPose->Scales[JointIndex];

					// TODO(Justin): Pre-process the animations so that the orientations are in the known correct neighborhood.
					quaternion Scaled = Factor * BlendedPose->Orientations[JointIndex];
					if(Dot(Scaled, FinalPose->Orientations[Index]) < 0.0f)
					{
						Scaled *= -1.0f;
					}

					FinalPose->Orientations[Index] += Scaled;
				}
			}
		}

		if(AnimationPlayer->ControlsPosition)
		{
			AnimationPlayer->RootMotionAccumulator += Factor * Animation->MotionDeltaPerFrame;
		}

		if(ControlsTurning(Animation))
		{
			AnimationPlayer->RootTurningAccumulator += Factor * Animation->TurningDeltaPerFrame;
		}
	}

	f32 Scale = 1.0f;
	if(FactorSum)
	{
		Scale = 1.0f / FactorSum;
	}

	if(AnimationPlayer->ControlsPosition)
	{
		AnimationPlayer->RootMotionAccumulator *= Scale;
	}

	if(AnimationPlayer->ControlsTurning)
	{
		AnimationPlayer->RootTurningAccumulator *= Scale;
	}

	//
	// NOTE(Justin): Scale the mixed channels, then copy.
	//

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
ModelTPose(model *Model)
{
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;

		joint RootJoint = Mesh->Joints[0];
		mat4 RootJointT = RootJoint.Transform;
		mat4 RootInvBind = Mesh->InvBindTransforms[0];

		Mesh->JointTransforms[0] = RootJointT;
		Mesh->ModelSpaceTransforms[0] = RootJointT * RootInvBind;

		for(u32 JointIndex = 1; JointIndex < Mesh->JointCount; ++JointIndex)
		{
			joint *Joint = Mesh->Joints + JointIndex;
			mat4 JointTransform = Joint->Transform;

			mat4 ParentTransform = Mesh->JointTransforms[Joint->ParentIndex];
			JointTransform = ParentTransform * JointTransform;
			mat4 InvBind = Mesh->InvBindTransforms[JointIndex];

			Mesh->JointTransforms[JointIndex] = JointTransform;
			Mesh->ModelSpaceTransforms[JointIndex] = JointTransform * InvBind;
		}
	}
}

internal void
ModelJointsUpdate(entity *Entity)
{
	animation_player *AnimationPlayer = Entity->AnimationPlayer;
	if(AnimationPlayer && AnimationPlayer->Model)
	{
		model *Model = AnimationPlayer->Model;
		for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
		{
			key_frame *FinalPose = AnimationPlayer->FinalPose + MeshIndex;

			mesh *Mesh = Model->Meshes + MeshIndex;
			joint RootJoint = Mesh->Joints[0];
			mat4 RootJointT = RootJoint.Transform;
			mat4 RootInvBind = Mesh->InvBindTransforms[0];

			sqt Xform;
			Xform.Position		= FinalPose->Positions[0];
			Xform.Orientation	= FinalPose->Orientations[0];
			Xform.Scale			= FinalPose->Scales[0];

			// The final set of transforms are in model space. All these positions
			// get converted to world space in the shader. These means the root is converted to
			// world space and is at the gameplay position. Therfore if doing gameplay driven animation
			// all we need to do is remove the xz translation, since the root position is going to be translated there,
			// and keep the up and down motion by using the y value of the root of the mixed pose

			if(AnimationPlayer->ControlsPosition)
			{
				// NOTE(Justin): This has the affect of removing xz translation while
				// keeping up and down motion.
				//Xform.Position = V3(0.0f, FinalPose->Positions[0].y,  0.0f);
			}

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

				Xform.Position		= FinalPose->Positions[JointIndex];
				Xform.Orientation	= FinalPose->Orientations[JointIndex];
				Xform.Scale			= FinalPose->Scales[JointIndex];

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

		// TODO(Justin): Store transform.
		mat4 Transform	= EntityTransform(Entity, Entity->VisualScale);

		mat4 LeftFootT	= Model->Meshes[0].JointTransforms[Model->LeftFootJointIndex];
		mat4 RightFootT = Model->Meshes[0].JointTransforms[Model->RightFootJointIndex];

		mat4 LeftHandT	= Model->Meshes[0].JointTransforms[Model->LeftHandJointIndex];
		mat4 RightHandT = Model->Meshes[0].JointTransforms[Model->RightHandJointIndex];

		v3 LeftFootModelP	= Mat4ColumnGet(LeftFootT, 3);
		v3 RightFootModelP	= Mat4ColumnGet(RightFootT, 3);

		v3 LeftHandModelP	= Mat4ColumnGet(LeftHandT, 3);
		v3 RightHandModelP	= Mat4ColumnGet(RightHandT, 3);

		Entity->LeftFootP	= Transform*LeftFootModelP;
		Entity->RightFootP	= Transform*RightFootModelP;

		Entity->LeftHandP	= Transform*LeftHandModelP;
		Entity->RightHandP	= Transform*RightHandModelP;
	}
}

internal void
AnimationGraphPerFrameUpdate(asset_manager *AssetManager, animation_player *AnimationPlayer,
														  animation_graph *Graph)
{
	if(!AnimationPlayer)
	{
		return;
	}
	// NOTE(Justin): This does not work 100% in the current system. The oldest is not necessarily the same
	// as the current node. The oldest may be another animation that is currently being blended out
	// whild the current node is blending in. So any work that is done is invalid.
	//Find the channel playing the currently active animation.
	//animation *Oldest = AnimationOldestGet(AnimationPlayer);

	// Should this just be the current node?
	animation *MostRecent = AnimationPlayer->Channels;
	if(!MostRecent)
	{
		return;
	}

	f32 RemainingTime = MostRecent->Info->Duration - MostRecent->CurrentTime;
	f32 DefaultTimeOffset = 0.0f;

	animation_graph_node *Node = &Graph->CurrentNode;
	animation_graph_arc Arc = Node->WhenDone;
	if((Arc.Destination.Size != 0) && (RemainingTime <= Arc.RemainingTimeBeforeCrossFade))
	{
		AnimationPlayer->MovementState = AnimationPlayer->NewState;

		f32 BlendDuration = Clamp01(RemainingTime);
		DefaultTimeOffset = Arc.StartTime;
		SwitchToNode(AssetManager, AnimationPlayer, Graph, Arc.Destination, BlendDuration, DefaultTimeOffset);
	}
}

internal animation_graph_node * 
AnimationGraphNodeAdd(animation_graph *Graph, char *Name)
{
	Assert(Graph->NodeCount < ArrayCount(Graph->Nodes));
	animation_graph_node *Node = Graph->Nodes + Graph->NodeCount;
	Node->Name = StringCopy(&Graph->Arena, Name);
	Node->Index = Graph->NodeCount;
	Node->AnimationFlags = 0;
	Node->TimeScale = 1.0f;
	Node->WhenDone = {};

	return(Node);
}

inline void
NodeBegin(animation_graph *Graph, char *NodeName)
{
	AnimationGraphNodeAdd(Graph, NodeName);
	Graph->CurrentNode = Graph->Nodes[Graph->NodeCount];
	Graph->Index = Graph->NodeCount;
}

inline void
NodeEnd(animation_graph *Graph)
{
	Graph->NodeCount++;
	Graph->Index = 0;
	Graph->CurrentNode = Graph->Nodes[0];
}

// TODO(Justin): This is dangerous as if the arguements get passed in the incorrect order
// then bad things can happen... Also there is no size checking on the buffer
internal void
BufferLine(u8 **Content, u8 *Buffer)
{
	u32 At = 0;
	while(!IsNewLine(**Content))
	{
		Buffer[At++] = **Content;
		(*Content)++;
	}
	Buffer[At] = '\0';
}

internal void
AdvanceLine(u8 **Content)
{
	while(IsNewLine(**Content))
	{
		(*Content)++;
	}
}

// TODO(Jusitn): Need to validate that the animations specified in the
// graph are correct.

// TODO(Jusitn): Fix parsing bug. A space, " ", after the state name
// does not parse correctly.

// TODO(Jusitn): Error handling
internal void 
AnimationGraphInitialize(animation_graph *G, char *FileName)
{
	debug_file File = Platform.DebugFileReadEntire(FileName);

	if(File.Size == 0)
	{
		return;
	}

	u8 *Content = (u8 *)File.Content;
	u8 Buffer_[4096];
	u8 *Buffer = &Buffer_[0];
	MemoryZero(Buffer, sizeof(Buffer));
	u32 At = 0;
	b32 ProcessingNode = false;
	while(*Content)
	{
		BufferLine(&Content, Buffer);
		AdvanceLine(&Content);

		switch(Buffer[0])
		{
			case ' ':
			case '\r':
			case '\n':
			case '\t':
			case '#':
			{
				// Comment, do nothing.
			} break;
			case '[':
			{
				u32 Version = U32FromASCII(&Buffer[1]);
				Assert(Version == 1);
			} break;
			case ':':
			{
				if(ProcessingNode)
				{
					NodeEnd(G);
					ProcessingNode = false;
				}

				EatUntilSpace(&Buffer);
				EatSpaces(&Buffer);
				NodeBegin(G, (char *)Buffer);
				ProcessingNode = true;
				BufferLine(&Content, Buffer);
			} break;
		}

		if(ProcessingNode)
		{
			char *Word = strtok((char *)Buffer, " ");

			animation_graph_node *Node = &G->Nodes[G->Index];
			if(StringsAreSame(Word, "message"))
			{
				Assert(Node->ArcCount < ArrayCount(Node->Arcs));
				animation_graph_arc *Arc = Node->Arcs + Node->ArcCount++;

				char *InBoundMessage = strtok(0, " ");
				char *DestNodeName = strtok(0, " ");
				char *Param = strtok(0, " ");

				Arc->Destination = StringCopy(&G->Arena, DestNodeName);
				Arc->Message	 = StringCopy(&G->Arena, InBoundMessage);
				Arc->Type		 = ArcType_None;

				f32 t0 = 0.0f;
				f32 t1 = 0.0f;
				f32 StartTime = 0.0f;
				f32 BlendDuration = 0.0f;

				while(Param)
				{
					if(StringsAreSame("t_start", Param))
					{
						Param = strtok(0, " ");
						StartTime = F32FromASCII(Param);
						Arc->StartTime = StartTime;
					}
					else if(StringsAreSame("t_blend", Param))
					{
						Param = strtok(0, " ");
						BlendDuration = F32FromASCII(Param);
						Arc->BlendDuration = BlendDuration;
						Arc->BlendDurationSet = true;
					}
					else if(StringsAreSame("t_interval", Param))
					{
						Param = strtok(0, " ");
						t0 = F32FromASCII(Param);
						Param = strtok(0, " ");
						t1 = F32FromASCII(Param);

						Arc->Type = ArcType_TimeInterval;
						Arc->t0 = t0;
						Arc->t1 = t1;
					}

					Param = strtok(0, " ");
				}
			}
			else if(StringsAreSame(Word, "when_done"))
			{
				animation_graph_arc *Arc = &Node->WhenDone;

				char *InBoundMessage = strtok(0, " ");
				char *DestNodeName = strtok(0, " ");
				char *Param = strtok(0, " ");

				Arc->Destination = StringCopy(&G->Arena, DestNodeName);
				Arc->Message	 = StringCopy(&G->Arena, InBoundMessage);
				Arc->Type		 = ArcType_WhenDone;

				while(Param)
				{
					if(StringsAreSame("t_remaining", Param))
					{
						Param = strtok(0, " ");
						Arc->RemainingTimeBeforeCrossFade = F32FromASCII(Param);
					}
					if(StringsAreSame("t_start", Param))
					{
						Param = strtok(0, " ");
						Arc->StartTime = F32FromASCII(Param);
					}

					Param = strtok(0, " ");
				}
			}
			else if(StringsAreSame(Word, "animation"))
			{
				// TODO(Justin): This should be a tag. E.g. idle not the actual name of the animaation?
				char *Param = strtok(0, " ");
				Node->Tag = StringCopy(&G->Arena, Param);
			}
			else if(StringsAreSame(Word, "controls_position"))
			{
				Node->AnimationFlags |= AnimationFlags_ControlsPosition;
			}
			else if(StringsAreSame(Word, "controls_turning"))
			{
				Node->AnimationFlags |= AnimationFlags_ControlsTurning;
			}
			else if(StringsAreSame(Word, "looping"))
			{
				Node->AnimationFlags |= AnimationFlags_Looping;
			}
			else if(StringsAreSame(Word, "remove_locomotion"))
			{
				Node->AnimationFlags |= AnimationFlags_RemoveLocomotion;
			}
			else if(StringsAreSame(Word, "t_scale"))
			{
				char *Param = strtok(0, " ");
				Node->TimeScale = F32FromASCII(Param);
			}

			else if(*Word == '#')
			{
				// Comment, do nothing.
			}

			AdvanceLine(&Content);
		}
	}
	NodeEnd(G);
}
