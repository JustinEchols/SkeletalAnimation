#if !defined(MESH_H)

struct joint_info
{
	u32 Count;
	u32 JointIndex[3];
	f32 Weights[3];
};

struct joint
{
	string Name;
	s32 ParentIndex;
	mat4 Transform;
};

struct key_frame
{
	v3 *Positions;
	quaternion *Orientations;
	v3 *Scales;
};

struct animation_info
{
	u32 JointCount;
	u32 KeyFrameCount;

	f32 CurrentTime;
	f32 Duration;
	f32 FrameRate;

	string *JointNames;
	key_frame *KeyFrames;
};

struct animations
{
	u32 Count;
	u32 Index;
	animation_info *Info;
};

struct material_spec
{
	v4 Ambient;
	v4 Diffuse;
	v4 Specular;

	f32 Shininess;
};

struct vertex_list
{
	v3 P;
	v3 N;
	v2 UV;

	joint_info JointInfo;
};

struct mesh
{
	string Name;

	u32 IndicesCount;
	u32 VertexCount;
	u32 JointCount;

	u32 *Indices;
	vertex_list *Vertices;

	string *JointNames;
	joint *Joints;

	mat4 BindTransform;
	mat4 *InvBindTransforms;
	mat4 *JointTransforms;
	mat4 *ModelSpaceTransforms;

	material_spec MaterialSpec;

	u32 TextureHandle;
};

struct model
{
	basis Basis;

	b32 HasSkeleton;

	u32 MeshCount;
	mesh *Meshes;

	// TODO(Justin): Should this be stored at the mesh level?
	u32 VA[2];
	u32 VB[2];
	u32 IBO[2];

	animations Animations;
};

#define MESH_H
#endif
