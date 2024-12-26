
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
		AnimationPlayer->MovementState = MovementState_Idle;
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
AnimationPlay(animation_player *AnimationPlayer, animation *NewAnimation,
		f32 BlendDuration = 0.0f,
		f32 TimeOffset = 0.0f,
		b32 MaskingJoints = false,
		b32 BlendingComposite = false)
{
	Assert(AnimationPlayer->IsInitialized);

	if(!AnimationPlayer->FreeChannels)
	{
		AnimationPlayer->FreeChannels = PushStruct(AnimationPlayer->Arena, animation);
		AnimationPlayer->FreeChannels->Next = 0;
	}

	animation *Animation = AnimationPlayer->FreeChannels;
	AnimationPlayer->FreeChannels = Animation->Next;

	for(animation *Current = AnimationPlayer->Channels; Current; Current = Current->Next)
	{
		if(Current->ID.Value == NewAnimation->ID.Value)
		{
			// TODO(Justin): Really think through the correct way to handle this....
		}
	}

	// Blend out currently playing animations
	if(BlendDuration != 0.0f)
	{
		for(animation *Current = AnimationPlayer->Channels; Current; Current = Current->Next)
		{
			// TODO(Justin): How do we blend if current is blending in?
			// TODO(Justin): How do we blend if current is blending out?
			if(!Current->BlendingOut)
			{
				Current->BlendDuration = BlendDuration;
				Current->BlendCurrentTime = 0.0f;
				Current->BlendingOut = true;
				Current->BlendingIn = false;
			}
		}
	}

	Animation->Name = NewAnimation->Name;
	Animation->Flags = NewAnimation->DefaultFlags | AnimationFlags_Playing;
	Animation->Duration = NewAnimation->Info->Duration;
	Animation->CurrentTime = TimeOffset;
	Animation->OldTime = 0.0f;
	Animation->TimeScale = NewAnimation->TimeScale;
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
	Animation->BlendingComposite = BlendingComposite;
	Animation->MaskingJoints = MaskingJoints;

	Animation->JointMasks = NewAnimation->JointMasks;

	Animation->ID = NewAnimation->ID;
	Animation->Info = NewAnimation->Info;
	Animation->BlendedPose = NewAnimation->BlendedPose;
	Animation->Next = AnimationPlayer->Channels;

	AnimationPlayer->Channels = Animation;
	AnimationPlayer->PlayingCount++;
}

// TODO(Justin): Better cubic?
inline f32
EaseFactor(f32 CurrentTime, f32 Duration)
{
	f32 tNormal = CurrentTime / Duration;
	f32 Result = 3.0f * Square(tNormal) - 2.0f * Cube(tNormal);
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
		Animation->BlendCurrentTime += dt;
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

	f32 BlendFactor = 1.0f;
	if(Animation->BlendingOut)
	{
		BlendFactor = 1.0f - EaseFactor(Animation->BlendCurrentTime, Animation->BlendDuration);
	}
	else if(Animation->BlendingIn)
	{
		BlendFactor = EaseFactor(Animation->BlendCurrentTime, Animation->BlendDuration);
	}
	else
	{
		BlendFactor = Animation->BlendFactor;
	}

	Animation->BlendFactor = Clamp01(BlendFactor);
}

internal void
SwitchToNode(asset_manager *AssetManager, animation_player *AnimationPlayer,
										  animation_graph *Graph, string Dest, f32 BlendDuration)
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

	// TODO(Justin): Asset manager and create a table lookup. Use the animation state name as a tag.
	// Right now the tag is the actual name of the animation...


	animation_graph_node *Node = &Graph->CurrentNode;
	animation *Animation = LookupAnimation(AssetManager, (char *)Node->Tag.Data);
	if(Animation)
	{
		AnimationPlay(AnimationPlayer, Animation, BlendDuration);
	}

	AnimationPlayer->MovementState = AnimationPlayer->NewState;
}

internal animation *
AnimationOldestGet(animation_player *AnimationPlayer)
{
	animation *Result = 0;

	for(animation *Current = AnimationPlayer->Channels; Current; Current = Current->Next)
	{
		if(!Current->Next)
		{
			Result = Current;
		}
	}

	return(Result);
}

