internal model
ObjLoad(memory_arena *Arena, char *FileName)
{
	model Result = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;

		u8 Buffer_[4096];
		u8 *Buffer = &Buffer_[0];
		MemoryZero(Buffer, sizeof(Buffer));
		u32 At = 0;

		u32 PositionCount = 0;
		u32 NormalCount = 0;
		u32 UVCount = 0;
		u32 FaceCount = 0;
		while(*Content)
		{
			BufferLine(&Content, Buffer);
			AdvanceLine(&Content);
			switch(Buffer[0])
			{
				case 'v':
				{
					if(Buffer[1] == ' ')
					{
						PositionCount++;
					}

					if(Buffer[1] == 'n')
					{
						NormalCount++;
					}

					if(Buffer[1] == 't')
					{
						UVCount++;
					}

				} break;
				case 'f':
				{
					char *Tok = strtok((char *)Buffer, " ");
					Tok = strtok(0, " ");

					while(Tok)
					{
						FaceCount++;
						Tok = strtok(0, " ");
					}
				} break;
			}
		}

		// TODO(Justin): Use temporary memory as buffer for vertex data 
		//temporary_memory VertexMemory = TemporaryMemoryBegin(Arena);

		v3 *Positions	= PushArray(Arena, PositionCount, v3);
		v3 *Normals		= PushArray(Arena, NormalCount, v3);
		v2 *UVs			= PushArray(Arena, UVCount, v2);
		u32 *IndicesP = PushArray(Arena, FaceCount, u32);
		u32 *IndicesN = PushArray(Arena, FaceCount, u32);
		u32 *IndicesUV = PushArray(Arena, FaceCount, u32);

		u32 PositionIndex = 0;
		u32 NormalIndex = 0;
		u32 UVIndex = 0;
		u32 IndexP = 0;
		u32 IndexUV = 0;
		u32 IndexN = 0;
		Content = (u8 *)File.Content;
		while(*Content)
		{
			BufferLine(&Content, Buffer);
			AdvanceLine(&Content);
			switch(Buffer[0])
			{
				case 'v':
				{
					if(Buffer[1] == ' ')
					{
						v3 *P = Positions + PositionIndex;

						char *Word = strtok((char *)Buffer, " ");
						Word = strtok(0, " ");
						P->x = F32FromASCII(Word);
						Word = strtok(0, " ");
						P->y = F32FromASCII(Word);
						Word = strtok(0, " ");
						P->z = F32FromASCII(Word);

						PositionIndex++;
					}

					if(Buffer[1] == 'n')
					{
						v3 *N = Normals + NormalIndex;

						char *Word = strtok((char *)Buffer, " ");
						Word = strtok(0, " ");
						N->x = F32FromASCII(Word);
						Word = strtok(0, " ");
						N->y = F32FromASCII(Word);
						Word = strtok(0, " ");
						N->z = F32FromASCII(Word);

						NormalIndex++;
					}

					if(Buffer[1] == 't')
					{
						v2 *UV = UVs + UVIndex;

						char *Word = strtok((char *)Buffer, " ");
						Word = strtok(0, " ");
						UV->x = F32FromASCII(Word);
						Word = strtok(0, " ");
						UV->y = F32FromASCII(Word);

						UVIndex++;
					}
				} break;
				case 'f':
				{
					At = 0;
					char IndexBuffer[64];

					char *Word = strtok((char *)Buffer, " ");
					while(Word)
					{
						Word = strtok(0, " ");
						if(Word)
						{
							char *P = Word;
							At = 0;
							while(*P)
							{
								IndexBuffer[At++] = *P++;
							}

							IndexBuffer[At] = '\0';
							IndicesP[IndexP++] = U32FromASCII(IndexBuffer) - 1;

							At = 0;
							while(IndexBuffer[At] != '/')
							{
								At++;
							}
							while(IndexBuffer[At] == '/')
							{
								At++;
							}

							if(UVCount != 0)
							{
								IndicesUV[IndexUV++] = U32FromASCII((IndexBuffer + At)) - 1;
								while(IndexBuffer[At] != '/')
								{
									At++;
								}
								while(IndexBuffer[At] == '/')
								{
									At++;
								}
								IndicesN[IndexN++] = U32FromASCII((IndexBuffer + At)) - 1;
							}
							else
							{
								IndicesN[IndexN++] = U32FromASCII((IndexBuffer + At)) - 1;
							}
						}
					}
				} break;
			}
		}

		Assert(PositionCount == PositionIndex);
		Assert(NormalCount == NormalIndex);
		Assert(UVCount == UVIndex);

		u32 MeshVertexCount = FaceCount;

		Result.MeshCount = 1;
		Result.Meshes = PushArray(Arena, Result.MeshCount, mesh);
		mesh *Mesh = Result.Meshes;
		Mesh->VertexCount = MeshVertexCount;
		Mesh->IndicesCount = MeshVertexCount;
		Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, vertex);
		Mesh->Indices = PushArray(Arena, Mesh->VertexCount, u32);

		for(u32 Index = 0; Index < Mesh->VertexCount; ++Index)
		{
			u32 IP = IndicesP[Index];
			u32 IN = IndicesN[Index];
			u32 IUV = IndicesUV[Index];

			v3 P = Positions[IP];
			v3 N = Normals[IN];
			v2 UV = UVs[IUV];

			vertex *V = Mesh->Vertices + Index;

			V->P = P;
			V->N = N;
			V->UV = UV;

			Mesh->Indices[Index] = Index;

		}

		//TemporaryMemoryEnd(VertexMemory);
	}

	return(Result);
}

