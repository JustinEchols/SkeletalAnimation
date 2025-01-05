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

struct ui
{
	v2 MouseP;
	v2 DeltaMouse;

	b32 LeftClick;

	s32 ActiveID;
	s32 HotID;

	rect Rect;

	struct asset_manager *Assets;
};

struct ui_button
{
	s32 ID;
	v2 P;
	rect Rect;
};


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
