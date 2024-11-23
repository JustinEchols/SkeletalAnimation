
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
	else
	{
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
	else
	{
	}

	return(Info);
}

char *LowerJointMasks[] =
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
	Animation_RunToStop,
};

char *AnimationFiles[] =
{
	"../data/XBot_IdleRight.animation",
	"../data/XBot_IdleLeft.animation",
	"../data/XBot_Running.animation",
	"../data/XBot_FastRun.animation",
	"../data/XBot_JumpForward.animation",
	"../data/XBot_RunningMirror.animation",
	"../data/XBot_FastRunMirror.animation",
	"../data/XBot_StandingToIdleRight.animation",
	"../data/XBot_StandingToIdleLeft.animation",
	"../data/XBot_IdleToSprint.animation",
	"../data/XBot_RunToStop.animation"
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
};

char *ModelFiles[] =
{
	"../data/XBot.mesh",
	"../data/Cube.mesh",
	"../data/Sphere.mesh",
	"../data/Arrow.mesh",
};

char *FontFiles[] =
{
	"c:/windows/fonts/arial.ttf",
};

internal void 
FileNameFromFullPath(char *FullPath, char *Buff)
{
	u64 OPLSlash = 0;
	for(char *C = FullPath; *C; ++C)
	{
		if(*C == '/')
		{
			OPLSlash = (C - FullPath) + 1;
		}
	}

	u32 At = 0;
	for(char *C = (FullPath + OPLSlash); *C; ++C)
	{
		if(*C == '.')
		{
			break;
		}

		Buff[At++] = *C;
	}
	Buff[At] = '\0';
}

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
		// TODO(Justin): All opengl allocations required on start up should be pooled together!
		OpenGLAllocateTexture(Texture);
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
		StringHashAdd(&Manager->ModelNames, Buffer, NameIndex);
		s32 Index = StringHashLookup(&Manager->ModelNames, Buffer);
		Assert(Index != -1);
		model *Model = Manager->Models + Index;
		*Model = ModelLoad(&Manager->Arena, FullPath);
	}

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
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = (AnimationFlags_JointMask | AnimationFlags_RemoveLocomotion);
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

				} break;
				case Animation_StandingToIdleRight:
				case Animation_StandingToIdleLeft:
				{
					Animation->TimeScale = 1.0f;
					Animation->DefaultFlags = (AnimationFlags_JointMask | AnimationFlags_RemoveLocomotion);
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
					Animation->DefaultFlags = AnimationFlags_RemoveLocomotion;
				} break;
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
		AnimationGraphInit(G, FullPath);
	}

	//
	// Font
	//

	FontInit(&Manager->Font, FontFiles[0]);
}


