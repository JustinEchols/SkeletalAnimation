
internal u32
MeshHeaderNumberGet(memory_arena *Arena, string_node *Line, u8 *LineDelimeters, u32 DelimCount)
{
	u32 Result = 0;

	string_list LineData = StringSplit(Arena, Line->String, LineDelimeters, DelimCount);
	string_node *Node = LineData.First;
	Node = Node->Next;
	string Value = Node->String;

	Result = U32FromASCII(Value.Data);

	return(Result);
}

internal void
ParseMeshU32Array(memory_arena *Arena, u32 **U32, u32 *Count, string_node *Line, u8 *Delimeters, u32 DelimCount,
		char *HeaderName)
{
	string_list LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	string_node *Node = LineData.First;
	string Header = Node->String;
	Header.Data[Header.Size] = 0;
	Assert(StringsAreSame(Header, HeaderName));
	Node = Node->Next;
	string StrCount = Node->String;
	StrCount.Data[StrCount.Size] = 0;

	*Count  = U32FromASCII(StrCount.Data);
	*U32 = PushArray(Arena, *Count, u32);

	Line = Line->Next;
	LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	Assert(LineData.Count == *Count);
	Node = LineData.First;

	u32 *A = *U32;
	for(u32 Index = 0; Index < *Count; ++Index)
	{
		string Value = Node->String;
		Value.Data[Value.Size] = 0;
		A[Index] = U32FromASCII(Value);
		Node = Node->Next;
	}
}

internal void
ParseMeshF32Array(memory_arena *Arena, f32 **F32, u32 *Count, string_node *Line, u8 *Delimeters, u32 DelimCount,
		char *HeaderName)
{
	string_list LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	string_node *Node = LineData.First;
	string Header = Node->String;
	Header.Data[Header.Size] = 0;
	Assert(StringsAreSame(Header, HeaderName));
	Node = Node->Next;
	string StrCount = Node->String;
	StrCount.Data[StrCount.Size] = 0;

	*Count  = U32FromASCII(StrCount.Data);
	*F32 = PushArray(Arena, *Count, f32);

	Line = Line->Next;
	LineData = StringSplit(Arena, Line->String, Delimeters, DelimCount);
	Assert(LineData.Count == *Count);
	Node = LineData.First;

	f32 *A = *F32;
	for(u32 Index = 0; Index < *Count; ++Index)
	{
		string Value = Node->String;
		Value.Data[Value.Size] = 0;
		A[Index] = F32FromASCII(Value);
		Node = Node->Next;
	}
}

internal b32
DoneProcessingMesh(string_node *Line)
{
	b32 Result = (Line->String.Data[0] == '*');
	return(Result);
}

