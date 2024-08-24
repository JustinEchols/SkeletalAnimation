
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

		if(Content)
		{
			Content[File.Size] = 0;
			string Data = String(Content);

			u8 Delimeters[] = "\n";
			u8 LineDelimeters[] = " \n\r";
			string Count = {};
			string Float = {};

			string_list Lines = StringSplit(Arena, Data, Delimeters, 1);
			string_node *Line = Lines.First;

			Model.HasSkeleton = false;
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

				Node = Node->Next;
				string Phong = Node->String;
				Phong.Data[Phong.Size] = 0;
				if(StringsAreSame(Phong, "Phong"))
				{
					Line = Line->Next;
					LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
					if(LineData.Count == 4)
					{
						Node = LineData.First;
						for(u32 Index = 0; Index < 4; ++Index)
						{
							Float = Node->String;
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
							Float = Node->String;
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
							Float = Node->String;
							Float.Data[Float.Size] = 0;
							Mesh->MaterialSpec.Specular.E[Index] = F32FromASCII(Float);
							Node = Node->Next;
						}

						Line = Line->Next;
						LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
						Assert(LineData.Count == 1);
						Node = LineData.First;
						Float = Node->String;
						Float.Data[Float.Size] = 0;
						Mesh->MaterialSpec.Shininess = F32FromASCII(Float);
					}
				}

				Line = Line->Next;

				u32 IndicesCount = 0;
				u32 *Indices;
				ParseMeshU32Array(Arena, &Indices, &IndicesCount, Line, LineDelimeters, 3, "INDICES:");

				Line = Line->Next;
				Line = Line->Next;
				u32 PositionsCount;
				f32 *Positions;
				ParseMeshF32Array(Arena, &Positions, &PositionsCount, Line, LineDelimeters, 3, "POSITIONS:");

				Line = Line->Next;
				Line = Line->Next;
				u32 NormalsCount;
				f32 *Normals;
				ParseMeshF32Array(Arena, &Normals, &NormalsCount, Line, LineDelimeters, 3, "NORMALS:");

				Line = Line->Next;
				Line = Line->Next;
				u32 UVCount;
				f32 *UV;
				ParseMeshF32Array(Arena, &UV, &UVCount, Line, LineDelimeters, 3, "UVS:");

				Line = Line->Next;
				Line = Line->Next;
				// TODO(Justin): Colors

				Line = Line->Next;
				LineData = StringSplit(Arena, Line->String, LineDelimeters, 3);
				Node = LineData.First;
				Header = Node->String;
				Header.Data[Header.Size] = 0;
				Assert(StringsAreSame(Header, "JOINT_INFO:"));

				Node = Node->Next;
				Count = Node->String;
				Count.Data[Count.Size] = 0;

				// TODO(Justin): This is ugly... find something better?

				u32 JointInfoCount = U32FromASCII(Count);
				joint_info *JointInfo = PushArray(Arena, JointInfoCount, joint_info);

				if(JointInfoCount != 0)
				{
					Model.HasSkeleton = true;
				}

				for(u32 Index = 0; Index < JointInfoCount; ++Index)
				{
					joint_info *Info = JointInfo + Index;

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

				Mesh->VertexCount = IndicesCount/3;
				Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, vertex_list);
				Mesh->IndicesCount = IndicesCount/3;
				Mesh->Indices = PushArray(Arena, Mesh->IndicesCount, u32);

				u32 Stride3 = 3;
				u32 Stride2 = 2;
				if(JointInfoCount == 0)
				{
					for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
					{
						Mesh->Indices[VertexIndex] = VertexIndex;
						vertex_list *Vertex = Mesh->Vertices + VertexIndex;

						u32 IndexP = Indices[3 * VertexIndex + 0];
						u32 IndexN = Indices[3 * VertexIndex + 1];
						u32 IndexUV = Indices[3 * VertexIndex + 2];

						Vertex->P.x = Positions[Stride3 * IndexP + 0];
						Vertex->P.y = Positions[Stride3 * IndexP + 1];
						Vertex->P.z = Positions[Stride3 * IndexP + 2];

						Vertex->N.x = Normals[Stride3 * IndexN + 0];
						Vertex->N.y = Normals[Stride3 * IndexN + 1];
						Vertex->N.z = Normals[Stride3 * IndexN + 2];

						Vertex->UV.x = UV[Stride2 * IndexUV + 0];
						Vertex->UV.y = UV[Stride2 * IndexUV + 1];
					}
				}
				else
				{
					for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
					{
						Mesh->Indices[VertexIndex] = VertexIndex;
						vertex_list *Vertex = Mesh->Vertices + VertexIndex;

						u32 IndexP = Indices[3 * VertexIndex + 0];
						u32 IndexN = Indices[3 * VertexIndex + 1];
						u32 IndexUV = Indices[3 * VertexIndex + 2];

						Vertex->P.x = Positions[Stride3 * IndexP + 0];
						Vertex->P.y = Positions[Stride3 * IndexP + 1];
						Vertex->P.z = Positions[Stride3 * IndexP + 2];

						Vertex->N.x = Normals[Stride3 * IndexN + 0];
						Vertex->N.y = Normals[Stride3 * IndexN + 1];
						Vertex->N.z = Normals[Stride3 * IndexN + 2];

						Vertex->UV.x = UV[Stride2 * IndexUV + 0];
						Vertex->UV.y = UV[Stride2 * IndexUV + 1];

						Vertex->JointInfo = JointInfo[IndexP];

					}
				}

				if(JointInfoCount != 0)
				{
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

					// TODO(Justin): This is not ok, probably has to do with the
					// way the file is enoded?
					RootJointName.Data[RootJointName.Size - 1] = 0;
					RootJointName.Size--;

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

						// TODO(Justin): This is not ok, probably has to do with the
						// way the file is enoded?
						string JointName = Line->String;
						JointName.Data[JointName.Size - 1] = 0;
						JointName.Size--;
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
		}
	}

	return(Model);
}

