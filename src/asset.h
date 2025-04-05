#if !defined(ASSET_H)

#pragma pack(push, 1)
struct asset_mesh_info
{
	u8 Name[32];
	u32 IndicesCount;
	u32 VertexCount;
	u32 JointCount;
	material_spec MaterialSpec;
	mat4 XFormBind;

	u64 OffsetToIndices;
	u64 OffsetToVertices;
	u64 OffsetToJoints;
	u64 OffsetToInvBindXForms;
};

struct asset_joint_info
{
	u8 Name[32];
	s32 ParentIndex;
	mat4 Transform;
};

#define MODEL_FILE_MAGIC_NUMBER ((1 << 24) | (2 << 16) | (3 << 8) | (4 << 0))
#define MODEL_FILE_VERSION 0
struct asset_model_header
{
	u32 MagicNumber;
	u32 Version;
	b32 HasSkeleton;
	u32 MeshCount;

	u64 OffsetToMeshInfo;
};

struct asset_joint_name
{
	u8 Name[64];
};

struct asset_animation_info
{
	u64 OffsetToPositions;
	u64 OffsetToQuaternions;
	u64 OffsetToScales;
};

#define ANIMATION_FILE_MAGIC_NUMBER ((0x1 << 24) | (0x2 << 16) | (0x3 << 8) | (0x4 << 0))
#define ANIMATION_FILE_VERSION 0
struct asset_animation_header
{
	u32 MagicNumber;
	u32 Version;
	u32 JointCount;
	u32 KeyFrameCount;
	u32 TimeCount;
	u32 KeyFrameIndex;
	f32 CurrentTime;
	f32 Duration;
	f32 FrameRate;

	u64 OffsetToTimes;
	u64 OffsetToNames;
	u64 OffsetToAnimationInfo;
};
#pragma pack(pop)

struct asset_entry
{
	s32 Index;
	union
	{
		texture *Texture;
		model *Model;
		animation_graph *Graph;
		animation_info *SampledAnimation;
	};
};

struct asset_manager
{
	memory_arena Arena;

	string_hash TextureNames;
	u32 TextureCount;
	texture Textures[64];

	string_hash ModelNames;
	u32 ModelCount;
	model Models[16];

	// TODO(Justin): Clean this up
	string_hash AnimationNames;
	u32 AnimationCount;
	animation_info *SampledAnimations;

	string_hash GraphNames;
	u32 GraphCount;
	animation_graph Graphs[8];

	font Font;

	// File infos for hot reloading changes when they occur

	string_hash ReloadableNames;
	platform_file_info FileInfos[8];
	platform_file_info PlayerGraphFileInfo;
	//platform_file_info YBotGraphFileInfo;
	//platform_file_info PaladinGraphFileInfo;
	//platform_file_info ArcherGraphFileInfo;
	//platform_file_info BruteGraphFileInfo;
	platform_file_info LevelFileInfo;

	model Cube;
	model Sphere;
	model TestModel;
};

internal asset_entry FindTexture(asset_manager *AssetManager, char *TextureName);
internal asset_entry FindModel(asset_manager *AssetManager, char *ModelName);
internal asset_entry FindAnimation(asset_manager *AssetManager, char *AnimationName);
internal asset_entry FindGraph(asset_manager *AssetManager, char *AnimationGraphName);

#define ASSET_H
#endif
