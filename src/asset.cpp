internal model
ObjLoad(memory_arena *Arena, char *FileName)
{
	model Result = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		//TODO(Justin): Log error. 
		Assert(0);
	}

	text_file_handler Handler_ = TextFileHandlerInitialize(File.Content);
	text_file_handler *Handler = &Handler_;

	char MaterialFileName[256];
	MemoryZero(MaterialFileName, sizeof(MaterialFileName));

	u32 MeshCount = 0;
	u32 PositionCount = 0;
	u32 NormalCount = 0;
	u32 UVCount = 0;
	u32 FaceCount = 0;

	while(IsValid(Handler))
	{
		BufferAndAdvanceALine(Handler);

		switch(Handler->Word[0])
		{
			case 'm':
			{
				if(StringsAreSame(Handler->Word, "mtllib"))
				{
					BufferAndAdvanceAWord(Handler);
					StringConcat(MaterialFileName, Handler->Word);
					Assert(StringLen(MaterialFileName) != 0);
				}
			} break;
			case 'v':
			{
				if(Handler->Word[1] == 'n')
				{
					NormalCount++;
				}
				else if(Handler->Word[1] == 't')
				{
					UVCount++;
				}
				else
				{
					PositionCount++;
				}

			} break;
			case 'f':
			{
				while(LineIsValid(Handler))
				{
					BufferAndAdvanceAWord(Handler);
					FaceCount++; //A face is made up of three of these, choose a better name
				}
			} break;
			case 'u':
			{
				//
				// NOTE(Justin): Assume that anytime a usemtl is visited this is a material for a new mesh which
				// also means that another mesh exists.
				// TODO(Justin): Determine if this is actually the case.
				//

				if(StringsAreSame(Handler->Word, "usemtl"))
				{
					MeshCount++;
				}
			} break;
		}
	}

	Result.Version = 2;
	Result.MeshCount = MeshCount;
	Result.Meshes	= PushArray(Arena, Result.MeshCount, mesh);

	// TODO(Justin): Use temporary memory for raw model data 
	v3 *Positions	= PushArray(Arena, PositionCount, v3);
	v3 *Normals		= PushArray(Arena, NormalCount, v3);
	v2 *UVs			= PushArray(Arena, UVCount, v2);
	u32 *IndicesP	= PushArray(Arena, FaceCount, u32);
	u32 *IndicesN	= PushArray(Arena, FaceCount, u32);
	u32 *IndicesUV	= PushArray(Arena, FaceCount, u32);

	u32 MeshIndex = 0;
	u32 PositionIndex = 0;
	u32 NormalIndex = 0;
	u32 UVIndex = 0;
	u32 IndexP = 0;
	u32 IndexUV = 0;
	u32 IndexN = 0;

	//
	// NOTE(Justin): Reset file pointer, clear buffers, parse data
	//

	TextFileHandlerReset(Handler, File.Content);

	// TODO(Justin): Maybe seek to the data then parse since we have counts of everything.

	mesh *Mesh = Result.Meshes;
	while(IsValid(Handler))
	{
		BufferAndAdvanceALine(Handler);
		switch(Handler->Word[0])
		{
			case 'v':
			{
				if(Handler->Word[1] == 'n')
				{
					v3 *N = Normals + NormalIndex;
					ParseV3(&Handler->Line, Handler->Word, N);
					NormalIndex++;
				}
				else if(Handler->Word[1] == 't')
				{
					v2 *UV = UVs + UVIndex;
					ParseV2(&Handler->Line, Handler->Word, UV);
					UVIndex++;
				}
				else
				{
					v3 *P = Positions + PositionIndex;
					ParseV3(&Handler->Line, Handler->Word, P);
					PositionIndex++;
				}
			} break;
			case 'f':
			{
				//
				// NOTE(Justin): Face/triangle, v/vt/vn, 1 - indexed
				//

				while(LineIsValid(Handler))
				{
					BufferAndAdvanceAWord(Handler);
					u8 *IndicesPointer = Handler->Word;

					u8 Number[64];
					MemoryZero(Number, sizeof(Number));

					BufferNumber(&IndicesPointer, Number);
					IndicesPointer++;
					IndicesP[IndexP++] = U32FromASCII(Number) - 1;

					BufferNumber(&IndicesPointer, Number);
					IndicesPointer++;
					IndicesUV[IndexUV++] = U32FromASCII(Number) - 1;

					BufferNumber(&IndicesPointer, Number);
					IndicesPointer++;
					IndicesN[IndexN++] = U32FromASCII(Number) - 1;

					Mesh->VertexCount++;
					Mesh->IndicesCount++;
				}
			} break;
			case 'u':
			{
				if(StringsAreSame(Handler->Word, "usemtl"))
				{
					BufferAndAdvanceAWord(Handler);
					if(MeshIndex == 0)
					{
						if(Mesh->VertexCount == 0)
						{
							Mesh->Name = StringCopy(Arena, Handler->Word);
						}
						else
						{
							MeshIndex++;
							Mesh = Result.Meshes + MeshIndex;
							Mesh->Name = StringCopy(Arena, Handler->Word);
						}
					}
					else
					{
						MeshIndex++;
						Mesh = Result.Meshes + MeshIndex;
						Mesh->Name = StringCopy(Arena, Handler->Word);
					}
				}
			} break;
		}
	}

	Assert(PositionCount == PositionIndex);
	Assert(NormalCount == NormalIndex);
	Assert(UVCount == UVIndex);

	//
	// NOTE(Justin): Initialze model. Also the indices of all the meshes are stored in one array.
	// We compute an offset into the array (IndexOffsetForMesh) after copying data to the mesh.
	//

	u32 IndexOffsetForMesh = 0;
	for(MeshIndex = 0; MeshIndex < Result.MeshCount; ++MeshIndex)
	{
		Mesh = Result.Meshes + MeshIndex;
		Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, vertex);
		Mesh->Indices = PushArray(Arena, Mesh->IndicesCount, u32);

		for(u32 Index = 0; Index < Mesh->VertexCount; ++Index)
		{
			u32 IP = IndicesP[IndexOffsetForMesh + Index];
			u32 IN = IndicesN[IndexOffsetForMesh + Index];
			u32 IUV = IndicesUV[IndexOffsetForMesh + Index];

			v3 P = Positions[IP];
			v3 N = Normals[IN];
			v2 UV = UVs[IUV];

			vertex *V = Mesh->Vertices + Index;

			V->P = P;
			V->N = N;
			V->UV = UV;

			Mesh->Indices[Index] = Index;
		}

		IndexOffsetForMesh += (Mesh->IndicesCount);
	}

	Platform.DebugFileFree(File.Content);

	return(Result);
}

