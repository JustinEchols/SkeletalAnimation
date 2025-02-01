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

	u32 ButtonCount;
	b32 ButtonState[16];

	b32 DebugEntityView;
	b32 DebugCollisionVolume;
	b32 DebugGroundArrow;
	b32 DebugAnimationPlayerView;
	b32 DebugDrawHandAndFoot;
	b32 DebugDrawTexture;
	b32 DebugDrawAnimation;

	f32 AtY;
	f32 LeftEdge;
	f32 LineGap;

	render_buffer *RenderBuffer;
	memory_arena *TempArena;
};

#define UI_H
#endif
