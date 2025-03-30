
inline void
FlagAdd(animation *Animation, u32 Flag)
{
	Animation->Flags |= Flag;
}

inline void
FlagClear(animation *Animation, u32 Flag)
{
	Animation->Flags &= ~Flag;
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
	b32 Result = FlagIsSet(Animation, AnimationFlag_Playing);
	return(Result);
}

inline b32
Finished(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlag_Finished);
	return(Result);
}

inline b32
Looping(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlag_Looping);
	return(Result);
}

inline b32
BlendingInOrOut(animation *Animation)
{
	b32 Result = (Animation->BlendingIn || Animation->BlendingOut);
	return(Result);
}

inline b32
RemoveLocomotion(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlag_RemoveLocomotion);
	return(Result);
}

inline b32
ControlsPosition(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlag_ControlsPosition);
	return(Result);
}

inline b32
ControlsTurning(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlag_ControlsTurning);
	return(Result);
}

inline b32
CompletedPlayback(animation *Animation)
{
	b32 Result = FlagIsSet(Animation, AnimationFlag_CompletedCycle);
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

	Assert(Model->Version == 2);
	AnimationPlayer->FinalPose = PushArray(AnimationPlayer->Arena, 1, key_frame);
	AllocateJointXforms(AnimationPlayer->Arena, AnimationPlayer->FinalPose, Model->JointCount);
	AnimationPlayer->Model = Model;
	AnimationPlayer->IsInitialized = true;
}

