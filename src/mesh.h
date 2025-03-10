#if !defined(MESH_H)

struct joint_info
{
	u32 Count;
	u32 JointIndex[4];
	f32 Weights[4];
};

struct joint
{
	string Name;
	s32 ParentIndex;
	mat4 Transform;
};

struct material_spec
{
	v4 Ambient;
	v4 Diffuse;
	v4 Specular;

	f32 Shininess;
};

struct vertex 
{
	v3 P;
	v3 N;
	v2 UV;
	joint_info JointInfo;
};

enum material_flags
{
	MaterialFlag_Diffuse = (1 << 1),
	MaterialFlag_Specular = (1 << 2),
	MaterialFlag_Normal = (1 << 3),
};

enum mesh_flags
{
	MeshFlag_DontDraw = (1 << 1),
};

struct mesh
{
	string Name;

	u32 Flags;

	// Vertex
	u32 IndicesCount;
	u32 *Indices;

	u32 VertexCount;
	vertex *Vertices;

	// Skeleton 
	u32 JointCount;
	joint *Joints;
	mat4 BindTransform;
	mat4 *InvBindTransforms;
	mat4 *JointTransforms;
	mat4 *ModelSpaceTransforms;

	// Materials
	material_spec MaterialSpec;
	u32 MaterialFlags;
	u32 DiffuseTexture;
	u32 SpecularTexture;
	u32 NormalTexture;

	// Data modified by renderer. Remove this?
	u32 VA;
	u32 VB;
	u32 IBO;
	texture *Texture;

	aabb BoundingBox;
};

struct model
{
	string Name;

	b32 HasSkeleton;
	u32 MeshCount;
	mesh *Meshes;

	u32 LeftFootJointIndex;
	u32 RightFootJointIndex;
	u32 LeftHandJointIndex;
	u32 RightHandJointIndex;

	f32 Height;

	// Data modified by renderer.
	b32 UploadedToGPU;

	aabb BoundingBox;
};

#define MESH_H
#endif