internal model 
ModelLoad(memory_arena *Arena, char *FileName)
{
	model Model = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;

		asset_model_header *Header = (asset_model_header *)Content;
		Assert(Header->MagicNumber == MODEL_FILE_MAGIC_NUMBER);
		Assert(Header->Version == MODEL_FILE_VERSION);
		Assert(Header->MeshCount != 0);

		Model.MeshCount = Header->MeshCount;
		Model.HasSkeleton = Header->HasSkeleton;
		Model.Meshes = PushArray(Arena, Model.MeshCount, mesh);

		asset_mesh_info *MeshSource = (asset_mesh_info *)(Content + Header->OffsetToMeshInfo);
		for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
		{
			mesh *Mesh = Model.Meshes + MeshIndex;
			Mesh->Name = String(MeshSource->Name);
			Mesh->IndicesCount = MeshSource->IndicesCount;
			Mesh->VertexCount = MeshSource->VertexCount;
			Mesh->JointCount = MeshSource->JointCount;
			Mesh->MaterialSpec = MeshSource->MaterialSpec;
			Mesh->BindTransform = MeshSource->XFormBind;

			Mesh->Indices = (u32 *)(Content + MeshSource->OffsetToIndices);
			Mesh->Vertices = (vertex *)(Content + MeshSource->OffsetToVertices);
			Mesh->InvBindTransforms = (mat4 *)(Content + MeshSource->OffsetToInvBindXForms);

			// TODO(Justin): Figure out best way to handle strings and joints
			// Right now we are copying the joint information which we dont want to do..
			//Mesh->Joints = (joint *)(Content + Source->OffsetToJoints);

			Mesh->Joints = PushArray(Arena, Mesh->JointCount, joint);
			Mesh->JointTransforms = PushArray(Arena, Mesh->JointCount, mat4);
			Mesh->ModelSpaceTransforms = PushArray(Arena, Mesh->JointCount, mat4);
			mat4 I = Mat4Identity();

			asset_joint_info *JointSource = (asset_joint_info *)(Content + MeshSource->OffsetToJoints);
			for(u32 JointIndex = 0; JointIndex < Mesh->JointCount; ++JointIndex)
			{
				joint *Dest = Mesh->Joints + JointIndex;
				Dest->Name = String(JointSource->Name);
				Dest->ParentIndex = JointSource->ParentIndex;
				Dest->Transform = JointSource->Transform;

				Mesh->JointTransforms[JointIndex] = I;
				Mesh->ModelSpaceTransforms[JointIndex] = I;

				JointSource++;
			}

			MeshSource++;
		}
	}
	else
	{
	}


