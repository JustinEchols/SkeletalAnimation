
internal void
UiBegin(ui *Ui, game_input *GameInput, asset_manager *Assets)
{
	Ui->MouseP = V2(GameInput->MouseX, GameInput->MouseY);
	Ui->Rect = RectMinDim(V2(0.0f), V2(100, 500.0f));
	Ui->LeftDown = IsDown(GameInput->MouseButtons[MouseButton_Left]);
	Ui->LeftUp = WasDown(GameInput->MouseButtons[MouseButton_Left]);
	Ui->HotID = 0;
	Ui->Assets = Assets;
	Ui->Font = &Assets->Font;
	Ui->P = V2(0.0f, (f32)GameInput->BackBufferHeight - Ui->Font->LineGap);
}

inline b32
UiWidgetUpdate(ui *Ui, s32 ID, b32 Over)
{
	b32 Result = false;

	// Interaction begin
	if(Ui->ActiveID == 0)
	{
		if(Over)
		{
			Ui->HotID = ID;
		}

		if((Ui->HotID == ID) && Ui->LeftDown)
		{
			Ui->ActiveID = ID;
		}
	}

	// Interaction end
	if(Ui->ActiveID == ID)
	{
		if(Over)
		{
			Ui->HotID = ID;
		}

		if(Ui->LeftUp)
		{
			if(Ui->HotID == ID)
			{
				Result = true;
			}

			Ui->ActiveID = 0;
		}
	}

	return(Result);
}

internal b32
Button(ui *Ui, v2 P, s32 ID, char *Label)
{
	rect Rect = RectMinDim(P, TextDim(Ui->Font, Ui->Font->Scale, Label));
	b32 Over = InRect(Rect, Ui->MouseP);
	b32 Result = UiWidgetUpdate(Ui, ID, Over);
	return(Result);
}
