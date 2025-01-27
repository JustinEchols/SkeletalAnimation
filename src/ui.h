#if !defined(UI_H)

struct ui_id_value
{
	void *Void;
	u64 U64;
	u32 U32[2];
};

struct ui_id
{
	ui_id_value Value[2];
};

enum debug_variable_type
{
	DebugVariableType_Bool32,
	DebugVariableType_Int32,
	DebugVariableType_Float32,
	DebugVariableType_Vector2,
	DebugVariableType_Vector3,
	DebugVariableType_Vector4,
};

struct debug_variable
{
	char *Name;
	debug_variable *Next;
	debug_variable *Parent;

	union
	{
		b32 Bool32;
		s32 Int32;
		f32 Float32;
		v2 Vector2;
		v3 Vector3;
		v4 Vector4;
	};
};

struct debug_variable_group
{
	b32 Expanded;
	debug_variable *FirstChild;
};

struct ui
{
	b32 Initialized;

	v2 P;
	v2 MouseP;
	v2 DeltaMouse;
	rect Rect;

	b32 LeftDown;
	b32 LeftUp;

	s32 ActiveID;
	s32 HotID;
	font *Font;

	struct asset_manager *Assets;

	debug_variable_group PlayerGroup;

	b32 EntityTreeview;

	f32 AtY;
	f32 LeftEdge;

	render_buffer *RenderBuffer;
};

struct ui_button
{
	s32 ID;
	v2 P;
	rect Rect;
};

struct widget_treeview
{
	s32 ID;
	b32 Expanded;
	char *Label;
	rect Rect;
	v2 P;
};

//#define UiID(N) ((__LINE__ << 16) | ((N & 0xFFFF) ^ ((long) & __FILE__)))
#define UiID (char *)(FILE_AND_LINE)

#define IDFromU32s(A, B) IDFromU32s_((A), (B), (char *)(FILE_AND_LINE))
internal ui_id
IDFromU32s_(u32 A, u32 B, char *String)
{
	ui_id Result;
	Result.Value[0].U32[0] = A;
	Result.Value[0].U32[1] = B;
	Result.Value[1].Void = String;

	return(Result);
}

#define UI_H
#endif
