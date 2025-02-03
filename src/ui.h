#if !defined(UI_H)

#define AlreadyInteracting()  (Ui.InteractingWith.ID != 0)
#define InteractingWith(ID)	  (Ui.InteractingWith.ID == (ID))
#define Hot(ID)			      (Ui.Hot.ID == (ID))

struct ui_id
{
	void *ID;
	s32 Index;
	void *Parent;
};

struct ui
{
	v2 P;
	v2 MouseP;
	v2 DeltaMouse;
	v3 HoverColor;
	v3 DefaultColor;

	b32 LeftDown;
	b32 LeftUp;

	ui_id InteractingWith;
	ui_id Hot;

	font *Font;
	struct asset_manager *Assets;

	u32 ToggleButtonCount;
	b32 ToggleButtonStates[16];

	f32 AtY;
	f32 LeftEdge;
	f32 LineGap;

	render_buffer *RenderBuffer;
	memory_arena *TempArena;
};

#define UI_H
#endif
