#if 1
internal model
ObjLoad(memory_arena *Arena, char *FileName)
{
	model Result = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		//TODO(Justin): Exit gracefully
		Assert(0);
	}

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
						IndicesN[IndexN++] = U32FromASCII((IndexBuffer + At)) - 1;
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

	Platform.DebugFileFree(File.Content);

	return(Result);
}
#else
internal model
ObjLoad(memory_arena *Arena, char *FileName)
{
	model Result = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		//TODO(Justin): Exit gracefully
		Assert(0);
	}

	u8 *Content = (u8 *)File.Content;

	u8 LineBuffer_[4096];
	MemoryZero(LineBuffer, sizeof(LineBuffer));
	u8 *LineBuffer = &LineBuffer_[0];

	u8 Word_[4096];
	MemoryZero(Word, sizeof(Word));
	u8 *Word = &Word_[0];

	u32 At = 0;

	u32 PositionCount = 0;
	u32 NormalCount = 0;
	u32 UVCount = 0;
	u32 FaceCount = 0;
	while(*Content)
	{
		BufferLine(&Content, LineBuffer);

		switch(LineBuffer[0])
		{
			case 'v':
			{
				if(LineBuffer[1] == ' ')
				{
					PositionCount++;
				}

				if(LineBuffer[1] == 'n')
				{
					NormalCount++;
				}

				if(LineBuffer[1] == 't')
				{
					UVCount++;
				}

			} break;
			case 'f':
			{
				u8 *Line = LineBuffer;
				BufferNextWord(&Line, Word);
				EatSpaces(&Line);

				while(*Line)
				{
					FaceCount++;
					BufferNextWord(&Line, Word);
					EatSpaces(&Line);
				}
			} break;
		}

		AdvanceLine(&Content);
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
						IndicesN[IndexN++] = U32FromASCII((IndexBuffer + At)) - 1;
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

	Platform.DebugFileFree(File.Content);

	return(Result);
}
#endif

internal model 
ModelLoad(memory_arena *Arena, char *FileName)
{
	model Model = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		Assert(0);
	}

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

			// TODO(Justin): Remove hardcoded values!
			if(MeshIndex == 0)
			{
				if(StringsAreSame(CString(Dest->Name), "mixamorig_RightFoot"))
				{
					Model.RightFootJointIndex = JointIndex;
				}

				if(StringsAreSame(CString(Dest->Name), "mixamorig_LeftFoot"))
				{
					Model.LeftFootJointIndex = JointIndex;
				}

				if(StringsAreSame(CString(Dest->Name), "mixamorig_RightHand"))
				{
					Model.RightHandJointIndex = JointIndex;
				}

				if(StringsAreSame(CString(Dest->Name), "mixamorig_LeftHand"))
				{
					Model.LeftHandJointIndex = JointIndex;
				}
			}
		}

		MeshSource++;
	}

	f32 Xmin, Ymin, Zmin;
	f32 Xmax, Ymax, Zmax;
	Xmin = Ymin = Zmin = F32Max;
	Xmax = Ymax = Zmax = -1.0f*F32Max;
	for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model.Meshes + MeshIndex;
		for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
		{
			vertex *V = Mesh->Vertices + VertexIndex;

			if(V->P.x > Xmax)
			{
				Xmax = V->P.x;
			}
			if(V->P.x < Xmin)
			{
				Xmin = V->P.x;
			}

			if(V->P.y > Ymax)
			{
				Ymax = V->P.y;
			}
			if(V->P.y < Ymin)
			{
				Ymin = V->P.y;
			}

			if(V->P.z > Zmax)
			{
				Zmax = V->P.z;
			}
			if(V->P.z < Zmin)
			{
				Zmin = V->P.z;
			}
		}
	}

	v3 Min = V3(Xmin, Ymin, Zmin);
	v3 Max = V3(Xmax, Ymax, Zmax);
	Model.BoundingBox = AABBMinMax(Min, Max);

	return(Model);
}

internal animation_info 
AnimationLoad(memory_arena *Arena, char *FileName)
{
	animation_info Info = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		Assert(0);
	}

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

	return(Info);
}


char *TexturesDirectory				= "..\\data\\textures";
char *TexturesDirectoryAndWildCard	= "..\\data\\textures\\*";

char *ModelsDirectory				= "..\\data\\models";
char *ModelsDirectoryAndWildCard	= "..\\data\\models\\*";

char *AnimationDirectory			= "..\\data\\animations\\";
char *AnimationDirectoryAndWildCard = "..\\data\\animations\\*";