internal void
MessageSend(asset_manager *AssetManager, animation_player *AnimationPlayer, animation_graph *Graph, char *Message)
{
	animation_graph_node *Node = &Graph->CurrentNode;
	string Dest = {};
	f32 DefaultBlendDuration = 0.2f;
	for(u32 ArcIndex = 0; ArcIndex < Node->ArcCount; ++ArcIndex)
	{
		animation_graph_arc *Arc = Node->Arcs + ArcIndex;
		if(StringsAreSame(Arc->Message, Message))
		{
			b32 ShouldSend = true;
			if(Arc->Type == ArcType_TimeInterval)
			{
				// NOTE(Justin): We send a message and do work to determine what animation should be played
				// and how to start playiang it. Therefore at this step the new animation has not been
				// determined and is not playing. This means that we should look at the most recent animation
				// to determine what the next animation should be. This animation is
				// the animation at the top of the linked list.
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
		SwitchToNode(AssetManager, AnimationPlayer, Graph, Dest, DefaultBlendDuration);
	}
}

internal void
Animate(entity *Entity, asset_manager *AssetManager)
{
	animation_graph *Graph = Entity->AnimationGraph;
	animation_player *AnimationPlayer = Entity->AnimationPlayer;
	Assert(Graph);
	Assert(AnimationPlayer);

	if(AnimationPlayer->PlayingCount == 0)
	{
		AnimationPlay(AnimationPlayer, LookupAnimation(AssetManager, "XBot_IdleRight"), 0.2f);
		return;
	}

	movement_state State = Entity->MovementState;
	if(AnimationPlayer->MovementState == State && Equal(Entity->dTheta, 0.0f))
	{
		return;
	}

	AnimationPlayer->NewState = State;
	switch(State)
	{
		case MovementState_Idle:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_idle");
		} break;
		case MovementState_Crouch:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_crouch");
		} break;
		case MovementState_Run:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_run");
		} break;
		case MovementState_Sprint:
		{
#if 1
			if(Entity->dTheta > 90.0f)
			{
				// Turning left
				MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_sprint_to_180");
			}
			else if(Entity->dTheta < -90.0f)
			{
				// Turning right
				MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_sprint_to_180");
			}
			else
			{
				MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_sprint");
			}
#else
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_sprint");
#endif
		} break;
		case MovementState_Jump:
		{
			MessageSend(AssetManager, AnimationPlayer, Graph, "go_state_jump");
		} break;
	}
}

internal void
AnimationPlayerUpdate(animation_player *AnimationPlayer, memory_arena *TempArena, f32 dt)
{
	Assert(AnimationPlayer->IsInitialized);

	AnimationPlayer->CurrentTime += dt;
	AnimationPlayer->dt = dt;

	//
	// NOTE(Justin): Update each channel.
	//

	AnimationPlayer->ControlsPosition = false;
	for(animation **AnimationPtr = &AnimationPlayer->Channels; *AnimationPtr;)
	{
		animation *Animation = *AnimationPtr;

#if 0
		if(ControlsPosition(Animation))
		{
			//v3 OldP = Animation->BlendedPose->Positions[0];
			AnimationUpdate(Animation, AnimationPlayer->dt);
			//v3 NewP = Animation->BlendedPose->Positions[0];
			//AnimationPlayer->AnimationDelta += NewP - OldP;
		}
		else
		{
			AnimationUpdate(Animation, AnimationPlayer->dt);
		}
#else
		if(ControlsPosition(Animation))
		{
			AnimationPlayer->ControlsPosition = true;
		}
		AnimationUpdate(Animation, AnimationPlayer->dt);
#endif



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
	// NOTE(Justin): Use scratch pose to mix all channels into, then copy to the final pose.
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
		b32 Masking = false;
		if(MaskingJoints(Animation))
		{
			Masking = true;
		}

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
					b32 ShouldMixJoint = Masking ? Animation->JointMasks[JointIndex] : 1;
					if(ShouldMixJoint)
					{
						FinalPose->Positions[Index]	+= Factor * BlendedPose->Positions[JointIndex];
						FinalPose->Scales[Index]	+= Factor * BlendedPose->Scales[JointIndex];

						// TODO(Justin): Pre-process the aniamtions so that the orientations are in the known correct neighborhood.
						quaternion Scaled = Factor * BlendedPose->Orientations[JointIndex];
						if(Dot(Scaled, FinalPose->Orientations[Index]) < 0.0f)
						{
							Scaled *= -1.0f;
						}

						FinalPose->Orientations[Index] += Scaled;
					}
					else
					{
						int breakhere = 0;
					}
				}
			}
		}
	}

	f32 Scale = 1.0f;
	if(FactorSum)
	{
		Scale = 1.0f / FactorSum;
	}

	//
	// NOTE(Justin): Scale mixed animation, then copy.
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
ModelJointsUpdate(animation_player *AnimationPlayer, v3 Offset = V3(0.0f))
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
			Xform.Position		= FinalPose->Positions[0];
			Xform.Orientation	= FinalPose->Orientations[0];
			Xform.Scale			= FinalPose->Scales[0];
			Xform.Position		+= Offset;

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


