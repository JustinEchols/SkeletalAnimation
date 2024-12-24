
inline b32
Button(ui *UI, ui_button *Button)
{
	b32 Result = false;

	if(InRect(Button->Rect, UI->MouseP))
	{
		UI->HotID = Button->ID;
		if(UI->ActiveID == 0 && UI->LeftClick)
		{
			UI->ActiveID = Button->ID;
		}
		else if(UI->ActiveID == Button->ID && UI->LeftClick)
		{
			UI->ActiveID = 0;
		}
		else if((UI->ActiveID != Button->ID) && UI->LeftClick)
		{
			UI->ActiveID = Button->ID;
		}
	}

	Result = (UI->ActiveID == Button->ID);

	return(Result);
}
