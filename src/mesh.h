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

struct vertex 
{
	v3 P;
	v3 N;
	v2 UV;
	joint_info JointInfo;
};

struct material_spec
{
	v4 Ambient;
	v4 Diffuse;
	v4 Specular;

	f32 Shininess;
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
	MeshFlag_Weapon = (1 << 2),
};

struct mesh_weapon_info
{
	u32 MeshIndex;
	s32 JointIndex;
};

// NOTE(Justin): A meshes vertices in Tpose are defined in relation to 1 or more model space joint transforms
// During a calculation of an animation pose we calculate joint transforms for the current pose. If we
// apply the joint transforms then we would be adding in 
//
// The inverse bind transform of a joint is just the inverse of the model space joint transform. We need
// to use this when updating the final transforms in order not to apply the original model space joint transform
// twice. The vertices of a mesh in Tpose are defined in relation to the model space transform of one or more joints.
// So in a sense the vertices in Tpose have already been transformed by these model space joint transforms.
// In order to calculate the new position, orientation, scale of a vertex we need to apply a delta to it
// So we take runtime model space joint transform and multiply it by the inverse bind transform.
struct mesh
{
	string Name;

	u32 Flags;

	// Vertex
	u32 IndicesCount;
	u32 *Indices;

	u32 VertexCount;
	vertex *Vertices;

	// TODO(Justin): Remove this from the mesh! It is now at the model level.
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

	// Data modified by renderer. Let the renderer keep track of this?!?
	u32 VA;
	u32 VB;
	u32 IBO;
	texture *Texture;

	aabb BoundingBox;
};

struct model
{
	string Name;

	u32 Version;
	u32 MeshCount;
	mesh *Meshes;

	b32 HasSkeleton;
	u32 LeftFootJointIndex;
	u32 RightFootJointIndex;
	u32 LeftHandJointIndex;
	u32 RightHandJointIndex;
	u32 WeaponJointIndex;
	f32 Height;

	// TODO(Justin): Compute this using union of mesh aabbs;
	aabb BoundingBox;

	// TODO(Justin): Mesh contains material index into array of textures
	u32 MaterialCount;
	//texture Materials[32];
	texture *Materials[32];

	//
	// TEST NEW SKELETON
	//
	// Skeleton 
	u32 JointCount;
	joint *Joints;
	mat4 BindTransform;
	mat4 *InvBindTransforms;
	mat4 *JointTransforms;
	mat4 *ModelSpaceTransforms;
};

#define MESH_H
#endif