char *GraphFiles[] =
{
	"../src/XBot_AnimationGraph.animation_graph",
	"../src/YBot_AnimationGraph.animation_graph",
	"../src/Vampire_AnimationGraph.animation_graph",
	"../src/Paladin_AnimationGraph.animation_graph",
	"../src/Archer_AnimationGraph.animation_graph",
	"../src/Brute_AnimationGraph.animation_graph",
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
		Assert(Index >= 0 && Index < ArrayCount(AssetManager->Textures));
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
		Assert(Index >= 0 && Index < ArrayCount(AssetManager->Models));
		Model = AssetManager->Models + Index;
	}

	Result.Index = Index;
	Result.Model = Model;

	return(Result);
}

internal asset_entry
LookupSampledAnimation(asset_manager *AssetManager, char *AnimationName)
{
	asset_entry Result = {};

	animation_info *SampledAnimation = 0;
	s32 Index = StringHashLookup(&AssetManager->AnimationNames, AnimationName);
	if(Index != -1)
	{
		SampledAnimation = AssetManager->SampledAnimations + Index;
	}

	Result.Index = Index;
	Result.SampledAnimation = SampledAnimation;

	return(Result);
}

internal asset_entry 
LookupGraph(asset_manager *AssetManager, char *AnimationGraphName)
{
	asset_entry Result = {};

	animation_graph *Graph = 0;
	s32 Index = StringHashLookup(&AssetManager->GraphNames, AnimationGraphName);
	if(Index != -1)
	{
		Assert(Index >= 0 && Index < ArrayCount(AssetManager->Graphs));
		Graph = AssetManager->Graphs + Index;
	}

	Result.Index = Index;
	Result.Graph = Graph;

	return(Result);
}

internal b32
ShouldReload(platform_file_info *FileInfo)
{
	b32 Result = Platform.DebugFileIsDirty(FileInfo->Path, &FileInfo->FileDate);
	return(Result);
}