#if 0
	f32 MaxHeight = 0.0f;
	for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model.Meshes + MeshIndex;
		for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
		{
			vertex *Vertex = Mesh->Vertices + VertexIndex;
			if(Vertex->P.y > MaxHeight)
			{
				MaxHeight = Vertex->P.y;
			}
		}
	}
	Model.Height = MaxHeight;
#else
	Model.Height = ModelHeight(&Model);
#endif

	return(Model);
}

internal animation_info 
AnimationLoad(memory_arena *Arena, char *FileName)
{
	animation_info Info = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;

		asset_animation_header *Header = (asset_animation_header *)Content;
		Assert(Header->MagicNumber == ANIMATION_FILE_MAGIC_NUMBER);
		Assert(Header->Version == ANIMATION_FILE_VERSION);
		Assert(Header->JointCount != 0);

		Info.JointCount = Header->JointCount;
		Info.KeyFrameCount = Header->KeyFrameCount;
		//Info.TimeCount = Header->TimeCount;
		//Info.KeyFrameIndex = Header->KeyFrameIndex;
		Info.CurrentTime = Header->CurrentTime;
		Info.Duration = Header->Duration;
		Info.FrameRate = Header->FrameRate;

		//Info.Times = (f32 *)(Content + Header->OffsetToTimes);
		
		// TODO(Justin): Should not have to copy the data!!
		//Info.JointNames = (string *)(Content + Header->OffsetToNames);
		Info.JointNames = PushArray(Arena, Info.JointCount, string);
		asset_joint_name *JointNameSource = (asset_joint_name *)(Content + Header->OffsetToNames);
		for(u32 JointIndex = 0; JointIndex < Info.JointCount; ++JointIndex)
		{
			Info.JointNames[JointIndex] = String(JointNameSource->Name);
			JointNameSource++;
		}

		Info.KeyFrames = PushArray(Arena, Info.KeyFrameCount, key_frame);
		asset_animation_info *AnimationSource = (asset_animation_info *)(Content + Header->OffsetToAnimationInfo);
		for(u32 KeyFrameIndex = 0; KeyFrameIndex < Info.KeyFrameCount; ++KeyFrameIndex)
		{
			key_frame *KeyFrame = Info.KeyFrames + KeyFrameIndex;
			KeyFrame->Positions = (v3 *)(Content + AnimationSource->OffsetToPositions);
			KeyFrame->Orientations = (quaternion *)(Content + AnimationSource->OffsetToQuaternions);
			KeyFrame->Scales = (v3 *)(Content + AnimationSource->OffsetToScales);
			AnimationSource++;
		}
	}
	else
	{
	}

	return(Info);
}

char *LowerJointTags[] =
{
	"Hips",
	"Leg",
	"Foot",
	"Toe"
};

enum animation_name
{
	Animation_IdleRight,
	Animation_IdleLeft,
	Animation_Run,
	Animation_Sprint,
	Animation_JumpForward,
	Animation_RunMirror,
	Animation_SprintMirror,
	Animation_StandingToIdleRight,
	Animation_StandingToIdleLeft,
	Animation_IdleToSprint,
	Animation_IdleToSprintMirror,
	Animation_RunToStop,
	Animation_Running180,
	Animation_CrouchedToIdleLeft,
	Animation_CrouchedToIdleRight,
	Animation_IdleLeftToCrouched,
	Animation_IdleRightToCrouched,
	Animation_CrouchingIdleLeft,
	Animation_CrouchingIdleRight,
};

