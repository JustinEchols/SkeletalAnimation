
internal model 
ModelLoad(memory_arena *Arena, char *FileName)
{
	model Model = {};

	debug_file File = Win32FileReadEntire(FileName);
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

	return(Model);
}

internal animation_info 
AnimationLoad(memory_arena *Arena, char *FileName)
{
	animation_info Info = {};

	debug_file File = Win32FileReadEntire(FileName);
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

	return(Info);
}

