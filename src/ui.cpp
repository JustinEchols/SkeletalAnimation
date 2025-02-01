
internal void
UiBegin(render_buffer *RenderBuffer, memory_arena *TempArena, game_input *GameInput, asset_manager *Assets)
{
	Ui.MouseP = V2(GameInput->MouseX, GameInput->MouseY);
	Ui.LeftDown = IsDown(GameInput->MouseButtons[MouseButton_Left]);
	Ui.LeftUp = WasDown(GameInput->MouseButtons[MouseButton_Left]);
	Ui.Hot.ID = 0;
	Ui.Assets = Assets;
	Ui.Font = &Assets->Font;
	Ui.P = V2(0.0f, (f32)GameInput->BackBufferHeight - Ui.Font->LineGap);
	Ui.AtY = Ui.P.y;
	Ui.LineGap = Ui.Font->LineGap + 5.0f;
	Ui.RenderBuffer = RenderBuffer;
	Ui.TempArena = TempArena;
	Ui.HoverColor = V3(1.0f, 1.0f, 0.0f);
	Ui.DefaultColor = V3(1.0f);
	Ui.ButtonCount = 0;
}

internal b32
UiWidgetUpdate(void *ID, b32 Over)
{
	b32 Result = false;

	if(!AlreadyInteracting())
	{
		if(Over)
		{
			Ui.Hot.ID = ID;
		}

		if(Hot(ID) && Ui.LeftDown)
		{
			Ui.InteractingWith.ID = ID;
		}
	}

	if(InteractingWith(ID))
	{
		if(Over)
		{
			Ui.Hot.ID = ID;
		}

		if(Ui.LeftUp)
		{
			if(Hot(ID))
			{
				Result = true;
			}

			Ui.InteractingWith.ID = 0;
		}
	}

	return(Result);
}

internal b32
UiButton(char *Label, void *ID)
{
	rect Rect = RectMinDim(Ui.P, TextDim(Ui.Font, Ui.Font->Scale, Label));
	b32 Over = InRect(Rect, Ui.MouseP);
	b32 Result = UiWidgetUpdate(ID, Over);
	return(Result);
}