internal model
TestLoadModel(memory_arena *Arena, char *FileName)
{
	model Model = {};

	debug_file File = Platform.DebugFileReadEntire(FileName);
	if(File.Size == 0)
	{
		Assert(0);
	}

	u8 *Content = (u8 *)File.Content;

	u8 LineBuffer_[4096];
	MemoryZero(LineBuffer_, sizeof(LineBuffer_));
	u8 *LineBuffer = LineBuffer_;

	u8 Word_[4096];
	MemoryZero(Word_, sizeof(Word_));
	u8 *Word = Word_;

	BufferLine(&Content, LineBuffer);
	u8 *Line = LineBuffer;
	BufferNextWord(&Line, Word);
	Assert(StringsAreSame(Word, "mesh_count"));

	EatSpaces(&Line);
	BufferNextWord(&Line, Word);

	Model.Path = StringCopy(Arena, FileName);
	Model.Version = 2;
	Model.HasSkeleton = true;
	Model.MeshCount = U32FromASCII(Word);
	Model.Meshes = PushArray(Arena, Model.MeshCount, mesh);


	AdvanceLine(&Content);
	u32 MeshCount = 0;
	mesh *Mesh = 0;
	while(*Content)
	{
		BufferLine(&Content, LineBuffer);
		Line = LineBuffer;
		BufferNextWord(&Line, Word);

		if(StringsAreSame(Word, "mesh"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			Mesh = Model.Meshes + MeshCount++;
			Mesh->Name = StringCopy(Arena, Word);
		}
		else if(StringsAreSame(Word, "vertex_count"))
		{
			EatSpaces(&Line);
			BufferNextWord(&Line, Word);
			Mesh->VertexCount = U32FromASCII(Word);
			Mesh->IndicesCount = Mesh->VertexCount;
			Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, vertex);
			Mesh->Indices = PushArray(Arena, Mesh->IndicesCount, u32);
		}
		else if(StringsAreSame(Word, "diffuse"))
		{
			ParseV4(&Line, Word, &Mesh->MaterialSpec.Diffuse);
		}
		else if(StringsAreSame(Word, "specular"))
		{
			ParseV4(&Line, Word, &Mesh->MaterialSpec.Specular);
		}
		else if(StringsAreSame(Word, "specular_intensity"))
		{
			ParseFloat(&Line, Word, &Mesh->MaterialSpec.Shininess);
		}
		else if(StringsAreSame(Word, "vertices"))
		{
			AdvanceLine(&Content);
			for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
			{
				vertex *V = Mesh->Vertices + VertexIndex;

				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "p"));

				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->P.x = F32FromASCII(Word);

					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->P.y = F32FromASCII(Word);

					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->P.z = F32FromASCII(Word);
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "n"));

				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->N.x = F32FromASCII(Word);

					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->N.y = F32FromASCII(Word);

					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->N.z = F32FromASCII(Word);
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "uv"));

				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->UV.x = F32FromASCII(Word);

					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->UV.y = F32FromASCII(Word);
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "joint_count"));

				{

					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					V->JointInfo.Count = U32FromASCII(Word);
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "joint_indices"));

				{
					for(u32 Index = 0; Index < V->JointInfo.Count; ++Index)
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						V->JointInfo.JointIndex[Index] = U32FromASCII(Word);
					}
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "joint_weights"));

				{
					for(u32 Index = 0; Index < V->JointInfo.Count; ++Index)
					{
						EatSpaces(&Line);
						BufferNextWord(&Line, Word);
						V->JointInfo.Weights[Index] = F32FromASCII(Word);
					}
				}

				AdvanceLine(&Content);
			}
		}
		else if(StringsAreSame(Word, "indices"))
		{
			AdvanceLine(&Content);
			for(u32 Index = 0; Index < Mesh->IndicesCount; ++Index)
			{
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);

				Mesh->Indices[Index] = U32FromASCII(Word);

				AdvanceLine(&Content);
			}
		}
		else if(StringsAreSame(Word, "Armature"))
		{
			AdvanceLine(&Content);
			BufferLine(&Content, LineBuffer);
			Line = LineBuffer;
			BufferNextWord(&Line, Word);
			Assert(StringsAreSame(Word, "joint_count"));

			EatSpaces(&Line);
			BufferNextWord(&Line, Word);

			Model.JointCount = U32FromASCII(Word);
			Model.Joints = PushArray(Arena, Model.JointCount, joint);
			Model.BindTransform = Mat4Identity();
			Model.InvBindTransforms = PushArray(Arena, Model.JointCount, mat4);
			Model.JointTransforms = PushArray(Arena, Model.JointCount, mat4);
			Model.ModelSpaceTransforms = PushArray(Arena, Model.JointCount, mat4);

			AdvanceLine(&Content);
			for(u32 JointIndex = 0; JointIndex < Model.JointCount; ++JointIndex)
			{
				joint *Joint = Model.Joints + JointIndex;

				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);

				{
					Joint->Name = StringCopy(Arena, Word);
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;
				BufferNextWord(&Line, Word);
				Assert(StringsAreSame(Word, "parent_index"));

				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					Joint->ParentIndex = S32FromASCII(Word);
				}

				AdvanceLine(&Content);
				BufferLine(&Content, LineBuffer);
				Line = LineBuffer;

				for(u32 i = 0; i < 16; ++i)
				{
					EatSpaces(&Line);
					BufferNextWord(&Line, Word);
					Joint->Transform.I[i] = F32FromASCII(Word);
				}

				AdvanceLine(&Content);
			}
		}

		AdvanceLine(&Content);
	}

	mat4 I = Mat4Identity();
	for(u32 JointIndex = 0; JointIndex < Model.JointCount; ++JointIndex)
	{
		Model.InvBindTransforms[JointIndex] = JointInverseBindTransform(Model.Joints, Model.Joints[JointIndex]);
		Model.JointTransforms[JointIndex] = I;
		Model.ModelSpaceTransforms[JointIndex] = I;
	}