internal void
AnimationPlay(animation_player *AnimationPlayer, animation_info *SampledAnimation, u32 ID, u32 AnimationFlags,
																						f32 BlendDuration = 0.0f,
																						f32 StartTime = 0.0f,
																						f32 TimeScale = 1.0f)
{
	Assert(AnimationPlayer->IsInitialized);

	//
	// NOTE(Justin): Do not repeatedly play the same animation
	//

	for(animation *Current = AnimationPlayer->Channels; Current; Current = Current->Next)
	{
		if(Current->ID.Value == ID)
		{
			return;
		}
	}

	//
	// NOTE(Justin): Allocate a new channel 
	//

	if(!AnimationPlayer->FreeChannels)
	{
		AnimationPlayer->FreeChannels = PushStruct(AnimationPlayer->Arena, animation);
		AnimationPlayer->FreeChannels->Next = 0;
	}

	animation *Animation = AnimationPlayer->FreeChannels;
	AnimationPlayer->FreeChannels = Animation->Next;

	//
	// NOTE(Justin): Clear previous blended pose. This is required for root motion to work properly
	//

	// TODO(Justin): Should we use temporary memory to allocate the blended pose that is the output
	// of a channel? Or keep it pre-allocated when a sample animation is loaded and then once the animation
	// is finished playing, clear the blended pose?

	key_frame *BlendedPoseOutput = SampledAnimation->ReservedForChannel;
	MemoryZero(BlendedPoseOutput->Positions, SampledAnimation->JointCount * sizeof(v3));
	MemoryZero(BlendedPoseOutput->Orientations, SampledAnimation->JointCount * sizeof(quaternion));
	MemoryZero(BlendedPoseOutput->Scales, SampledAnimation->JointCount * sizeof(v3));

	//
	// NOTE(Justin): Set all current channels to start blending out
	//

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
	Animation->Flags = (AnimationFlags | AnimationFlag_Playing);
	Animation->Duration = SampledAnimation->Duration;
	Animation->CurrentTime = StartTime;
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
	animation_info *Samples = Animation->Info;
	Assert(Samples);

	Animation->CurrentTime += dt * Animation->TimeScale;

	if(Animation->CurrentTime >= Samples->Duration)
	{
		if(!Looping(Animation))
		{
			FlagAdd(Animation, AnimationFlag_Finished);
			Animation->CurrentTime = Samples->Duration;
		}

		Animation->CurrentTime -= Samples->Duration;
	}

	if(BlendingInOrOut(Animation))
	{
		Animation->BlendCurrentTime += dt * Animation->TimeScale;

		if((Animation->BlendCurrentTime >= Animation->BlendDuration))
		{
			if(Animation->BlendingOut)
			{
				FlagAdd(Animation, AnimationFlag_Finished);
				Animation->BlendingOut = false;
			}

			if(Animation->BlendingIn)
			{
				Animation->BlendingIn = false;
			}
		}
	}

	if(!Finished(Animation))
	{
		key_frame *BlendedPose = Animation->BlendedPose;

		f32 t01 = Animation->CurrentTime / Samples->Duration;
		u32 LastKeyFrameIndex = Samples->KeyFrameCount - 1;
		u32 KeyFrameIndex = F32TruncateToS32(t01 * (f32)LastKeyFrameIndex);
		Assert(KeyFrameIndex != LastKeyFrameIndex);

		f32 DtPerKeyFrame = Samples->Duration / (f32)LastKeyFrameIndex;
		f32 KeyFrameTime = KeyFrameIndex * DtPerKeyFrame;
		f32 t = (Animation->CurrentTime - KeyFrameTime) / DtPerKeyFrame;
		t = Clamp01(t);

		key_frame *KeyFrame		= Samples->KeyFrames + KeyFrameIndex;
		key_frame *NextKeyFrame = Samples->KeyFrames + (KeyFrameIndex + 1);
		sqt RootTransform = InterpolatedSQT(KeyFrame, t, NextKeyFrame, 0);

		if(RemoveLocomotion(Animation))
		{
			v3 RootStartP = Samples->KeyFrames[0].Positions[0];
			RootTransform.Position.x = RootStartP.x;
			RootTransform.Position.z = RootStartP.z;
		}

		BlendedPose->Positions[0]	 = RootTransform.Position;
		BlendedPose->Orientations[0] = RootTransform.Orientation;
		BlendedPose->Scales[0]		 = RootTransform.Scale;

		for(u32 JointIndex = 1; JointIndex < Samples->JointCount; ++JointIndex)
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
	animation_graph_node DestNode = {};
	u32 DestIndex = 0;
	for(u32 Index = 0; Index < Graph->NodeCount; ++Index)
	{
		animation_graph_node *Node = Graph->Nodes + Index;
		if(StringsAreSame(Node->Name, Dest))
		{
			DestNode = *Node;
			DestIndex = Index;
			break;
		}
	}

	asset_entry Entry = FindAnimation(AssetManager, CString(DestNode.Tag));
	if(Entry.SampledAnimation)
	{
		AnimationPlay(AnimationPlayer, Entry.SampledAnimation, Entry.Index, DestNode.AnimationFlags, ArcBlendDuration, ArcTimeOffset, DestNode.TimeScale);
	}

	//
	// NOTE(Justin): Update the current node of the graph iff the animation was successfully played.
	//

	animation *JustAdded = AnimationPlayer->Channels;
	if(StringsAreSame(JustAdded->Name, DestNode.Tag))
	{
		Graph->CurrentNode = DestNode;
		Graph->Index = DestIndex;
		AnimationPlayer->MovementState = AnimationPlayer->NewState;
	}
}

// NOTE(Justin): Determine the next animation and when/how to start playing it
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
				if(!Animation)
				{
					// TODO(Justin): This should never happen in production
					SwitchToNode(AssetManager, AnimationPlayer, Graph, Graph->Nodes[0].Name, DefaultBlendDuration, DefaultTimeOffset);
					return;
				}

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

				if(Arc->StartTime == -1.0f)
				{
					DefaultTimeOffset = AnimationPlayer->Channels->CurrentTime;
				}

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

	if(!AnimationPlayer->ControlsPosition)
	{
		AnimationPlayer->EntityPLockedAt = Entity->P;
	}

	if(AnimationPlayer->ControlsPosition && AnimationPlayer->UpdateLockedP)
	{
		AnimationPlayer->EntityPLockedAt = Entity->P;
		AnimationPlayer->UpdateLockedP = false;
	}

	AnimationPlayer->NewState = Entity->MovementState;
	movement_state OldState = AnimationPlayer->MovementState;
	movement_state State	= AnimationPlayer->NewState;
	switch(State)
	{
		case MovementState_Idle:
		{
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
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		case MovementState_Sprint:
		{
			char *Message = "go_state_sprint";
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		case MovementState_Jump:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_jump");
		} break;
		case MovementState_InAir:
		{
			char *Message = "go_state_falling";
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		case MovementState_Land:
		{
			char *Message = "go_state_fall_to_land";
			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;

		case MovementState_Sliding:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_slide");
		} break;
		case MovementState_Attack:
		{
			char *Message = "go_state_neutral_attack";
			if((Entity->AttackType == AttackType_Neutral1) ||
			   (Entity->AttackType == AttackType_Neutral2) ||
			   (Entity->AttackType == AttackType_Neutral3))
			{
				Message = "go_state_neutral_attack";
			}
			if(Entity->AttackType == AttackType_Sprint)
			{
				Message = "go_state_sprint_attack";
			}

			MessageSend(AssetManager, AnimationPlayer, Graph, Message);
		} break;
		InvalidDefaultCase;
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

		if(ControlsPosition(Animation))
		{
			if(!Animation->BlendingOut)
			{
				AnimationPlayer->ControlsPosition = true;
			}
		}

		v3 OldP = Animation->BlendedPose->Positions[0];
		if(Equal(OldP, V3(0.0f)))
		{
			OldP = Animation->Info->KeyFrames[0].Positions[0];
		}

		AnimationUpdate(Animation, AnimationPlayer->dt);
		v3 NewP = Animation->BlendedPose->Positions[0];
		Animation->RootMotionDeltaPerFrame = NewP - OldP;
		Animation->RootVelocityDeltaPerFrame = dt * Animation->RootMotionDeltaPerFrame;

		if(Animation->Flags & AnimationFlag_IgnoreYMotion)
		{
			Animation->RootMotionDeltaPerFrame.y = 0.0f;
			Animation->RootVelocityDeltaPerFrame.y = 0.0f;
		}

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
	Assert(Model->Version == 2);

	key_frame *TempPose = PushArray(TempArena, 1, key_frame);
	AllocateJointXforms(TempArena, TempPose, Model->JointCount);
	MemoryZero(TempPose->Positions, Model->JointCount * sizeof(v3));
	MemoryZero(TempPose->Orientations, Model->JointCount * sizeof(quaternion));
	MemoryZero(TempPose->Scales, Model->JointCount * sizeof(v3));

	f32 FactorSum = 0.0f;
	for(animation *Animation = AnimationPlayer->Channels; Animation; Animation = Animation->Next)
	{
		Assert(!Finished(Animation));

		key_frame *BlendedPose = Animation->BlendedPose;
		animation_info *Samples = Animation->Info;

		f32 Factor = Animation->BlendFactor;
		FactorSum += Factor;
		for(u32 Index = 0; Index < Model->JointCount; ++Index)
		{
			joint *Joint = Model->Joints + Index;
			s32 JointIndex = JointIndexGet(Samples->JointNames, Samples->JointCount, Joint->Name);
			if(JointIndex != -1)
			{
				v3 P = Factor * BlendedPose->Positions[JointIndex];
				if(ControlsPosition(Animation) && Animation->BlendingOut && (JointIndex == 0))
				{
					P = Factor*V3(0.0f, BlendedPose->Positions[JointIndex].y, 0.0f);
				}

				TempPose->Positions[Index]	+= P;
				TempPose->Scales[Index]	+= Factor * BlendedPose->Scales[JointIndex];

				// TODO(Justin): Pre-process the animations so that the orientations are in the known correct neighborhood.
				quaternion Scaled = Factor * BlendedPose->Orientations[JointIndex];
				if(Dot(Scaled, TempPose->Orientations[Index]) < 0.0f)
				{
					Scaled *= -1.0f;
				}

				TempPose->Orientations[Index] += Scaled;
			}
		}

		if(AnimationPlayer->ControlsPosition)
		{
			AnimationPlayer->RootMotionAccumulator += Animation->RootMotionDeltaPerFrame;
			AnimationPlayer->RootVelocityAccumulator += Animation->RootVelocityDeltaPerFrame;
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
		AnimationPlayer->RootVelocityAccumulator *= Scale;
	}

	//
	// NOTE(Justin): Scale the mixed channels, then copy.
	//

	key_frame *SrcPose = TempPose;
	key_frame *DestPose = AnimationPlayer->FinalPose;
	for(u32 JointIndex = 0; JointIndex < Model->JointCount; ++JointIndex)
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

internal void
ModelTPose(model *Model)
{
	Assert(Model->Version == 2);

	joint RootJoint = Model->Joints[0];
	mat4 RootJointT = RootJoint.Transform;
	mat4 RootInvBind = Model->InvBindTransforms[0];

	Model->JointTransforms[0] = RootJointT;
	Model->ModelSpaceTransforms[0] = RootJointT * RootInvBind;

	for(u32 JointIndex = 1; JointIndex < Model->JointCount; ++JointIndex)
	{
		joint *Joint = Model->Joints + JointIndex;
		mat4 JointTransform = Joint->Transform;

		mat4 ParentTransform = Model->JointTransforms[Joint->ParentIndex];
		JointTransform = ParentTransform * JointTransform;
		mat4 InvBind = Model->InvBindTransforms[JointIndex];

		Model->JointTransforms[JointIndex] = JointTransform;
		Model->ModelSpaceTransforms[JointIndex] = JointTransform * InvBind;
	}
}

internal void
ModelJointsUpdate(entity *Entity)
{
	animation_player *AnimationPlayer = Entity->AnimationPlayer;
	if(AnimationPlayer && AnimationPlayer->Model)
	{
		model *Model = AnimationPlayer->Model;
		Assert(Model->Version == 2);

		key_frame *FinalPose = AnimationPlayer->FinalPose;
		joint RootJoint = Model->Joints[0];
		mat4 RootJointT = RootJoint.Transform;
		mat4 RootInvBind = Model->InvBindTransforms[0];

		sqt Xform;
		Xform.Position		= FinalPose->Positions[0];
		Xform.Orientation	= FinalPose->Orientations[0];
		Xform.Scale			= FinalPose->Scales[0];

		if(!Equal(Xform.Position, V3(0.0f)) &&
				!Equal(Xform.Scale, V3(0.0f)))
		{
			RootJointT = JointTransformFromSQT(Xform);
		}

		Model->JointTransforms[0] = RootJointT;
		Model->ModelSpaceTransforms[0] = RootJointT * RootInvBind;

		for(u32 JointIndex = 1; JointIndex < Model->JointCount; ++JointIndex)
		{
			joint *Joint = Model->Joints + JointIndex;
			mat4 JointTransform = Joint->Transform;

			Xform.Position		= FinalPose->Positions[JointIndex];
			Xform.Orientation	= FinalPose->Orientations[JointIndex];
			Xform.Scale			= FinalPose->Scales[JointIndex];

			if(!Equal(Xform.Position, V3(0.0f)) &&
					!Equal(Xform.Scale, V3(0.0f)))
			{
				JointTransform = JointTransformFromSQT(Xform);
			}

			mat4 ParentTransform = Model->JointTransforms[Joint->ParentIndex];
			JointTransform = ParentTransform * JointTransform;
			mat4 InvBind = Model->InvBindTransforms[JointIndex];

			Model->JointTransforms[JointIndex] = JointTransform;
			Model->ModelSpaceTransforms[JointIndex] = JointTransform * InvBind;
		}

		// TODO(Justin): Store transform.
		// TODO(Justin): Handle root motion case.

		mat4 Transform	= EntityTransform(Entity, Entity->VisualScale);

		mat4 LeftFootT	= Model->JointTransforms[Model->LeftFootJointIndex];
		mat4 RightFootT = Model->JointTransforms[Model->RightFootJointIndex];

		mat4 LeftHandT	= Model->JointTransforms[Model->LeftHandJointIndex];
		mat4 RightHandT = Model->JointTransforms[Model->RightHandJointIndex];

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

	animation *MostRecent = AnimationPlayer->Channels;
	if(!MostRecent)
	{
		return;
	}

	f32 RemainingTime = MostRecent->Info->Duration - MostRecent->CurrentTime;
	f32 DefaultTimeOffset = 0.0f;

	animation_graph_node *Node = &Graph->CurrentNode;
	animation_graph_arc Arc = Node->WhenDone;

	if(Node->Collidert0 != 0.0f)
	{
		f32 t = MostRecent->CurrentTime;
		if((t >= Node->Collidert0) && (t <= Node->Collidert1))
		{
			AnimationPlayer->SpawnAttackCollider = true;
		}
		else
		{
			AnimationPlayer->SpawnAttackCollider = false;
		}
	}

	if((Arc.Destination.Size != 0) && (RemainingTime <= Arc.RemainingTimeBeforeCrossFade))
	{
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

// TODO(Jusitn): Need to validate that the animations specified in the
// graph are correct.

// TODO(Jusitn): Fix parsing bug. A space, " ", after the state name
// does not parse correctly.

// TODO(Jusitn): Error handling
// TODO(Jusitn): Robustness

internal void 
AnimationGraphInitialize(animation_graph *G, char *FileName)
{
	debug_file File = Platform.DebugFileReadEntire(FileName);

	if(File.Size == 0)
	{
		return;
	}

	G->Path = StringCopy(&G->Arena, FileName);

	char Name[256];
	MemoryZero(Name, sizeof(Name));
	FileNameFromFullPath(FileName, Name);
	G->Name = StringCopy(&G->Arena, Name);

	u8 *Content = (u8 *)File.Content;

	u8 LineBuffer_[4096];
	MemoryZero(LineBuffer_, sizeof(LineBuffer_));
	u8 *LineBuffer = &LineBuffer_[0];

	u8 Word_[4096];
	MemoryZero(Word_, sizeof(Word_));
	u8 *Word = &Word_[0];

	b32 ProcessingNode = false;
	while(*Content)
	{
		BufferLine(&Content, LineBuffer);
		u8 *Line = LineBuffer;
		BufferNextWord(&Line, Word);

		switch(Word[0])
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
				u32 Version = U32FromASCII(&Word[1]);
			} break;
			case ':':
			{
				if(ProcessingNode)
				{
					NodeEnd(G);
					ProcessingNode = false;
				}

				EatUntilSpace(&Line);
				EatSpaces(&Line);
				NodeBegin(G, (char *)Line);
				ProcessingNode = true;
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
			} break;
		}

		if(ProcessingNode)
		{
			animation_graph_node *Node = &G->Nodes[G->Index];
			if(StringsAreSame(Word, "message"))
			{
				Assert(Node->ArcCount < ArrayCount(Node->Arcs));
				animation_graph_arc *Arc = Node->Arcs + Node->ArcCount++;

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Arc->Message = StringCopy(&G->Arena, Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Arc->Destination = StringCopy(&G->Arena, Word);

				Arc->Type = ArcType_None;

				while(*Line)
				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);

					if(StringsAreSame("t_start", Word))
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						Arc->StartTime = F32FromASCII(Word);
					}
					else if(StringsAreSame("t_blend", Word))
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						Arc->BlendDuration = F32FromASCII(Word);
						Arc->BlendDurationSet = true;
					}
					else if(StringsAreSame("t_interval", Word))
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						Arc->t0 = F32FromASCII(Word);

						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						Arc->t1 = F32FromASCII(Word);

						Arc->Type = ArcType_TimeInterval;
					}
				}
			}
			else if(StringsAreSame(Word, "when_done"))
			{
				animation_graph_arc *Arc = &Node->WhenDone;

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Arc->Message = StringCopy(&G->Arena, Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Arc->Destination = StringCopy(&G->Arena, Word);

				Arc->Type = ArcType_WhenDone;

				while(*Line)
				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);

					if(StringsAreSame("t_remaining", Word))
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						Arc->RemainingTimeBeforeCrossFade = F32FromASCII(Word);
					}
					if(StringsAreSame("t_start", Word))
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						Arc->StartTime = F32FromASCII(Word);
					}
				}
			}
			// TODO(Justin): This should be a tag. E.g. idle not the actual name of the animaation?
			else if(StringsAreSame(Word, "animation"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Node->Tag = StringCopy(&G->Arena, Word);
			}
			else if(StringsAreSame(Word, "t_scale"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Node->TimeScale = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "collider_interval"))
			{
				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Node->Collidert0 = F32FromASCII(Word);

				EatSpaces(&Line);
				BufferNextWord(&Line, Word);
				Node->Collidert1 = F32FromASCII(Word);
			}
			else if(StringsAreSame(Word, "controls_position"))
			{
				Node->AnimationFlags |= AnimationFlag_ControlsPosition;
			}
			else if(StringsAreSame(Word, "controls_turning"))
			{
				Node->AnimationFlags |= AnimationFlag_ControlsTurning;
			}
			else if(StringsAreSame(Word, "looping"))
			{
				Node->AnimationFlags |= AnimationFlag_Looping;
			}
			else if(StringsAreSame(Word, "remove_locomotion"))
			{
				Node->AnimationFlags |= AnimationFlag_RemoveLocomotion;
			}
			else if(StringsAreSame(Word, "ignore_y_motion"))
			{
				Node->AnimationFlags |= AnimationFlag_IgnoreYMotion;
			}
			else if(*Word == '#')
			{
				// Comment, do nothing.
			}
		}

		AdvanceLine(&Content);
	}

	NodeEnd(G);

	Platform.DebugFileFree(File.Content);
}

internal void
AnimationGraphReload(asset_manager *AssetManager, char *GraphName)
{
	animation_graph *G = FindGraph(AssetManager, GraphName).Graph;
	string Path = G->Path;
	ArenaClear(&G->Arena);
	G->NodeCount = 0;
	G->Index = 0;
	G->CurrentNode = {};
	MemoryZero(&G->Nodes, sizeof(G->Nodes));
	AnimationGraphInitialize(G, CString(Path));
}