internal model
ModelLoad(memory_arena *Arena, char *FileName)
{
	model Model = {};

	debug_file File = Win32FileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;
		Content[File.Size] = 0;
		string Data = String(Content);

		u8 Delimeters[] = "\n";
		u8 LineDelimeters[] = " \n\r";
		string Count = {};

		string_list Lines = StringSplit(Arena, Data, Delimeters, 1);
		string_node *Line = Lines.First;

		Model.MeshCount = MeshHeaderNumberGet(Arena, Line, LineDelimeters, 3);
		Model.Meshes = PushArray(Arena, Model.MeshCount, mesh);

		for(u32 MeshIndex = 0; MeshIndex < Model.MeshCount; ++MeshIndex)
		{
			mesh *Mesh = Model.Meshes + MeshIndex;

			Line = Line->Next;
			string_list LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			string_node *Node = LineData.First;
			string Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "MESH:"));

			Node = Node->Next;
			string MeshName = Node->String;
			MeshName.Data[MeshName.Size] = 0;
			Mesh->Name = MeshName;

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "LIGHTING:"));

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 4);
			Node = LineData.First;

			for(u32 Index = 0; Index < 4; ++Index)
			{
				string Float = Node->String;
				Float.Data[Float.Size] = 0;
				Mesh->MaterialSpec.Ambient.E[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 4);
			Node = LineData.First;

			for(u32 Index = 0; Index < 4; ++Index)
			{
				string Float = Node->String;
				Float.Data[Float.Size] = 0;
				Mesh->MaterialSpec.Diffuse.E[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 4);
			Node = LineData.First;

			for(u32 Index = 0; Index < 4; ++Index)
			{
				string Float = Node->String;
				Float.Data[Float.Size] = 0;
				Mesh->MaterialSpec.Specular.E[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Assert(LineData.Count == 1);
			Node = LineData.First;
			string Float = Node->String;
			Float.Data[Float.Size] = 0;
			Mesh->MaterialSpec.Shininess = F32FromASCII(Float);

			Line = Line->Next;
			ParseMeshU32Array(Arena, &Mesh->Indices, &Mesh->IndicesCount, Line, LineDelimeters, 3, "INDICES:");

			Line = Line->Next;

			Line = Line->Next;
			Line = Line->Next;
			ParseMeshF32Array(Arena, &Mesh->Positions, &Mesh->PositionsCount, Line, LineDelimeters, 3, "POSITIONS:");

			Line = Line->Next;
			Line = Line->Next;
			ParseMeshF32Array(Arena, &Mesh->Normals, &Mesh->NormalsCount, Line, LineDelimeters, 3, "NORMALS:");

			Line = Line->Next;
			Line = Line->Next;
			ParseMeshF32Array(Arena, &Mesh->UV, &Mesh->UVCount, Line, LineDelimeters, 3, "UVS:");

			Line = Line->Next;
			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "JOINT_INFO:"));

			Node = Node->Next;
			Count = Node->String;
			Count.Data[Count.Size] = 0;

			Mesh->JointInfoCount = U32FromASCII(Count);
			Mesh->JointsInfo = PushArray(Arena, Mesh->JointInfoCount, joint_info);
			for(u32 Index = 0; Index < Mesh->JointInfoCount; ++Index)
			{
				joint_info *Info = Mesh->JointsInfo + Index;

				Line = Line->Next;

				Count = Line->String;
				Count.Data[Count.Size] = 0;

				Info->Count = U32FromASCII(Count);

				Line = Line->Next;
				string_list IndicesList = StringSplit(Arena, Line->String, LineDelimeters, 3);

				Node = IndicesList.First;
				string StrIndex = Node->String;
				StrIndex.Data[StrIndex.Size] = 0;
				Info->JointIndex[0] = U32FromASCII(StrIndex);

				Node = Node->Next;
				StrIndex = Node->String;
				StrIndex.Data[StrIndex.Size] = 0;
				Info->JointIndex[1] = U32FromASCII(StrIndex);

				Node = Node->Next;
				StrIndex = Node->String;
				StrIndex.Data[StrIndex.Size] = 0;
				Info->JointIndex[2] = U32FromASCII(StrIndex);

				Line = Line->Next;
				string_list WeightsList = StringSplit(Arena, Line->String, LineDelimeters, 3);

				Node = WeightsList.First;
				string Weight = Node->String;
				Weight.Data[Weight.Size] = 0;
				Info->Weights[0] = F32FromASCII(Weight);

				Node = Node->Next;
				Weight = Node->String;
				Weight.Data[Weight.Size] = 0;
				Info->Weights[1] = F32FromASCII(Weight);

				Node = Node->Next;
				Weight = Node->String;
				Weight.Data[Weight.Size] = 0;
				Info->Weights[2] = F32FromASCII(Weight);
			}

			Line = Line->Next;
			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "JOINTS:"));

			Node = Node->Next;
			Count = Node->String;
			Count.Data[Count.Size] = 0;

			Mesh->JointCount = U32FromASCII(Count);
			Mesh->Joints = PushArray(Arena, Mesh->JointCount, joint);
			Mesh->JointNames = PushArray(Arena, Mesh->JointCount, string);
			Mesh->JointTransforms = PushArray(Arena, Mesh->JointCount, mat4);
			Mesh->ModelSpaceTransforms = PushArray(Arena, Mesh->JointCount, mat4);

			Line = Line->Next;
			string RootJointName = Line->String;
			RootJointName.Data[RootJointName.Size] = 0;

			Mesh->Joints[0].Name = RootJointName;
			Mesh->Joints[0].ParentIndex = -1;

			Line = Line->Next;
			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			f32 *T = &Mesh->Joints[0].Transform.E[0][0];
			for(u32 Index = 0; Index < 16; Index++)
			{
				Float = Node->String;
				Float.Data[Float.Size] = 0;
				T[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			for(u32 Index = 1; Index < Mesh->JointCount; ++Index)
			{
				Line = Line->Next;

				joint *Joint = Mesh->Joints + Index;

				string JointName = Line->String;
				JointName.Data[JointName.Size] = 0;
				Joint->Name = JointName;

				Line = Line->Next;
				string ParentIndex = Line->String;
				ParentIndex.Data[ParentIndex.Size] = 0;
				Joint->ParentIndex = U32FromASCII(ParentIndex);

				Line = Line->Next;
				LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
				Node = LineData.First;
				T = &Joint->Transform.E[0][0];
				for(u32 k = 0; k < 16; k++)
				{
					Float = Node->String;
					Float.Data[Float.Size] = 0;
					T[k] = F32FromASCII(Float);
					Node = Node->Next;
				}
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "BIND:"));

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			f32 *BindT = &Mesh->BindTransform.E[0][0];
			for(u32 Index = 0; Index < 16; Index++)
			{
				Float = Node->String;
				Float.Data[Float.Size] = 0;
				BindT[Index] = F32FromASCII(Float);
				Node = Node->Next;
			}

			Line = Line->Next;
			LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
			Node = LineData.First;
			Header = Node->String;
			Header.Data[Header.Size] = 0;
			Assert(StringsAreSame(Header, "INV_BIND:"));

			Node = Node->Next;
			Count = Node->String;
			Count.Data[Count.Size] = 0;
			u32 InvBindTCount = U32FromASCII(Count);
			Assert(InvBindTCount == (Mesh->JointCount * 16));

			Mesh->InvBindTransforms = PushArray(Arena, Mesh->JointCount, mat4);
			for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
			{
				Line = Line->Next;
				LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
				Node = LineData.First;

				mat4 *InvBindT = Mesh->InvBindTransforms + Index;
				T = &InvBindT->E[0][0];

				for(u32 k = 0; k < 16; k++)
				{
					Float = Node->String;
					Float.Data[Float.Size] = 0;
					T[k] = F32FromASCII(Float);
					Node = Node->Next;
				}
			}

			Line = Line->Next;
			Line = Line->Next;
			Assert(DoneProcessingMesh(Line));

			mat4 I = Mat4Identity();
			for(u32 Index = 0; Index < Mesh->JointCount; ++Index)
			{
				joint *Joint = Mesh->Joints + Index;

				Mesh->JointNames[Index] = Joint->Name;
				Mesh->JointTransforms[Index] = I;
				Mesh->ModelSpaceTransforms[Index] = I;
			}
		}
	}

	return(Model);
}