#if 0
	Model.Meshes[0].MaterialSpec.Ambient = V4(0.000000, 0.000000, 0.000000, 1.000000); 
	Model.Meshes[0].MaterialSpec.Diffuse = V4(0.266667, 0.099623, 0.080812, 1.000000);
	Model.Meshes[0].MaterialSpec.Specular = V4(0.299138, 0.299138, 0.299138, 1.000000);
	Model.Meshes[0].MaterialSpec.Shininess = 2.000000;

	Model.Meshes[1].MaterialSpec.Ambient = V4(0.0f, 0.0f, 0.0f, 1.0f);
	Model.Meshes[1].MaterialSpec.Diffuse = V4(0.669600, 0.241846, 0.210924, 1.000000);
	Model.Meshes[1].MaterialSpec.Specular = V4(0.487175, 0.487175, 0.487175, 1.000000);
	Model.Meshes[1].MaterialSpec.Shininess = 3.675214;
#endif

	//
	// NOTE(Justin): Header
	//

	//
	// NOTE(Justin): Mesh data 
	//

	Platform.DebugFileFree(File.Content);

	return(Model);
}

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

	Model.Path = StringCopy(Arena, FileName);
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
		// TODO(Justin): Think about removing these from the file format? (since we can compute the inverse bind transform..)
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


			//
			// NOTE(Justin): The inverse bind transforms are part of the file format. This was to test to see if we could compute them
			//

			Mesh->InvBindTransforms[JointIndex] = JointInverseBindTransform(Mesh->Joints, *Dest);

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

				if(StringsAreSame(CString(Dest->Name), "mixamorig_Weapon"))
				{
					Model.WeaponJointIndex = JointIndex;
				}

				if(StringsAreSame(CString(Dest->Name), "mixamorig_Sword_joint"))
				{
					Model.WeaponJointIndex = JointIndex;
				}
			}
		}

		// AABB info

		f32 Xmin, Ymin, Zmin;
		f32 Xmax, Ymax, Zmax;
		Xmin = Ymin = Zmin = F32Max;
		Xmax = Ymax = Zmax = -1.0f*F32Max;
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

		v3 Min = V3(Xmin, Ymin, Zmin);
		v3 Max = V3(Xmax, Ymax, Zmax);
		Mesh->BoundingBox = AABBMinMax(Min, Max);

		Model.BoundingBox = AABBUnion(Model.BoundingBox, Mesh->BoundingBox);

		if((SubStringExists(CString(Mesh->Name), "Axe")) ||
		   (SubStringExists(CString(Mesh->Name), "Shield")))
		{
			Mesh->Flags |= MeshFlag_Weapon;
			// TODO(Justin): Need to compute an offset from the weapon joint index
			// to correctly center the aabb when using the weapon joint as the position of
			// the aabb
		}

		MeshSource++;
	}

	//
	// HACK TESTING VERSION 2
	//

	//
	// NOTE(Justin): Model/mesh data structure revision to 2 moves skeleton to model level.
	// The mesh bin file format has not been updated so we have to hack the version here..
	// The current file format DUPLICATES THE SKELTON FOR EACH MESH. When first learning
	// I thought this was required. But it is not. Each vertex of the mesh knows what joints
	// affect it so there is no need to duplicate the skeleton. We send the all the runtime
	// transforms to the gpu and each vertex of each mesh can index into the array to read
	// which transforms affect it 
	//

	Model.Version = 2;
	mesh *Mesh = Model.Meshes;
	Model.JointCount = Mesh->JointCount;
	Model.Joints = Mesh->Joints;
	Model.InvBindTransforms = Mesh->InvBindTransforms;
	Model.JointTransforms = Mesh->JointTransforms;
	Model.ModelSpaceTransforms = Mesh->ModelSpaceTransforms;

	return(Model);
}

