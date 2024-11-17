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

struct asset_manager
{
	memory_arena Arena;

	string_hash TextureNames;
	string_hash ModelNames;
	string_hash AnimationNames;
	string_hash GraphNames;

	texture Textures[32];
	model Models[16];
	animation_graph Graphs[2];
	font Font;

	// TODO(Justin): Clean this up
	animation_info *AnimationInfos;
	animation *Animations;
};

#define ASSET_H
#endif
