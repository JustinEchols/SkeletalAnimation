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

#if 0
char *LowerJointTags[] =
{
	"Hips",
	"Leg",
	"Foot",
	"Toe"
};

internal void 
LowerJointMasksInitialize(animation *Animation, animation_info *Info)
{
	for(u32 JointIndex = 0; JointIndex < Info->JointCount; ++JointIndex)
	{
		string JointName = Info->JointNames[JointIndex];
		for(u32 StringIndex = 0; StringIndex < ArrayCount(LowerJointTags); ++StringIndex)
		{
			char *MaskTag = LowerJointTags[StringIndex];
			if(SubStringExists(JointName, MaskTag))
			{
				Animation->JointMasks[JointIndex] = true;
			}
		}
	}
}
#endif

char *AnimationDirectory = "animations";
char *AnimationDirectoryAndWildCard = "animations\\XBot*";

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
	"../data/textures/red_texture_02.png",
};

char *ModelFiles[] =
{
	"../data/models/XBot.mesh",
	"../data/models/Cube.mesh",
	"../data/models/Sphere.mesh",
	"../data/models/Arrow.mesh",
	"../data/models/XArrow.mesh",
	"../data/models/YArrow.mesh",
	"../data/models/Capsule.obj",
	"../data/models/Cylinder.obj",
	"../data/models/Cone.obj",
};

char *FontFiles[] =
{
	"c:/windows/fonts/arial.ttf",
};

internal asset_entry 
LookupTexture(asset_manager *AssetManager, char *TextureName)
{
	asset_entry Result = {};

	texture *Texture = 0;
	s32 Index = StringHashLookup(&AssetManager->TextureNames, TextureName);
	if(Index != -1)
	{
		Texture = AssetManager->Textures + Index;
	}

	Result.Index = Index;
	Result.Texture = Texture;

	return(Result);
}

internal asset_entry
LookupModel(asset_manager *AssetManager, char *ModelName)
{
	asset_entry Result = {};

	model *Model = 0;
	s32 Index = StringHashLookup(&AssetManager->ModelNames, ModelName);
	if(Index != -1)
	{
		Model = AssetManager->Models + Index;
	}

	Result.Index = Index;
	Result.Model = Model;

	return(Result);
}

internal asset_entry
LookupAnimation(asset_manager *AssetManager, char *AnimationName)
{
	asset_entry Result = {};

	animation *Animation = 0;
	s32 Index = StringHashLookup(&AssetManager->AnimationNames, AnimationName);
	if(Index != -1)
	{
		Animation = AssetManager->Animations + Index;
	}

	Result.Index = Index;
	Result.Animation = Animation;

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
AssetManagerInitialize(asset_manager *Manager)
{
	char Buffer[256];
	char ExtBuffer[256];
	char AssetName[256];

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

		asset_entry Entry = LookupTexture(Manager, "orange_texture_02");
		Assert(Entry.Texture);
		if(StringsAreSame(ExtBuffer, "mesh"))
		{
			*Model = ModelLoad(&Manager->Arena, FullPath);
		}
		else
		{
			*Model = ObjLoad(&Manager->Arena, FullPath);
			Model->Meshes[0].Texture = Entry.Texture;
		}

		if(StringsAreSame(Buffer, "Cube"))
		{
			Model->Meshes[0].Texture = Entry.Texture;
		}
		else if(StringsAreSame(Buffer, "Sphere"))
		{
			Model->Meshes[0].Texture = Entry.Texture;
		}
		else if(StringsAreSame(Buffer, "Arrow"))
		{
			Model->Meshes[0].Texture = Entry.Texture;
		}
	}

	//
	// Animations
	//

	file_group_info AnimationFileGroup = Platform.DebugFileGroupLoad(AnimationDirectoryAndWildCard);

	ArenaSubset(&Manager->Arena, &Manager->AnimationNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->AnimationNames);
	Manager->AnimationInfos = PushArray(&Manager->Arena, AnimationFileGroup.Count, animation_info);
	Manager->Animations		= PushArray(&Manager->Arena, AnimationFileGroup.Count, animation);
	for(u32 FileIndex = 0; FileIndex < AnimationFileGroup.Count; ++FileIndex)
	{
		char *FileName = AnimationFileGroup.FileNames[FileIndex];
		FileNameFromFullPath(FileName, AssetName);
		StringHashAdd(&Manager->AnimationNames, AssetName, FileIndex);
		s32 Index = StringHashLookup(&Manager->AnimationNames, AssetName);
		Assert(Index != -1);

		MemoryZero(Buffer, sizeof(Buffer));
		strcat(Buffer, AnimationDirectory);
		strcat(Buffer, "/");
		strcat(Buffer, FileName);

		animation_info *Info = Manager->AnimationInfos + Index;
		animation *Animation = Manager->Animations + Index;
		*Info = AnimationLoad(&Manager->Arena, Buffer);
		if(Info)
		{
			Animation->Name = StringCopy(&Manager->Arena, AssetName);
			Animation->ID.Value = Index;
			Animation->Info = Info;
			Animation->BlendedPose = PushStruct(&Manager->Arena, key_frame);
			key_frame *BlendedPose = Animation->BlendedPose;
			AllocateJointXforms(&Manager->Arena, BlendedPose, Info->JointCount);

			// TODO(Justin): Either put this in the file format or let the 
			// animation system determine this.
			if(StringsAreSame(AssetName, "XBot_IdleRight") ||
			   StringsAreSame(AssetName, "XBot_IdleLeft") ||
			   StringsAreSame(AssetName, "XBot_Running") ||
			   StringsAreSame(AssetName, "XBot_RunningMirror") ||
	   		   StringsAreSame(AssetName, "XBot_FastRun") ||
   			   StringsAreSame(AssetName, "XBot_FastRunMirror") ||
   			   StringsAreSame(AssetName, "XBot_CrouchingIdleLeft") ||
   			   StringsAreSame(AssetName, "XBot_CrouchingIdleRight"))
			{
				Animation->DefaultFlags = AnimationFlags_Looping;
			}
			if(StringsAreSame(AssetName, "XBot_IdleToSprint") ||
			   StringsAreSame(AssetName, "XBot_IdleToSprintMirror"))
			{
				Animation->DefaultFlags = (AnimationFlags_RemoveLocomotion);
			}
			if(StringsAreSame(AssetName, "XBot_JumpForward") ||
			   StringsAreSame(AssetName, "XBot_JumpForwardMirror"))
			{
				Animation->DefaultFlags = (AnimationFlags_ControlsPosition);
			}
		}
	}

	//
	// Graphs
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
		AnimationGraphInitialize(G, FullPath);
	}

	//
	// Font
	//

	FontInitialize(&Manager->Arena, &Manager->Font, FontFiles[0]);
}
