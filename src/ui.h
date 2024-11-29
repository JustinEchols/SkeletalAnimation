#if !defined(UI_H)


struct ui
{
	v2 MouseP;
	b32 LeftClick;

	s32 ActiveID;
	s32 HotID;
};

struct ui_button
{
	s32 ID;
	v2 P;
	rect Rect;
};

#define UI_H
#endif
