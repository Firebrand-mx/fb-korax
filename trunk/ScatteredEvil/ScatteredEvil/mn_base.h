//==========================================================================
//
//	KMenuItem_t
//
//==========================================================================

class KMenuItem_t:public KWindow
{
	DECLARE_CLASS(KMenuItem_t, KWindow, 0);

	KMenuItem_t()
	{
		Width = 320;
		Height = ITEM_HEIGHT;
		bIsSelectable = true;
	}
};
IMPLEMENT_CLASS(KMenuItem_t);

//==========================================================================
//
//	KMenuUIChoice
//
//==========================================================================

class KMenuUIChoice:public KMenuItem_t
{
	DECLARE_CLASS(KMenuUIChoice, KMenuItem_t, 0);
	
	char *ActionText;

	KMenuUIChoice(void)
	{
		Font = KCanvas::SmallFont;
		ActionText = "Choice";
	}

	void InitWindow(void)
	{
		Super::InitWindow();

		LoadSetting();
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(0, 0, ActionText);
	}

	virtual void LoadSetting(void)
	{
	}

	virtual void SaveSetting(void)
	{
	}
};
IMPLEMENT_CLASS(KMenuUIChoice);

//==========================================================================
//
//	KMenuUIChoiceSlider
//
//==========================================================================

class KMenuUIChoiceSlider:public KMenuUIChoice
{
	DECLARE_CLASS(KMenuUIChoiceSlider, KMenuUIChoice, 0);

	int TickPosition;
	int NumTicks;
	float StartValue;
	float EndValue;

	int *pInteger;
	float *pFloat;
	char *CVarName;
	cvar_t *pCVar;

	KTexture *TexMid1;
	KTexture *TexMid2;
	KTexture *TexLeft;
	KTexture *TexRight;
	KTexture *TexKnob;

	KMenuUIChoiceSlider(void)
	{
		ActionText = "Slider choice";
		NumTicks = 11;
		EndValue = 10.0;
	}

	void InitWindow(void)
	{
		if (CVarName)
		{
			pCVar = CvarGet(CVarName);
			if (pCVar->type == CVT_INT)
			{
				pInteger = (int *)pCVar->ptr;
			}
			else if (pCVar->type == CVT_FLOAT)
			{
				pFloat = (float *)pCVar->ptr;
			}
			StartValue = pCVar->min;
			EndValue = pCVar->max;
		}

		Super::InitWindow();

		TexMid1 = FindTexture("M_SLDMD1");
		TexMid2 = FindTexture("M_SLDMD2");
		TexLeft = FindTexture("M_SLDLT");
		TexRight = FindTexture("M_SLDRT");
		TexKnob = FindTexture("M_SLDKB");
	}

	void CycleNextValue(void)
	{
		int NewValue;

		NewValue = GetTickPosition();
		if (NewValue < NumTicks - 1)
		{
			NewValue++;
		}
		SetTickPosition(NewValue);
	}

	void CyclePreviousValue(void)
	{
		int NewValue;

		NewValue = GetTickPosition();
		if (NewValue > 0)
		{
			NewValue--;
		}
		SetTickPosition(NewValue);
	}

	int GetTickPosition(void)
	{
		return TickPosition;
	}

	void SetTickPosition(int NewPosition)
	{
		TickPosition = NewPosition;
	}

	void SetValue(float NewValue)
	{
		SetTickPosition(int((NewValue - StartValue) * (NumTicks - 1) / 
			(EndValue - StartValue)));
	}

	float GetValue(void)
	{
		return StartValue + GetTickPosition() * (EndValue - StartValue) / 
			(NumTicks - 1);
	}

	void LoadSetting(void)
	{
		if (pInteger)
		{
			SetValue(float(*pInteger));
		}
		else if (pFloat)
		{
			SetValue(*pFloat);
		}
	}

	void SaveSetting(void)
	{
		if (pInteger)
		{
			*pInteger = int(GetValue());
		}
		else if (pFloat)
		{
			*pFloat = GetValue();
		}
	}

	void DrawWindow(KGC *gc)
	{
		int x;
		int y;
		int i;

		Super::DrawWindow(gc);

		x = 24 + 160;
		y = 0;
		for (i = 0; i < 12; i++)
		{
			gc->DrawIcon(x + i * 8, y, i & 1 ?  TexMid2 : TexMid1);
		}
		gc->DrawIcon(x - 32, y, TexLeft);
		gc->DrawIcon(x + 96, y, TexRight);
		gc->DrawIcon(x - 2 + TickPosition * 100 / (NumTicks - 1), y + 7, TexKnob);
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_LEFTARROW:
			CyclePreviousValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;

		case DDKEY_RIGHTARROW:
		case DDKEY_ENTER:
			CycleNextValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuUIChoiceSlider);

//==========================================================================
//
//	KMenuUIChoiceEnum
//
//==========================================================================

class KMenuUIChoiceEnum:public KMenuUIChoice
{
	DECLARE_CLASS(KMenuUIChoiceEnum, KMenuUIChoice, 0);