internal animation_graph_node * 
AnimationGraphNodeAdd(animation_graph *Graph, char *Name)
{
	Assert(Graph->NodeCount < ArrayCount(Graph->Nodes));
	animation_graph_node *Node = Graph->Nodes + Graph->NodeCount;
	Node->Name = StringCopy(&Graph->Arena, Name);
	Node->Index = Graph->NodeCount;
	// TODO(justin): This is supposed to be a tag that maps to a string which is the name of the animation..
	//Node->Tag = StringCopy(Graph->Arena, Animation); 
	Node->WhenDone = {};

	return(Node);
}

// TODO(Justin): Inseat of having a bunch of defaults. Should do begin graph node, then add parameters, then
// end graph node.
internal void
AnimationGraphNodeAddArc(memory_arena *Arena, animation_graph_node *Node, char *InboundMessage, char *Dest,
		arc_type Type,
		f32 t0 = 0.0f,
		f32 t1 = 0.0f,
		f32 RemainingTimeBeforeCrossFade = 0.0f,
		b32 BlendDurationSet = false,
		f32 BlendDuration = 0.0f)
{
	Assert(Node->ArcCount < ArrayCount(Node->Arcs));
	animation_graph_arc *Arc = Node->Arcs + Node->ArcCount++;
	Arc->Destination = StringCopy(Arena, Dest);
	Arc->Message = StringCopy(Arena, InboundMessage);
	Arc->RemainingTimeBeforeCrossFade = RemainingTimeBeforeCrossFade;
	Arc->Type = Type;
	Arc->t0 = t0;
	Arc->t1 = t1;
	Arc->BlendDurationSet = BlendDurationSet;
	Arc->BlendDuration = BlendDuration;
}

internal void
AnimationGraphNodeAddWhenDoneArc(memory_arena *Arena, animation_graph_node *Node, char *Message, char *Dest,
		f32 RemainingTimeBeforeCrossFade = 0.2f,
		arc_type Type = ArcType_None,
		f32 t0 = 0.0f,
		f32 t1 = 0.0f,
		b32 BlendDurationSet = false,
		f32 BlendDuration = 0.2f)
{
	Node->WhenDone.Destination = StringCopy(Arena, Dest);
	Node->WhenDone.Message = StringCopy(Arena, Message);
	Node->WhenDone.RemainingTimeBeforeCrossFade = RemainingTimeBeforeCrossFade;
	Node->WhenDone.Type = Type;
	Node->WhenDone.t0 = t0;
	Node->WhenDone.t1 = t1;
	Node->WhenDone.BlendDurationSet = BlendDurationSet;
	Node->WhenDone.BlendDuration = BlendDuration;
}

internal void
AnimationGraphPerFrameUpdate(asset_manager *AssetManager, animation_player *AnimationPlayer,
													animation_graph *Graph)
{
	// NOTE(Justin): This does not work 100% in the current system. The oldest is not necessarily the same
	// as the current node. The oldest may be another animation that is currently being blended out
	// whild the current node is blending in. So any work that is done is invalid.
	//Find the channel playing the currently active animation.
	//animation *Oldest = AnimationOldestGet(AnimationPlayer);

	// Should this just be the current node?
	animation *Oldest = AnimationPlayer->Channels;
	if(!Oldest)
	{
		return;
	}

	f32 RemainingTime = Oldest->Info->Duration - Oldest->CurrentTime;
	animation_graph_node *Node = &Graph->CurrentNode;
	animation_graph_arc Arc = Node->WhenDone;
	if((Arc.Destination.Size != 0) && (RemainingTime <= Arc.RemainingTimeBeforeCrossFade))
	{
		f32 FadeTime = Clamp01(RemainingTime);
		SwitchToNode(AssetManager, AnimationPlayer, Graph, Arc.Destination, FadeTime);
	}
}

