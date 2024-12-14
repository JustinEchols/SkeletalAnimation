
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

internal model
ModelCircleInitialize(memory_arena *Arena, f32 Radius, u32 SectorCount, f32 Y)
{
	model Model = {};

	Model.MeshCount = 1;
	Model.Meshes = PushArray(Arena, Model.MeshCount, mesh);
	mesh *Mesh = Model.Meshes;
	Mesh->Name = StringCopy(Arena, "Circle");

	f32 SectorSize = 2.0f * Pi32 / (f32)SectorCount;

	Mesh->IndicesCount	= 2*SectorCount;
	Mesh->VertexCount	= SectorCount;
	Mesh->Indices	= PushArray(Arena, Mesh->IndicesCount, u32);
	Mesh->Vertices	= PushArray(Arena, Mesh->VertexCount, vertex);

	for(u32 SectorIndex = 0; SectorIndex < SectorCount; ++SectorIndex)
	{
		f32 SectorAngle = (f32)SectorIndex * SectorSize;
		f32 X = Radius * Cos(SectorAngle);
		f32 Z = Radius * Sin(SectorAngle);

		vertex *V = Mesh->Vertices + SectorIndex;
		V->P.x = X;
		V->P.y = Y;
		V->P.z = Z;
	}

	for(u32 Index = 0; Index < Mesh->IndicesCount/2; ++Index)
	{
		Mesh->Indices[2*Index] = Index;
		Mesh->Indices[2*Index + 1] = (Index + 1) % (Mesh->IndicesCount / 2);
	}

	return(Model);
}

internal mesh 
MeshCircleInitialize(memory_arena *Arena, f32 Radius, u32 SectorCount, f32 Y)
{
	mesh Mesh = {};

	Mesh.Name = StringCopy(Arena, "Circle");

	f32 SectorSize = 2.0f * Pi32 / (f32)SectorCount;

	Mesh.IndicesCount	= 2*SectorCount;
	Mesh.VertexCount	= SectorCount;
	Mesh.Indices		= PushArray(Arena, Mesh.IndicesCount, u32);
	Mesh.Vertices		= PushArray(Arena, Mesh.VertexCount, vertex);

	for(u32 SectorIndex = 0; SectorIndex < SectorCount; ++SectorIndex)
	{
		f32 SectorAngle = (f32)SectorIndex * SectorSize;
		f32 X = Radius * Cos(SectorAngle);
		f32 Z = Radius * Sin(SectorAngle);

		vertex *V = Mesh.Vertices + SectorIndex;
		V->P.x = X;
		V->P.y = Y;
		V->P.z = Z;
	}

	for(u32 Index = 0; Index < Mesh.IndicesCount/2; ++Index)
	{
		Mesh.Indices[2*Index] = Index;
		Mesh.Indices[2*Index + 1] = (Index + 1) % (Mesh.IndicesCount / 2);
	}

	return(Mesh);
}

internal model
ModelCapsuleInitialize(memory_arena *Arena, v3 Min, v3 Max, f32 Radius)
{
	model Model = {};

	Model.MeshCount = 4;
	Model.Meshes = PushArray(Arena, Model.MeshCount, mesh);
	mesh *Mesh = Model.Meshes;
	Mesh->Name = StringCopy(Arena, "Capsule0");

	u32 SectorCount = 64;
	f32 SectorAngleSize = Pi32 / (f32)SectorCount;
	v3 OffsetToOrigin = -0.5f*(Min + Max);

	Mesh->VertexCount	= 2*SectorCount;
	Mesh->IndicesCount	= 2*Mesh->VertexCount;
	Mesh->Indices	= PushArray(Arena, Mesh->IndicesCount, u32);
	Mesh->Vertices	= PushArray(Arena, Mesh->VertexCount, vertex);

	vertex *V = Mesh->Vertices;

	V->P = OffsetToOrigin + V3(Radius, Min.y, 0.0f);
	V++;
	for(u32 SectorIndex = 0; SectorIndex < SectorCount; ++SectorIndex)
	{
		f32 SectorAngle = SectorIndex * SectorAngleSize;
		f32 X = Radius * Cos(SectorAngle);
		f32 Y = Max.y + Radius * Sin(SectorAngle);
		f32 Z = 0.0f;

		V->P = OffsetToOrigin + V3(X, Y, Z);
		V++;
	}

	V->P = OffsetToOrigin + V3(-Radius, Max.y, 0.0f);
	V++;
	for(u32 SectorIndex = 0; SectorIndex < SectorCount; ++SectorIndex)
	{
		f32 SectorAngle = SectorIndex * SectorAngleSize;
		f32 X = -1.0f*Radius * Cos(SectorAngle);
		f32 Y = Min.y - Radius * Sin(SectorAngle);
		f32 Z = 0.0f;

		V->P = OffsetToOrigin + V3(X, Y, Z);
		V++;
	}

	for(u32 Index = 0; Index < Mesh->IndicesCount/2; Index++)
	{
		Mesh->Indices[2*Index] = Index;
		Mesh->Indices[2*Index + 1] = (Index + 1) % (Mesh->IndicesCount / 2);
	}

	Mesh++;
	Mesh->Name = StringCopy(Arena, "Capsule1");
	Mesh->VertexCount	= Model.Meshes[0].VertexCount;
	Mesh->IndicesCount	= Model.Meshes[0].IndicesCount;
	Mesh->Indices	= PushArray(Arena, 1, u32);
	Mesh->Indices	= Model.Meshes[0].Indices;
	Mesh->Vertices	= PushArray(Arena, Mesh->VertexCount, vertex);

	V = Mesh->Vertices;
	quaternion Q = Quaternion(V3(0.0f, 1.0f, 0.0f), DegreeToRad(90.0f));
	for(u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
	{
		V->P = Q * Model.Meshes[0].Vertices[VertexIndex].P;
		V++;
	}

	Mesh++;
	*Mesh = MeshCircleInitialize(Arena, Radius, SectorCount, OffsetToOrigin.y + Min.y);

	Mesh++;
	*Mesh = MeshCircleInitialize(Arena, Radius, SectorCount, OffsetToOrigin.y + Max.y);

	return(Model);
}