enum animation_header
{
	AnimationHeader_Invalid,
	AnimationHeader_Joints,
	AnimationHeader_Times,
	AnimationHeader_KeyFrame,
};

inline animation_header
AnimationHeaderGet(u8 *Buff)
{
	animation_header Result = AnimationHeader_Invalid;
	if(StringsAreSame(Buff, "JOINTS:"))
	{
		Result = AnimationHeader_Joints;
	}
	if(StringsAreSame(Buff, "TIMES:"))
	{
		Result = AnimationHeader_Times;
	}
	if(StringsAreSame(Buff, "KEY_FRAME:"))
	{
		Result = AnimationHeader_KeyFrame;
	}

	return(Result);
}

internal animation_info
AnimationInfoLoad(memory_arena *Arena, char *FileName)
{
	animation_info Info = {};
	debug_file File = Win32FileReadEntire(FileName);
	if(File.Size != 0)
	{
		u8 *Content = (u8 *)File.Content;
		if(Content)
		{
			Content[File.Size] = 0;
			string Data = String(Content);

			u8 Buff[512];
			u8 Delimeters[] = "\r\n";
			u32 TimeCount = 0;

			string_list Lines = StringSplit(Arena, Data, Delimeters, ArrayCount(Delimeters));
			string_node *Line = Lines.First;
			for(u32 Index = 0; Line && Line->Next; Line = Line->Next, ++Index)
			{
				if(Info.KeyFrameCount != 0)
				{
					// TODO(Justin): Figure out why we continue to have data
					// to process!!?!
					break;
				}

				u8 *C = Line->String.Data;
				BufferNextWord(&C, Buff);
				animation_header Header = AnimationHeaderGet(Buff);
				switch(Header)
				{
					case AnimationHeader_Joints:
					{
						EatSpaces(&C);
						BufferNextWord(&C, Buff);
						Info.JointCount = U32FromASCII(Buff);
						Info.JointNames = PushArray(Arena, Info.JointCount, string);

						for(u32 NameIndex = 0; NameIndex < Info.JointCount; ++NameIndex)
						{
							Line = Line->Next;
							C = Line->String.Data;
							BufferNextWord(&C, Buff);
							Info.JointNames[NameIndex] = StringAllocAndCopy(Arena, Buff);
						}
					} break;
					case AnimationHeader_Times:
					{
						EatSpaces(&C);
						BufferNextWord(&C, Buff);
						TimeCount = U32FromASCII(Buff);
						f32 *Times = PushArray(Arena, TimeCount, f32);

						Line = Line->Next;
						C = Line->String.Data;
						for(u32 TimeIndex = 0; TimeIndex < TimeCount; ++TimeIndex)
						{
							BufferNextWord(&C, Buff);
							Times[TimeIndex] = F32FromASCII(Buff);
							EatSpaces(&C);
						}

						Info.Duration = Times[TimeCount - 1];
						Info.FrameRate = Info.Duration / (f32)TimeCount;
					} break;
					case AnimationHeader_KeyFrame:
					{
						Info.KeyFrameCount = TimeCount;
						Info.KeyFrames = PushArray(Arena, Info.KeyFrameCount, key_frame);
						for(u32 KeyFrameIndex = 0; KeyFrameIndex < Info.KeyFrameCount; ++KeyFrameIndex)
						{
							Info.KeyFrames[KeyFrameIndex].Positions = PushArray(Arena, Info.JointCount, v3);
							Info.KeyFrames[KeyFrameIndex].Orientations = PushArray(Arena, Info.JointCount, quaternion);
							Info.KeyFrames[KeyFrameIndex].Scales = PushArray(Arena, Info.JointCount, v3);
						}

						for(u32 KeyFrameIndex = 0; KeyFrameIndex < Info.KeyFrameCount; ++KeyFrameIndex)
						{
							key_frame *KeyFrame = Info.KeyFrames + KeyFrameIndex;
							for(u32 JointIndex = 0; JointIndex < Info.JointCount; ++JointIndex)
							{
								v3 *P = KeyFrame->Positions + JointIndex;
								quaternion *Q = KeyFrame->Orientations + JointIndex;
								v3 *Scale = KeyFrame->Scales + JointIndex;

								Line = Line->Next;
								C = Line->String.Data;
								f32 *F32 = (f32 *)P;
								for(u32 FloatIndex = 0; FloatIndex < 3; ++FloatIndex)
								{
									BufferNextWord(&C, Buff);
									F32[FloatIndex] = F32FromASCII(Buff);
									EatSpaces(&C);
								}

								Line = Line->Next;
								C = Line->String.Data;
								F32 = (f32 *)Q;
								for(u32 FloatIndex = 0; FloatIndex < 4; ++FloatIndex)
								{
									BufferNextWord(&C, Buff);
									F32[FloatIndex] = F32FromASCII(Buff);
									EatSpaces(&C);
								}

								Line = Line->Next;
								C = Line->String.Data;
								F32 = (f32 *)Scale;
								for(u32 FloatIndex = 0; FloatIndex < 3; ++FloatIndex)
								{
									BufferNextWord(&C, Buff);
									F32[FloatIndex] = F32FromASCII(Buff);
									EatSpaces(&C);
								}
							}

#if 1
							if(Line->Next)
							{
								Line = Line->Next;
							}
							else
							{
								break;
							}
#endif
						}
					} break;
				}
			}
		}
	}
	return(Info);
}