internal void
NodeBegin(animation_graph *Graph, char *NodeName)
{
	AnimationGraphNodeAdd(Graph, NodeName);
	Graph->CurrentNode = Graph->Nodes[Graph->NodeCount];
	Graph->Index = Graph->NodeCount;
}

internal void
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

internal void 
AnimationGraphInitialize(animation_graph *G, char *FileName)
{
	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size != 0)
	{
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
				if(StringsAreSame(Word, "message"))
				{
					char *InBoundMessage = strtok(0, " ");
					char *DestNodeName = strtok(0, " ");
					char *Param = strtok(0, " ");
					arc_type Type = ArcType_None;
					f32 t0 = 0.0f;
					f32 t1 = 0.0f;
					if(Param)
					{
						t0 = F32FromASCII(Param);
						Param = strtok(0, " ");
						t1 = F32FromASCII(Param);
						Type = ArcType_TimeInterval;
					}

					AnimationGraphNodeAddArc(&G->Arena, &G->Nodes[G->Index], InBoundMessage, DestNodeName, Type, t0, t1);
				}
				else if(StringsAreSame(Word, "animation"))
				{
					// TODO(Justin): This should be a tag. E.g. idle not the actual name of the animaation?
					char *AnimationName = strtok(0, " ");
					G->Nodes[G->Index].Tag = StringCopy(&G->Arena, AnimationName);
				}
				else if(StringsAreSame(Word, "when_done"))
				{
					char *InBoundMessage = strtok(0, " ");
					char *DestNodeName = strtok(0, " ");
					char *Param = strtok(0, " ");
					arc_type Type = ArcType_None;
					f32 RemainingTimeBeforeCrossFade = 0.0f;
					if(Param)
					{
						RemainingTimeBeforeCrossFade = F32FromASCII(Param);
					}

					AnimationGraphNodeAddWhenDoneArc(&G->Arena, &G->Nodes[G->Index], InBoundMessage, DestNodeName, RemainingTimeBeforeCrossFade);
				}
				else if(StringsAreSame(Word, "controls_position"))
				{
					G->Nodes[G->Index].ControlsPosition = true;
				}
				else if(*Word == '#')
				{
					// Comment, do nothing.
				}

				AdvanceLine(&Content);
			}
		}
	}
	else
	{
	}

	// TODO(Justin): Fix the ending of the last node. This is a hack?
	NodeEnd(G);
}

internal void
AnimationGraphSave(animation_graph *Graph, char *FileName)
{
	char Buff[4096];
	MemoryZero(Buff, sizeof(Buff));

	for(u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
	{
		animation_graph_node *Node = Graph->Nodes + NodeIndex;
		strcat(Buff, ": ");
		strcat(Buff, (char *)Node->Name.Data);
		strcat(Buff, "\n");
		for(u32 ArcIndex = 0; ArcIndex < Node->ArcCount; ++ArcIndex)
		{
			animation_graph_arc *Arc = Node->Arcs + ArcIndex;
			switch(Arc->Type)
			{
				case ArcType_None:
				{
					strcat(Buff, "message ");
					strcat(Buff, (char *)Arc->Message.Data);
					strcat(Buff, " ");
					strcat(Buff, (char *)Arc->Destination.Data);
				} break;
				case ArcType_TimeInterval:
				{
					char FloatBuff[32];

					strcat(Buff, "message ");
					strcat(Buff, (char *)Arc->Message.Data);
					strcat(Buff, " ");
					strcat(Buff, (char *)Arc->Destination.Data);
					strcat(Buff, " ");

					F32ToString(FloatBuff, "%.1f", Arc->t0);
					strcat(Buff, FloatBuff); 
					strcat(Buff, " ");

					F32ToString(FloatBuff, "%.1f", Arc->t1);
					strcat(Buff, FloatBuff); 
				} break;
			}

			strcat(Buff, "\n");
		}

		if(Node->WhenDone.Message.Size != 0)
		{
			char FloatBuff[32];

			strcat(Buff, "when_done ");
			strcat(Buff, (char *)Node->WhenDone.Message.Data);
			strcat(Buff, " ");
			strcat(Buff, (char *)Node->WhenDone.Destination.Data);
			strcat(Buff, " ");

			F32ToString(FloatBuff, "%.1f", Node->WhenDone.RemainingTimeBeforeCrossFade);
			strcat(Buff, FloatBuff); 
		}

		strcat(Buff, "\n");
	}

	Platform.DebugFileWriteEntire(FileName, Buff, (u32)String(Buff).Size);
}