char *AnimationFiles[] =
{
	"../data/animations/XBot_IdleRight.animation",
	"../data/animations/XBot_IdleLeft.animation",
	"../data/animations/XBot_Running.animation",
	"../data/animations/XBot_FastRun.animation",
	"../data/animations/XBot_JumpForward.animation",
	"../data/animations/XBot_RunningMirror.animation",
	"../data/animations/XBot_FastRunMirror.animation",
	"../data/animations/XBot_StandingToIdleRight.animation",
	"../data/animations/XBot_StandingToIdleLeft.animation",
	"../data/animations/XBot_IdleToSprint.animation",
	"../data/animations/XBot_IdleToSprintMirror.animation",
	"../data/animations/XBot_RunToStop.animation",
	"../data/animations/XBot_Running180.animation",
	"../data/animations/XBot_CrouchedToIdleLeft.animation",
	"../data/animations/XBot_CrouchedToIdleRight.animation",
	"../data/animations/XBot_IdleLeftToCrouched.animation",
	"../data/animations/XBot_IdleRightToCrouched.animation",
	"../data/animations/XBot_CrouchingIdleLeft.animation",
	"../data/animations/XBot_CrouchingIdleRight.animation",
};

char *GraphFiles[] =
{
	"../src/XBot_AnimationGraph.animation_graph",
};

char *TextureFiles[] =
{
	"../data/textures/tile_gray.bmp",
	"../data/textures/left_arrow.png",
	"../data/textures/texture_01.png",
	"../data/textures/texture_13.png",
	"../data/textures/orange_texture_02.png",
};

char *ModelFiles[] =
{
	"../data/models/XBot.mesh",
	"../data/models/Cube.mesh",
	"../data/models/Sphere.mesh",
	"../data/models/Arrow.mesh",
	"../data/models/Capsule.obj",
	"../data/models/Cylinder.obj",
	"../data/models/Cone.obj",
};

char *FontFiles[] =
{
	"c:/windows/fonts/arial.ttf",
};

internal texture *
LookupTexture(asset_manager *AssetManager, char *TextureName)
{
	texture *Result = 0;
	s32 Index = StringHashLookup(&AssetManager->TextureNames, TextureName);
	if(Index != -1)
	{
		Result = AssetManager->Textures + Index;
	}

	return(Result);
}

internal model *
LookupModel(asset_manager *AssetManager, char *ModelName)
{
	model *Result = 0;
	s32 Index = StringHashLookup(&AssetManager->ModelNames, ModelName);
	if(Index != -1)
	{
		Result = AssetManager->Models + Index;
	}

	return(Result);
}

internal animation *
LookupAnimation(asset_manager *AssetManager, char *AnimationName)
{
	animation *Result = 0;
	s32 Index = StringHashLookup(&AssetManager->AnimationNames, AnimationName);
	if(Index != -1)
	{
		Result = AssetManager->Animations + Index;
	}

	return(Result);
}

internal animation_graph *
LookupGraph(asset_manager *AssetManager, char *AnimationGraphName)
{
	animation_graph *Result = 0;
	s32 Index = StringHashLookup(&AssetManager->GraphNames, AnimationGraphName);
	if(Index != -1)
	{
		Result = AssetManager->Graphs + Index;
	}

	return(Result);
}