internal void
SaveModelJointTransforms(model *Model, char *FileName)
{
	FILE *Test = fopen(FileName, "w");
	char Buffer[256];
	MemoryZero(Buffer, sizeof(Buffer));

	char *Fmt = "%f %f %f %f";
	f32 A[4];
	for(u32 JointIndex = 0; JointIndex < Model->JointCount; ++JointIndex)
	{
		joint *Joint = Model->Joints + JointIndex;
		sprintf(Buffer, "%s\n", CString(Joint->Name));
		u32 Length = StringLen(Buffer);
		fwrite(Buffer, sizeof(u8), Length, Test);
		for(u32 Row = 0; Row < 4; ++Row)
		{
			for(u32 Col = 0; Col < 4; ++Col)
			{
				A[Col] = Joint->Transform.E[Row][Col];
			}

			MemoryZero(Buffer, sizeof(Buffer));
			sprintf(Buffer, Fmt, A[0], A[1], A[2], A[3]);
			Length = StringLen(Buffer);
			fwrite(Buffer, sizeof(u8), Length, Test);

			Buffer[0] = '\n';
			fwrite(Buffer, sizeof(u8), 1, Test);
		}
		fwrite(Buffer, sizeof(u8), 1, Test);
	}

	fclose(Test);
}

internal void
SaveModelJointTransformsInModelSpace(model *Model, char *FileName)
{
	FILE *Test = fopen(FileName, "w");
	char Buffer[256];
	MemoryZero(Buffer, sizeof(Buffer));

	char *Fmt = "%f %f %f %f";
	f32 A[4];
	for(u32 JointIndex = 0; JointIndex < Model->JointCount; ++JointIndex)
	{
		joint *Joint = Model->Joints + JointIndex;
		if(JointIndex == 0)
		{
			Model->JointTransforms[JointIndex] = Joint->Transform;
		}
		else
		{
			mat4 ParentT = Model->JointTransforms[Joint->ParentIndex];
			Model->JointTransforms[JointIndex] = ParentT * Joint->Transform;
		}
		mat4 JointTransform = Model->JointTransforms[JointIndex];

		sprintf(Buffer, "%s\n", CString(Joint->Name));
		u32 Length = StringLen(Buffer);
		fwrite(Buffer, sizeof(u8), Length, Test);
		for(u32 Row = 0; Row < 4; ++Row)
		{
			for(u32 Col = 0; Col < 4; ++Col)
			{
				//A[Col] = Joint->Transform.E[Row][Col];
				A[Col] = JointTransform.E[Row][Col];
			}

			MemoryZero(Buffer, sizeof(Buffer));
			sprintf(Buffer, Fmt, A[0], A[1], A[2], A[3]);
			Length = StringLen(Buffer);
			fwrite(Buffer, sizeof(u8), Length, Test);

			Buffer[0] = '\n';
			fwrite(Buffer, sizeof(u8), 1, Test);
		}
		fwrite(Buffer, sizeof(u8), 1, Test);
	}

	fclose(Test);
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

char *AnimationDirectory			= "..\\data\\animations";
char *AnimationDirectoryAndWildCard = "..\\data\\animations\\*";

char *GraphDirectory				= "..\\data\\graphs";
char *GraphDirectoryAndWildCard		= "..\\data\\graphs\\*";

char *FontFiles[] =
{
	"c:/windows/fonts/arial.ttf",
};

internal asset_entry 
FindTexture(asset_manager *AssetManager, char *TextureName)
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
FindModel(asset_manager *AssetManager, char *ModelName)
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
FindAnimation(asset_manager *AssetManager, char *AnimationName)
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
FindGraph(asset_manager *AssetManager, char *AnimationGraphName)
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

	ArenaSubset(&Manager->Arena, &Manager->TextureNames.Arena, Kilobyte(2));
	StringHashInitialize(&Manager->TextureNames);
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

		Texture->Path = StringCopy(&Manager->Arena, Buffer);
		Texture->Name = StringCopy(&Manager->Arena, (char *)AssetName);
		*Texture = TextureLoad(Buffer);

		Manager->TextureCount++;
	}

	//
	// Models
	//

	ArenaSubset(&Manager->Arena, &Manager->ModelNames.Arena, Kilobyte(2));
	StringHashInitialize(&Manager->ModelNames);

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
		strcat(Buffer, "\\");
		strcat(Buffer, FileName);

		if(StringsAreSame(ExtBuffer, "mesh"))
		{
			//
			// NOTE(Justin): Testing blender exported file format...
			//

			if(StringsAreSame(AssetName, "XBot"))
			{
#if 1
				*Model = TestLoadModel(&Manager->Arena, "test/Beta_Joints.mesh");
#else
				*Model = ModelLoad(&Manager->Arena, Buffer);
#endif
			}
			else
			{
				*Model = ModelLoad(&Manager->Arena, Buffer);
			}

			if(StringsAreSame(AssetName, "Cube"))
			{
				asset_entry Entry = FindTexture(Manager, "orange_texture_02");
				Assert(Entry.Texture);
				Model->Meshes[0].DiffuseTexture = Entry.Index;
				Model->Meshes[0].MaterialFlags |= MaterialFlag_Diffuse;
			}
			if(StringsAreSame(AssetName, "PaladinWithProp"))
			{
				asset_entry Diffuse = FindTexture(Manager, "Paladin_diffuse");
				asset_entry Specular = FindTexture(Manager, "Paladin_specular");
				Assert(Diffuse.Texture);
				Assert(Specular.Texture);

				for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
				{
					mesh *Mesh = Model->Meshes + MeshIndex;
					Mesh->DiffuseTexture = Diffuse.Index;
					Mesh->SpecularTexture = Specular.Index;
					Mesh->MaterialFlags |= (MaterialFlag_Diffuse | MaterialFlag_Specular);
				}
			}
			if(StringsAreSame(AssetName, "Brute"))
			{
				asset_entry Diffuse = FindTexture(Manager, "axe_diffuse");
				asset_entry Specular = FindTexture(Manager, "axe_specular");

				Model->Meshes[0].DiffuseTexture = Diffuse.Index;
				Model->Meshes[0].SpecularTexture = Specular.Index;
				Model->Meshes[0].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = FindTexture(Manager, "MaleBruteA_Hair_diffuse");
				Specular = FindTexture(Manager, "MaleBruteA_Hair_specular");
				Model->Meshes[1].DiffuseTexture = Diffuse.Index;
				Model->Meshes[1].SpecularTexture = Specular.Index;
				Model->Meshes[1].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = FindTexture(Manager, "MaleBruteA_Bottom_diffuse1");
				Specular = FindTexture(Manager, "MaleBruteA_Bottom_specular");
				Model->Meshes[2].DiffuseTexture = Diffuse.Index;
				Model->Meshes[2].SpecularTexture = Specular.Index;
				Model->Meshes[2].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = FindTexture(Manager, "MaleBruteA_Shoes_diffuse1");
				Specular = FindTexture(Manager, "MaleBruteA_Shoes_specular");
				Model->Meshes[3].DiffuseTexture = Diffuse.Index;
				Model->Meshes[3].SpecularTexture = Specular.Index;
				Model->Meshes[3].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = FindTexture(Manager, "MaleBruteA_Moustache_diffuse");
				Specular = FindTexture(Manager, "MaleBruteA_Moustache_specular");
				Model->Meshes[4].DiffuseTexture = Diffuse.Index;
				Model->Meshes[4].SpecularTexture = Specular.Index;
				Model->Meshes[4].MaterialFlags = (MaterialFlag_Diffuse | MaterialFlag_Specular);

				Diffuse = FindTexture(Manager, "MaleBruteA_Body_diffuse1");
				Model->Meshes[5].DiffuseTexture = Diffuse.Index;
				Model->Meshes[5].MaterialFlags = (MaterialFlag_Diffuse);

				Diffuse = FindTexture(Manager, "MaleBruteA_Body_diffuse");
				Model->Meshes[6].DiffuseTexture = Diffuse.Index;
				Model->Meshes[6].MaterialFlags = (MaterialFlag_Diffuse);

				Diffuse = FindTexture(Manager, "MaleBruteA_Body_diffuse1");
				Model->Meshes[7].DiffuseTexture = Diffuse.Index;
				Model->Meshes[7].MaterialFlags = (MaterialFlag_Diffuse);
			}
		}
		else if(StringsAreSame(ExtBuffer, "obj"))
		{
			*Model = ObjLoad(&Manager->Arena, Buffer);
			asset_entry Diffuse1 = FindTexture(Manager, "barrel");
			asset_entry Diffuse2 = FindTexture(Manager, "planks");

			for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
			{
				mesh *Mesh = Model->Meshes + MeshIndex;
				if(StringsAreSame(Mesh->Name, "barrel"))
				{
					Mesh->DiffuseTexture = Diffuse1.Index;
				}
				else
				{
					Mesh->DiffuseTexture = Diffuse2.Index;
				}

				Mesh->MaterialFlags |= MaterialFlag_Diffuse;
			}
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

		Manager->ModelCount++;
	}

	//
	// Animations
	//

	file_group_info AnimationFileGroup = Platform.DebugFileGroupLoad(AnimationDirectoryAndWildCard);
	ArenaSubset(&Manager->Arena, &Manager->AnimationNames.Arena, Kilobyte(2));
	StringHashInitialize(&Manager->AnimationNames);
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
		strcat(Buffer, "\\");
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

		Manager->AnimationCount++;
	}

	//
	// Graphs
	//

	file_group_info GraphFileGroup = Platform.DebugFileGroupLoad(GraphDirectoryAndWildCard);
	ArenaSubset(&Manager->Arena, &Manager->GraphNames.Arena, Kilobyte(2));
	StringHashInitialize(&Manager->GraphNames);
	for(u32 FileIndex = 0; FileIndex < GraphFileGroup.Count; ++FileIndex)
	{
		char *FileName = GraphFileGroup.FileNames[FileIndex];
		if(FileName[0] == '.') continue;

		ExtFromFullPath(FileName, ExtBuffer);
		Assert(StringsAreSame(ExtBuffer, "animation_graph"));
		FileNameFromFullPath(FileName, AssetName);
		StringHashAdd(&Manager->GraphNames, AssetName, FileIndex);
		s32 Index = StringHashLookup(&Manager->GraphNames, AssetName);
		Assert(Index != -1);

		MemoryZero(Buffer, sizeof(Buffer));
		strcat(Buffer, GraphDirectory);
		strcat(Buffer, "\\");
		strcat(Buffer, FileName);

		animation_graph *G = Manager->Graphs + Index;
		ArenaSubset(&Manager->Arena, &G->Arena, Kilobyte(4));
		AnimationGraphInitialize(G, Buffer);

		Manager->GraphCount++;
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
#if 0
	Manager->XBotGraphFileInfo = {};
	Manager->XBotGraphFileInfo.Path = "../data/graphs/XBot_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->XBotGraphFileInfo.Path, &Manager->XBotGraphFileInfo.FileDate);

	Manager->YBotGraphFileInfo = {};
	Manager->YBotGraphFileInfo.Path = "../data/graphs/YBot_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->YBotGraphFileInfo.Path, &Manager->YBotGraphFileInfo.FileDate);

	Manager->PaladinGraphFileInfo = {};
	Manager->PaladinGraphFileInfo.Path = "../data/graphs/Paladin_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->PaladinGraphFileInfo.Path, &Manager->PaladinGraphFileInfo.FileDate);

	Manager->ArcherGraphFileInfo = {};
	Manager->ArcherGraphFileInfo.Path = "../data/graphs/Archer_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->ArcherGraphFileInfo.Path, &Manager->ArcherGraphFileInfo.FileDate);

	Manager->BruteGraphFileInfo = {};
	Manager->BruteGraphFileInfo.Path = "../data/graphs/Brute_AnimationGraph.animation_graph";
	Platform.DebugFileIsDirty(Manager->BruteGraphFileInfo.Path, &Manager->BruteGraphFileInfo.FileDate);
#else

	ArenaSubset(&Manager->Arena, &Manager->ReloadableNames.Arena, Kilobyte(2));
	StringHashInitialize(&Manager->ReloadableNames);
	for(u32 GraphIndex = 0; GraphIndex < ArrayCount(Manager->Graphs); ++GraphIndex)
	{
		animation_graph *G = Manager->Graphs + GraphIndex;
		if(G->NodeCount == 0) continue;

		StringHashAdd(&Manager->ReloadableNames, CString(G->Name), GraphIndex);
		s32 Index = StringHashLookup(&Manager->ReloadableNames, CString(G->Name));
		Assert((Index >= 0) && (Index < ArrayCount(Manager->FileInfos)));

		platform_file_info *FileInfo = Manager->FileInfos + Index;
		*FileInfo = {};
		FileInfo->Path = CString(G->Path);
		Platform.DebugFileIsDirty(FileInfo->Path, &FileInfo->FileDate);
	}
#endif

	Manager->LevelFileInfo = {};
	Manager->LevelFileInfo.Path = "../src/test.level";
	Platform.DebugFileIsDirty(Manager->LevelFileInfo.Path, &Manager->LevelFileInfo.FileDate);

	Manager->Cube = DebugModelCubeInitialize(&Manager->Arena);
	Manager->Sphere = DebugModelSphereInitialize(&Manager->Arena, 0.5f);

	Platform.UploadModelToGPU(&Manager->Cube);
	Platform.UploadModelToGPU(&Manager->Sphere);
}
