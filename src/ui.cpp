
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

internal void
UiBegin(ui *UI, game_input *GameInput, asset_manager *Assets)
{
	UI->MouseP = V2(GameInput->MouseX, GameInput->MouseY);
	UI->LeftClick = WasPressed(GameInput->MouseButtons[MouseButton_Left]);
	UI->HotID = 0;
	UI->Assets = Assets;
}