internal void
AssetManagerInit(asset_manager *Manager)
{
	char Buffer[256];
	char ExtBuffer[256];

	//
	// Textures
	//

	ArenaSubset(&Manager->Arena, &Manager->TextureNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->TextureNames);
	for(u32 NameIndex = 0; NameIndex < ArrayCount(TextureFiles); ++NameIndex)
	{
		char *FullPath = TextureFiles[NameIndex];
		FileNameFromFullPath(FullPath, Buffer);
		StringHashAdd(&Manager->TextureNames, Buffer, NameIndex);
		s32 Index = StringHashLookup(&Manager->TextureNames, Buffer);
		Assert(Index != -1);
		texture *Texture = Manager->Textures + Index;
		*Texture = TextureLoad(FullPath);
		Texture->Name = StringCopy(&Manager->Arena, (char *)Buffer);
	}

	//
	// Models
	//

	ArenaSubset(&Manager->Arena, &Manager->ModelNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->ModelNames);
	for(u32 NameIndex = 0; NameIndex < ArrayCount(ModelFiles); ++NameIndex)
	{
		char *FullPath = ModelFiles[NameIndex];
		FileNameFromFullPath(FullPath, Buffer);
		ExtFromFullPath(FullPath, ExtBuffer);
		StringHashAdd(&Manager->ModelNames, Buffer, NameIndex);
		s32 Index = StringHashLookup(&Manager->ModelNames, Buffer);
		Assert(Index != -1);
		model *Model = Manager->Models + Index;

		texture *Texture = LookupTexture(Manager, "orange_texture_02");
		Assert(Texture);
		if(StringsAreSame(ExtBuffer, "mesh"))
		{
			*Model = ModelLoad(&Manager->Arena, FullPath);
		}
		else
		{
			*Model = ObjLoad(&Manager->Arena, FullPath);
			Model->Meshes[0].Texture = Texture;
		}

		if(StringsAreSame(Buffer, "Cube"))
		{
			Model->Meshes[0].Texture = Texture;
		}
		else if(StringsAreSame(Buffer, "Sphere"))
		{
			Model->Meshes[0].Texture = Texture;
		}
	}

#if 0
	{
		texture *Texture = LookupTexture(Manager, "orange_texture_02");
		Assert(Texture);

		char *FullPath = "../data/models/Capsule.obj";
		FileNameFromFullPath(FullPath, Buffer);
		StringHashAdd(&Manager->ModelNames, Buffer, ArrayCount(ModelFiles) + 1);
		s32 Index = StringHashLookup(&Manager->ModelNames, Buffer);
		Assert(Index != -1);
		model *Model = Manager->Models + Index;
		*Model = ObjLoad(&Manager->Arena, FullPath);
		Model->Meshes[0].Texture = Texture;

		FullPath = "../data/models/Cylinder.obj";
		FileNameFromFullPath(FullPath, Buffer);
		StringHashAdd(&Manager->ModelNames, Buffer, ArrayCount(ModelFiles) + 1);
		Index = StringHashLookup(&Manager->ModelNames, Buffer);
		Assert(Index != -1);
		model *Cylinder = Manager->Models + Index;
		*Cylinder = ObjLoad(&Manager->Arena, FullPath);
		Cylinder->Meshes[0].Texture = Texture;

		FullPath = "../data/models/Cone.obj";
		FileNameFromFullPath(FullPath, Buffer);
		StringHashAdd(&Manager->ModelNames, Buffer, ArrayCount(ModelFiles) + 1);
		Index = StringHashLookup(&Manager->ModelNames, Buffer);
		Assert(Index != -1);
		model *Cone = Manager->Models + Index;
		*Cone = ObjLoad(&Manager->Arena, FullPath);
		Cone ->Meshes[0].Texture = Texture;

	}
#endif

	//
	// Animations
	//

	ArenaSubset(&Manager->Arena, &Manager->AnimationNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->AnimationNames);
	Manager->AnimationInfos = PushArray(&Manager->Arena, ArrayCount(AnimationFiles), animation_info);
	Manager->Animations		= PushArray(&Manager->Arena, ArrayCount(AnimationFiles), animation);
	for(u32 NameIndex = 0; NameIndex < ArrayCount(AnimationFiles); ++NameIndex)
	{
		char *FullPath = AnimationFiles[NameIndex];
		FileNameFromFullPath(FullPath, Buffer);
		StringHashAdd(&Manager->AnimationNames, Buffer, NameIndex);
		s32 Index = StringHashLookup(&Manager->AnimationNames, Buffer);
		Assert(Index != -1);

		animation_info *Info = Manager->AnimationInfos + Index;
		animation *Animation = Manager->Animations + Index;
		*Info = AnimationLoad(&Manager->Arena, FullPath);
		if(Info)
		{
			Animation->Name = StringCopy(&Manager->Arena, Buffer);
			Animation->ID.Value = Index;
			Animation->Info = Info;
			Animation->BlendedPose = PushStruct(&Manager->Arena, key_frame);
			key_frame *BlendedPose = Animation->BlendedPose;
			AllocateJointXforms(&Manager->Arena, BlendedPose, Info->JointCount);



			// TODO(Justin): Load this from a file.
			switch(NameIndex)
			{
				case Animation_IdleRight:
				case Animation_IdleLeft:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = AnimationFlags_Looping;
				} break;
				case Animation_IdleToSprint:
				case Animation_IdleToSprintMirror:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = (AnimationFlags_RemoveLocomotion);
#if 0
					Animation->JointMasks = PushArray(&Manager->Arena, Info->JointCount, b32);
					for(u32 JointIndex = 0; JointIndex < Info->JointCount; ++JointIndex)
					{
						string JointName = Info->JointNames[JointIndex];
						for(u32 StringIndex = 0; StringIndex < ArrayCount(LowerJointMasks); ++StringIndex)
						{
							char *MaskTag = LowerJointMasks[StringIndex];
							if(SubStringExists(JointName, MaskTag))
							{
								Animation->JointMasks[JointIndex] = true;
							}
						}
					}
#endif

				} break;
				case Animation_StandingToIdleRight:
				case Animation_StandingToIdleLeft:
				{
					Animation->TimeScale = 1.0f;
#if 0
					//Animation->DefaultFlags = (AnimationFlags_JointMask | AnimationFlags_RemoveLocomotion);
					Animation->DefaultFlags = AnimationFlags_JointMask;// | AnimationFlags_RemoveLocomotion);
					Animation->JointMasks = PushArray(&Manager->Arena, Info->JointCount, b32);
					for(u32 JointIndex = 0; JointIndex < Info->JointCount; ++JointIndex)
					{
						string JointName = Info->JointNames[JointIndex];
						for(u32 StringIndex = 0; StringIndex < ArrayCount(LowerJointMasks); ++StringIndex)
						{
							char *MaskTag = LowerJointMasks[StringIndex];
							if(SubStringExists(JointName, MaskTag))
							{
								Animation->JointMasks[JointIndex] = true;
							}
						}
					}
#endif
				} break;
				case Animation_Run:
				case Animation_RunMirror:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = AnimationFlags_Looping;
				} break;
				case Animation_RunToStop:
				{
					Animation->TimeScale = 1.0f;
					Animation->TimeOffset = 0.6f;
					Animation->DefaultFlags = AnimationFlags_RemoveLocomotion;
				} break;
				case Animation_Sprint:
				case Animation_SprintMirror:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = AnimationFlags_Looping;
				} break;
				case Animation_JumpForward:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = AnimationFlags_ControlsPosition;