internal animation_info
AnimationInfoLoad(memory_arena *Arena, char *FileName)
{
	animation_info Info = {};
	debug_file File = Win32FileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;
		Content[File.Size] = 0;
		string Data = String(Content);
		string Header = {};
		string Count = {};

		char Delimeters[] = "\n\r";
		char LineDelimeters[] = " \n\r";

		string_list Lines = StringSplit(Arena, Data,(u8 *)Delimeters, 1);
		string_node *Line = Lines.First;

		string_list LineData = StringSplit(Arena, Line->String,(u8 *)LineDelimeters, 3);
		string_node *Node = LineData.First;
		Header = Node->String;
		Header.Data[Header.Size] = 0;
		Assert(StringsAreSame(Header, "JOINTS:"));

		Node = Node->Next;
		Count = Node->String;
		Count.Data[Count.Size] = 0;

		Info.JointCount = U32FromASCII(Count);

		Line = Line->Next;
		LineData = StringSplit(Arena, Line->String,(u8 *)LineDelimeters, 3);
		Node = LineData.First;
		Header = Node->String;
		Header.Data[Header.Size] = 0;
		Assert(StringsAreSame(Header, "TIMES:"));

		Node = Node->Next;
		Count = Node->String;
		Count.Data[Count.Size] = 0;

		Info.TimeCount = U32FromASCII(Count);

		Line = Line->Next;
		LineData = StringSplit(Arena, Line->String,(u8 *)LineDelimeters, 3);
		Node = LineData.First;
		Header = Node->String;
		Header.Data[Header.Size] = 0;
		Assert(StringsAreSame(Header, "TRANSFORMS:"));

		Node = Node->Next;
		Count = Node->String;
		Count.Data[Count.Size] = 0;

		Info.TransformCount = U32FromASCII(Count);
		Assert(Info.TimeCount == Info.TransformCount);

		Line = Line->Next;
		Assert(Line->String.Data[0] == '*');

		Info.JointNames = PushArray(Arena, Info.JointCount, string);
		Info.Times = PushArray(Arena, Info.JointCount, f32 *);
		Info.Transforms = PushArray(Arena, Info.JointCount, mat4 *);

		for(u32 JointIndex = 0; JointIndex < Info.JointCount; ++JointIndex)
		{
			Info.Times[JointIndex] = PushArray(Arena, Info.TimeCount, f32);
			Info.Transforms[JointIndex] = PushArray(Arena, Info.TimeCount, mat4);

			Line = Line->Next;
			string JointName = Line->String;
			//JointName.Data[JointName.Size - 1] = 0;
			//JointName.Size--;
			Info.JointNames[JointIndex] = JointName;

			Line = Line->Next;
			string_array StrTimes = StringSplitIntoArray(Arena, Line->String,(u8 *)LineDelimeters, 3);

			Line = Line->Next;
			string_array StrTransforms = StringSplitIntoArray(Arena, Line->String,(u8 *)LineDelimeters, 3);

			for(u32 TimeIndex = 0; TimeIndex < Info.TimeCount; ++TimeIndex)
			{
				string Time = StrTimes.Strings[TimeIndex];
				Info.Times[JointIndex][TimeIndex] = F32FromASCII(Time);
			}

			for(u32 MatrixIndex = 0; MatrixIndex < Info.TransformCount; ++MatrixIndex)
			{
				mat4 *M = &Info.Transforms[JointIndex][MatrixIndex];
				f32 *Float = &M->E[0][0];
				for(u32 FloatIndex = 0; FloatIndex < 16; ++FloatIndex)
				{
					string StrFloat = StrTransforms.Strings[16 * MatrixIndex + FloatIndex];
					Float[FloatIndex] = F32FromASCII(StrFloat);
				}
			}
		}
	}
	else
	{
		printf("Error could not read %s\n", FileName);
		perror("");
	}

	return(Info);
}