#if 1
internal void
AssetManagerInitialize(asset_manager *Manager)
{
	char Buffer[256];
	char ExtBuffer[256];
	char AssetName[256];

	// TODO(Justin): Clean this up.

	//
	// Textures
	//

	ArenaSubset(&Manager->Arena, &Manager->TextureNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->TextureNames);
	StringHashAdd(&Manager->TextureNames, "(null)", 0);

	file_group_info TextureGroup = Platform.DebugFileGroupLoad(TexturesDirectoryAndWildCard);
	for(u32 FileIndex = 0; FileIndex < TextureGroup.Count; ++FileIndex)
	{
		// TODO(Justin): Remove incorrect files
		char *FileName = TextureGroup.FileNames[FileIndex];
		if(FileName[0] == '.') continue;

		FileNameFromFullPath(FileName, AssetName);
		StringHashAdd(&Manager->TextureNames, AssetName, FileIndex + 1);
		s32 Index = StringHashLookup(&Manager->TextureNames, AssetName);
		Assert(Index != -1);
		texture *Texture = Manager->Textures + Index;

		MemoryZero(Buffer, sizeof(Buffer));
		strcat(Buffer, TexturesDirectory);
		strcat(Buffer, "\\");
		strcat(Buffer, FileName);

		*Texture = TextureLoad(Buffer);
		Texture->Name = StringCopy(&Manager->Arena, (char *)AssetName);
	}

	//
	// Models
	//

	ArenaSubset(&Manager->Arena, &Manager->ModelNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->ModelNames);

	file_group_info ModelGroup = Platform.DebugFileGroupLoad(ModelsDirectoryAndWildCard);
	for(u32 FileIndex = 0; FileIndex < ModelGroup.Count; ++FileIndex)
	{
		char *FileName = ModelGroup.FileNames[FileIndex];
		if(FileName[0] == '.') continue;
		
		FileNameFromFullPath(FileName, AssetName);
		ExtFromFullPath(FileName, ExtBuffer);
		StringHashAdd(&Manager->ModelNames, AssetName, FileIndex);
		s32 Index = StringHashLookup(&Manager->ModelNames, AssetName);
		Assert(Index != -1);
		model *Model = Manager->Models + Index;

		MemoryZero(Buffer, sizeof(Buffer));
		strcat(Buffer, ModelsDirectory);
		strcat(Buffer, "/");
		strcat(Buffer, FileName);

		if(StringsAreSame(ExtBuffer, "mesh"))
		{
			*Model = ModelLoad(&Manager->Arena, Buffer);

			if(StringsAreSame(AssetName, "Cube"))
			{
				asset_entry Entry = LookupTexture(Manager, "orange_texture_02");
				Assert(Entry.Texture);
				Model->Meshes[0].DiffuseTexture = Entry.Index;
				Model->Meshes[0].MaterialFlags |= MaterialFlag_Diffuse;
			}

			if(StringsAreSame(AssetName, "PaladinWithProp"))
			{
				asset_entry Diffuse = LookupTexture(Manager, "Paladin_diffuse");
				asset_entry Specular = LookupTexture(Manager, "Paladin_specular");
				Assert(Diffuse.Texture);
				Assert(Specular.Texture);

				for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
				{
					mesh *Mesh = Model->Meshes + MeshIndex;
					Mesh->DiffuseTexture = Diffuse.Index;
					Mesh->SpecularTexture = Specular.Index;
					Model->Meshes[0].MaterialFlags |= (MaterialFlag_Diffuse | MaterialFlag_Specular);
				}
			}

			if(StringsAreSame(AssetName, "Erika_ArcherWithBowArrow"))
			{
				/*
				//0
				FemaleFitA_eyelash_diffuse.png

				//1
				FemaleFitA_Body_diffuse.png
				FemaleFitA_normal.png

				//2
				FemaleFitA_Body_diffuse.png
				Erika_Archer_Clothes_normal.png

				//3
				Bow_diffuse.jpg
				Bow_normal.jpg

				//4
				Arrow_diffuse.png
				Arrow_normal.jpg

				//5
				Erika_Archer_Clothes_diffuse.png
				Erika_Archer_Clothes_normal.png
				*/


				//0
				asset_entry Diffuse = LookupTexture(Manager, "FemaleFitA_eyelash_diffuse");
				Model->Meshes[0].DiffuseTexture = Diffuse.Index;
				Model->Meshes[0].MaterialFlags = (MaterialFlag_Diffuse);

				//1
				Diffuse = LookupTexture(Manager, "FemaleFitA_Body_diffuse");
				Model->Meshes[1].DiffuseTexture = Diffuse.Index;
				Model->Meshes[1].MaterialFlags = (MaterialFlag_Diffuse);

				//2
				Model->Meshes[2].DiffuseTexture = Diffuse.Index;
				Model->Meshes[2].MaterialFlags = (MaterialFlag_Diffuse);

				//3
				Diffuse = LookupTexture(Manager, "Bow_diffuse");
				Model->Meshes[3].DiffuseTexture = Diffuse.Index;
				Model->Meshes[3].MaterialFlags = (MaterialFlag_Diffuse);
				
				//4
				Diffuse = LookupTexture(Manager, "Arrow_diffuse");
				Model->Meshes[4].DiffuseTexture = Diffuse.Index;
				Model->Meshes[4].MaterialFlags = (MaterialFlag_Diffuse);
				Model->Meshes[4].Flags |= MeshFlag_DontDraw;
				
				//5
				Diffuse = LookupTexture(Manager, "Erika_Archer_Clothes_diffuse");
				Model->Meshes[5].DiffuseTexture = Diffuse.Index;
				Model->Meshes[5].MaterialFlags = (MaterialFlag_Diffuse);
			}

			if(StringsAreSame(AssetName, "Brute"))
			{
				asset_entry Diffuse = LookupTexture(Manager, "axe_diffuse");
				asset_entry Specular = LookupTexture(Manager, "axe_specular");
				Model->Meshes[0].DiffuseTexture = Diffuse.Index;
				Model->Meshes[0].SpecularTexture = Specular.Index;
				Model->Meshes[0].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Hair_diffuse");
				Specular = LookupTexture(Manager, "MaleBruteA_Hair_specular");
				Model->Meshes[1].DiffuseTexture = Diffuse.Index;
				Model->Meshes[1].SpecularTexture = Specular.Index;
				Model->Meshes[1].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Bottom_diffuse1");
				Specular = LookupTexture(Manager, "MaleBruteA_Bottom_specular");
				Model->Meshes[2].DiffuseTexture = Diffuse.Index;
				Model->Meshes[2].SpecularTexture = Specular.Index;
				Model->Meshes[2].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Shoes_diffuse1");
				Specular = LookupTexture(Manager, "MaleBruteA_Shoes_specular");
				Model->Meshes[3].DiffuseTexture = Diffuse.Index;
				Model->Meshes[3].SpecularTexture = Specular.Index;
				Model->Meshes[3].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Moustache_diffuse");
				Specular = LookupTexture(Manager, "MaleBruteA_Moustache_specular");
				Model->Meshes[4].DiffuseTexture = Diffuse.Index;
				Model->Meshes[4].SpecularTexture = Specular.Index;
				Model->Meshes[4].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Body_diffuse1");
				Model->Meshes[5].DiffuseTexture = Diffuse.Index;
				Model->Meshes[5].MaterialFlags = (MaterialFlag_Diffuse);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Body_diffuse");
				Model->Meshes[6].DiffuseTexture = Diffuse.Index;
				Model->Meshes[6].MaterialFlags = (MaterialFlag_Diffuse);

				Diffuse = LookupTexture(Manager, "MaleBruteA_Body_diffuse1");
				Model->Meshes[7].DiffuseTexture = Diffuse.Index;
				Model->Meshes[7].MaterialFlags = (MaterialFlag_Diffuse);
			}
		}
		else
		{
			*Model = ObjLoad(&Manager->Arena, Buffer);

			asset_entry Entry = LookupTexture(Manager, "orange_texture_02");
			Assert(Entry.Texture);
			Model->Meshes[0].DiffuseTexture = Entry.Index;
			Model->Meshes[0].MaterialFlags |= MaterialFlag_Diffuse;
		}

		Model->Name = StringCopy(&Manager->Arena, AssetName);

		if(Model->HasSkeleton)
		{
			Platform.UploadAnimatedModelToGPU(Model);
		}
		else
		{
			Platform.UploadModelToGPU(Model);
		}
	}

	//
	// Animations
	//

	file_group_info AnimationFileGroup = Platform.DebugFileGroupLoad(AnimationDirectoryAndWildCard);
	ArenaSubset(&Manager->Arena, &Manager->AnimationNames.Arena, Kilobyte(8));
	StringHashInit(&Manager->AnimationNames);
	Manager->SampledAnimations = PushArray(&Manager->Arena, AnimationFileGroup.Count, animation_info);
	for(u32 FileIndex = 0; FileIndex < AnimationFileGroup.Count; ++FileIndex)
	{
		char *FileName = AnimationFileGroup.FileNames[FileIndex];
		if(FileName[0] == '.') continue;

		ExtFromFullPath(FileName, ExtBuffer);
		if(!StringsAreSame("animation", ExtBuffer)) continue;

		FileNameFromFullPath(FileName, AssetName);
		StringHashAdd(&Manager->AnimationNames, AssetName, FileIndex);
		s32 Index = StringHashLookup(&Manager->AnimationNames, AssetName);
		Assert(Index != -1);

		MemoryZero(Buffer, sizeof(Buffer));
		strcat(Buffer, AnimationDirectory);
		strcat(Buffer, "/");
		strcat(Buffer, FileName);

		animation_info *SampledAnimation = Manager->SampledAnimations + Index;
		*SampledAnimation = AnimationLoad(&Manager->Arena, Buffer);
		Assert(SampledAnimation);
		if(SampledAnimation)
		{
			SampledAnimation->Name = StringCopy(&Manager->Arena, AssetName);
			SampledAnimation->ReservedForChannel = PushStruct(&Manager->Arena, key_frame);
			key_frame *ReservedForChannel = SampledAnimation->ReservedForChannel;
			AllocateJointXforms(&Manager->Arena, ReservedForChannel, SampledAnimation->JointCount);
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

#if !RELEASE
	FontInitialize(&Manager->Arena, &Manager->Font, FontFiles[0]);
#endif

	//
	// Debug
	//

	// For hot reloading animation graph files
	Manager->XBotGraphFileInfo = {};
	Manager->XBotGraphFileInfo.Path = "../src/XBot_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->XBotGraphFileInfo.Path, &Manager->XBotGraphFileInfo.FileDate);

	Manager->YBotGraphFileInfo = {};
	Manager->YBotGraphFileInfo.Path = "../src/YBot_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->YBotGraphFileInfo.Path, &Manager->YBotGraphFileInfo.FileDate);

	Manager->PaladinGraphFileInfo = {};
	Manager->PaladinGraphFileInfo.Path = "../src/Paladin_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->PaladinGraphFileInfo.Path, &Manager->PaladinGraphFileInfo.FileDate);

	Manager->ArcherGraphFileInfo = {};
	Manager->ArcherGraphFileInfo.Path = "../src/Archer_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->ArcherGraphFileInfo.Path, &Manager->ArcherGraphFileInfo.FileDate);

	Manager->BruteGraphFileInfo = {};
	Manager->BruteGraphFileInfo.Path = "../src/Brute_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->BruteGraphFileInfo.Path, &Manager->BruteGraphFileInfo.FileDate);

	Manager->LevelFileInfo = {};
	Manager->LevelFileInfo.Path = "../src/test.level";
	Platform.DebugFileIsDirty(Manager->LevelFileInfo.Path, &Manager->LevelFileInfo.FileDate);

	// NOTE(Justin): Initialize a default capsule st it can be used for any entity.
	capsule Cap = CapsuleMinMaxRadius(V3(0.0f, 0.4f, 0.0f), V3(0.0f, 1.8f - 0.4f, 0.0f), 0.4f);
	Manager->Capsule = DebugModelCapsuleInitialize(&Manager->Arena, Cap.Min, Cap.Max, Cap.Radius);
	Manager->Cube = DebugModelCubeInitialize(&Manager->Arena);
	Manager->Sphere = DebugModelSphereInitialize(&Manager->Arena, 0.5f);

	Platform.UploadModelToGPU(&Manager->Capsule);
	Platform.UploadModelToGPU(&Manager->Cube);
	Platform.UploadModelToGPU(&Manager->Sphere);
}
#else
#endif