	enum { MAX_ENUM_COUNT = 40 };

	const char *EnumText[MAX_ENUM_COUNT];

	int CurrentValue;

	int *pInteger;
	char *CVarName;

	KMenuUIChoiceEnum(void)
	{
	}

	void InitWindow(void)
	{
		if (CVarName)
		{
			cvar_t *pCVar;
			pCVar = CvarGet(CVarName);
			pInteger = (int *)pCVar->ptr;
		}
		Super::InitWindow();
	}

	void CycleNextValue(void)
	{
		int NewValue;

		// Cycle to the next value, but make sure we don't exceed the 
		// bounds of the enumText array.  If we do, start back at the 
		// bottom.

		NewValue = GetValue() + 1;

		if (NewValue == MAX_ENUM_COUNT)
			NewValue = 0;
		else if (!EnumText[NewValue])
			NewValue = 0;

		SetValue(NewValue);
	}

	void CyclePreviousValue(void)
	{
		int NewValue;

		NewValue = GetValue() - 1;

		if (NewValue < 0)
		{
			NewValue = MAX_ENUM_COUNT - 1;

			while (!EnumText[NewValue] && NewValue > 0)
				NewValue--;	
		}

		SetValue(NewValue);
	}

	void SetValue(int NewValue)
	{
		CurrentValue = NewValue;
	}

	int GetValue(void)
	{
		return CurrentValue;
	}

	void LoadSetting(void)
	{
		if (pInteger)
		{
			SetValue(*pInteger);
		}
	}

	void SaveSetting(void)
	{
		if (pInteger)
		{
			*pInteger = GetValue();
		}
	}

	void DrawWindow(KGC *gc)
	{
		Super::DrawWindow(gc);

		gc->DrawText(160, 0, EnumText[CurrentValue]);
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_LEFTARROW:
			CyclePreviousValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;

		case DDKEY_RIGHTARROW:
		case DDKEY_ENTER:
			CycleNextValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuUIChoiceEnum);

//==========================================================================
//
//	KMenuChoice_OnOff
//
//==========================================================================

class KMenuChoice_OnOff:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_OnOff, KMenuUIChoiceEnum, 0);

	KMenuChoice_OnOff(void)
	{
		EnumText[0] = "OFF";
		EnumText[1] = "ON";
	}
};
IMPLEMENT_CLASS(KMenuChoice_OnOff);

//==========================================================================
//
//	KMenuUIFileSlot
//
//==========================================================================

class KMenuUIFileSlot:public KMenuItem_t
{
	DECLARE_CLASS(KMenuUIFileSlot, KMenuItem_t, 0);

	int SlotIndex;

	KMenuUIFileSlot(void)
	{
		Font = KCanvas::SmallFont;
	}

	void PrepareSlot(int InIndex)
	{
		SlotIndex = InIndex;

		if (!slottextloaded)
		{
			MN_LoadSlotText();
		}
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawIcon(0, 0, FindTexture("M_FSLOT"));
		if (_SlotStatus[SlotIndex])
		{
			gc->DrawText(5, 5, _SlotText[SlotIndex]);
		}
	}
};
IMPLEMENT_CLASS(KMenuUIFileSlot);

//==========================================================================
//
//	KMenuUIButton
//
//==========================================================================

class KMenuUIButton:public KMenuItem_t
{
	DECLARE_CLASS(KMenuUIButton, KMenuItem_t, 0);

	char *ButtonText;

	KMenuUIButton(void)
	{
		ButtonText = "Button";
		Font = KCanvas::BigFont;
	}

	void SetButtonText(char *NewText)
	{
		ButtonText = NewText;
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(0, 0, ButtonText);
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_ENTER:
			GetParent()->ButtonActivated(this);
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuUIButton);

//==========================================================================
//
//	KMenuScreen
//
//==========================================================================

class KMenuScreen:public KModalWindow
{
	DECLARE_CLASS(KMenuScreen, KModalWindow, 0);

	struct FButtonDefault
	{
		char *Name;
		int Action;
		MenuType_t Invoke;
		int Key;

		FButtonDefault(void) { }
		FButtonDefault(char *InName, int InAction,
			MenuType_t InInvoke = MENU_NONE, int InKey = 0)
			: Name(InName), Action(InAction), Invoke(InInvoke), Key(InKey)
		{ }
	};

	enum { MAX_BUTTONS = 10 };

	KClass *Choices[16];

