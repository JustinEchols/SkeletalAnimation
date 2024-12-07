
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

internal model
ModelCylinderInititalize(memory_arena *Arena)
{
	model Model = {};
	Model.Meshes = PushArray(Arena, 1, mesh);
	mesh *Mesh = Model.Meshes;
	Mesh->Name = StringCopy(Arena, "Cylinder");

	u32 SectorCount = 16;
	f32 SectorAngleSize = 2.0f * Pi32 / (f32)SectorCount;
	u32 CrossSectionCount = 2;
	f32 Height = 1.0f;
	f32 Radius = 1.0f;

	Mesh->IndicesCount = CrossSectionCount * SectorCount;
	Mesh->VertexCount = Mesh->IndicesCount;
	Mesh->Indices = PushArray(Arena, Mesh->IndicesCount, u32);
	Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, vertex);

	f32 Y = -Height/2.0f;
	u32 CrossSectionIndex = 0;
	for(u32 SectorIndex = 0; SectorIndex < SectorCount; ++ SectorIndex)
	{
		f32 Theta = SectorIndex * SectorAngleSize;
		f32 X = Radius * Cos(Theta); 
		f32 Z = Radius * -1.0f*Sin(Theta);

		vertex *V = Mesh->Vertices + (CrossSectionIndex * SectorCount) + SectorIndex;
		V->P = V3(X, Y, Z);
		V->N = V3(X, 0.0f, Z);

		Mesh->Indices[CrossSectionIndex * SectorCount + SectorIndex] = CrossSectionIndex * SectorCount + SectorIndex;
	}

	Y = Height/2.0f;
	CrossSectionIndex = 1;
	for(u32 SectorIndex = 0; SectorIndex < SectorCount; ++ SectorIndex)
	{
		f32 Theta = SectorIndex * SectorAngleSize;
		f32 X = Radius * Cos(Theta); 
		f32 Z = Radius * -1.0f*Sin(Theta);

		vertex *V = Mesh->Vertices + (CrossSectionIndex * SectorCount) + SectorIndex;
		V->P = V3(X, Y, Z);
		V->N = V3(X, 0.0f, Z);

		Mesh->Indices[CrossSectionIndex * SectorCount + SectorIndex] = CrossSectionIndex * SectorCount + SectorIndex;
	}

	return(Model);
}
