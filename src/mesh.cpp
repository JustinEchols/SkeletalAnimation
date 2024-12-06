
internal s32
JointIndexGet(string *JointNames, u32 JointCount, string JointName)
{
	s32 Result = -1;
	for(u32 Index = 0; Index < JointCount; ++Index)
	{
		string *Name = JointNames + Index;
		if(StringsAreSame(*Name, JointName))
		{
			Result = Index;
			break;
		}
	}

	return(Result);
}

internal f32
ModelHeight(model *Model)
{
	f32 MaxHeight = 0.0f;
	for(u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
	{
		mesh *Mesh = Model->Meshes + MeshIndex;
		for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
		{
			vertex *Vertex = Mesh->Vertices + VertexIndex;
			if(Vertex->P.y > MaxHeight)
			{
				MaxHeight = Vertex->P.y;
			}
		}
	}

	return(MaxHeight);
}

internal v3
JointPositionGet(model *Model, u32 JointIndex)
{
	mesh *Mesh = Model->Meshes;
	Assert(JointIndex >= 0 && JointIndex < Mesh->JointCount);
	v3 Result = Mat4ColumnGet(Mesh->Joints[JointIndex].Transform, 3);
	return(Result);
}

internal v3
JointPositionGet(mesh *Mesh, u32 JointIndex)
{
	Assert(JointIndex >= 0 && JointIndex < Mesh->JointCount);
	v3 Result = Mat4ColumnGet(Mesh->Joints[JointIndex].Transform, 3);
	return(Result);
}