	int NumItems;
	KMenuItem_t *CurrentItem;

	int ChoiceStartX;
	int ChoiceStartY;
	int	itemHeight;

	bool bUseSelector;

	FButtonDefault ButtonDefaults[MAX_BUTTONS];
	KMenuUIButton *WinButtons[MAX_BUTTONS];

	//	Constructor
	KMenuScreen(void)
	{
		Width = 640;
		Height = 480;
		itemHeight = ITEM_HEIGHT;
		Font = KCanvas::BigFont;
		bUseSelector = true;
	}

	void InitWindow(void)
	{
		guard(KMenuScreen::InitWindow);
		Super::InitWindow();
		CreateChoices();
		CreateButtons();
		unguard;
	}

	virtual void CreateChoices(void)
	{
		int i;
		KMenuItem_t *NewChoice;

		for (i = 0; i < 16; i++)
		{
			if (Choices[i])
			{
				NewChoice = (KMenuItem_t *)StaticCreateWindow(Choices[i], this);
				NewChoice->SetPos(ChoiceStartX, ChoiceStartY + i * itemHeight);
				NumItems++;
			}
		}
	}

	virtual void CreateButtons(void)
	{
		int ButtonIndex;

		for (ButtonIndex = 0; ButtonIndex < MAX_BUTTONS; ButtonIndex++)
		{
			if (ButtonDefaults[ButtonIndex].Name)
			{
				WinButtons[ButtonIndex] = NewWindow(KMenuUIButton, this);

				WinButtons[ButtonIndex]->SetButtonText(ButtonDefaults[ButtonIndex].Name);
				WinButtons[ButtonIndex]->SetPos(ChoiceStartX, 
					ChoiceStartY + NumItems * itemHeight);
				NumItems++;
			}
			else
			{
				break;
			}
		}
	}

	void FocusEnteredDescendant(KWindow *pEnterWindow)
	{
		KMenuItem_t *Item;

		Item = Cast<KMenuItem_t>(pEnterWindow);
		if (!Item)
		{
			Item = Cast<KMenuItem_t>(pEnterWindow->GetParent());
		}
		if (Item)
		{
			CurrentItem = Item;
		}
		Super::FocusEnteredDescendant(pEnterWindow);
	}

	void FocusLeftDescendant(KWindow *pLeftWindow)
	{
		CurrentItem = NULL;
		Super::FocusLeftDescendant(pLeftWindow);
	}

	void PostDrawWindow(KGC *gc)
	{
		if (bUseSelector && CurrentItem)
		{
			gc->DrawIcon(CurrentItem->X + SELECTOR_XOFFSET,
				CurrentItem->Y + SELECTOR_YOFFSET - (10 - itemHeight / 2),
				FindTexture(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2"));
		}
	}

	void CyclePrevChoice(void)
	{
		MoveFocusUp();
	}

	void CycleNextChoice(void)
	{
		MoveFocusDown();
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_DOWNARROW:
			CycleNextChoice();
			S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
			return true;

		case DDKEY_UPARROW:
			CyclePrevChoice();
			S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
			return true;

		case DDKEY_ESCAPE:
			PopMenu();
			return true;
		}
		return Super::KeyPressed(key);
	}

	bool ButtonActivated(KWindow *ButtonPressed)
	{
		bool bHandled;
		int ButtonIndex;

		bHandled = false;

		Super::ButtonActivated(ButtonPressed);

		// Figure out which button was pressed
		for (ButtonIndex = 0; ButtonIndex < MAX_BUTTONS; ButtonIndex++)
		{
			if (ButtonPressed == WinButtons[ButtonIndex])
			{
				// Check to see if there's somewhere to go
				ProcessMenuAction(ButtonDefaults[ButtonIndex].Action, 
					ButtonDefaults[ButtonIndex].Invoke, 
					ButtonDefaults[ButtonIndex].Key);

				bHandled = true;
				break;
			}
		}

		return bHandled;
	}

	void ProcessMenuAction(int Action, MenuType_t Invoke, int Key)
	{
		switch (Action)
		{
		case MA_None:
			break;

		case MA_Menu:
			SetMenu(Invoke);
			break;

		case MA_MenuNC:
			if (SCNetCheck(Key))
			{
				SetMenu(Invoke);
			}
			break;

		case MA_EndGame:
			if (!demoplayback && SCNetCheck(3))
			{
				ForceMenuOff();
				askforquit = true;
				typeofask = 2; //endgame
			}
			break;

		case MA_Quit:
			SCQuitGame(0);
			break;

		case MA_Custom:
			ProcessCustomMenuAction(Key);
			break;
		}
	}

	virtual void ProcessCustomMenuAction(int Key)
	{
	}
};
IMPLEMENT_CLASS(KMenuScreen);