#if 0
					f32 Scale = 0.025f;
					for(u32 KeyFrameIndex = 0; KeyFrameIndex < Info->KeyFrameCount; ++KeyFrameIndex)
					{
						key_frame *KeyFrame = Info->KeyFrames + KeyFrameIndex;
						for(u32 JointIndex = 0; JointIndex < Info->JointCount; ++JointIndex)
						{
							KeyFrame->Scales[JointIndex] *= Scale;
						}
					}
#endif

				} break;
				case Animation_Running180:
				{
					Animation->TimeScale = 1.0f;
				} break;

				case Animation_CrouchedToIdleLeft:
				case Animation_CrouchedToIdleRight:
				case Animation_IdleLeftToCrouched:
				case Animation_IdleRightToCrouched:
				{
					Animation->TimeScale = 1.0f;
				} break;
				case Animation_CrouchingIdleLeft:
				case Animation_CrouchingIdleRight:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = AnimationFlags_Looping;
				} break;
			}
		}
	}

	// // Graphs
	//

	ArenaSubset(&Manager->Arena, &Manager->GraphNames.Arena, Kilobyte(4));
	StringHashInit(&Manager->GraphNames);
	for(u32 NameIndex = 0; NameIndex < ArrayCount(GraphFiles); ++NameIndex)
	{
		char *FullPath = GraphFiles[NameIndex];
		FileNameFromFullPath(FullPath, Buffer);
		StringHashAdd(&Manager->GraphNames, Buffer, NameIndex);
		s32 Index = StringHashLookup(&Manager->GraphNames, Buffer);
		Assert(Index != -1);
		animation_graph *G = Manager->Graphs + Index;
		ArenaSubset(&Manager->Arena, &G->Arena, Kilobyte(4));
		AnimationGraphInit(G, FullPath);
	}

	//
	// Font
	//

	FontInit(&Manager->Arena, &Manager->Font, FontFiles[0]);
}
