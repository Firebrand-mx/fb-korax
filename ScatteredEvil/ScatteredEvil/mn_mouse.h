//==========================================================================
//
//	KMenuChoice_MouseInverseY
//
//==========================================================================

class KMenuChoice_MouseInverseY:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_MouseInverseY, KMenuUIChoiceEnum, 0);

	KMenuChoice_MouseInverseY(void)
	{
		ActionText = "INVERSE Y";
	}

	void LoadSetting(void)
	{
		SetValue(gi.Get(DD_MOUSE_INVERSE_Y));
	}

	void SaveSetting(void)
	{
		gi.Set(DD_MOUSE_INVERSE_Y, GetValue());
	}
};
IMPLEMENT_CLASS(KMenuChoice_MouseInverseY);

//==========================================================================
//
//	KMenuChoice_MouseLook
//
//==========================================================================

class KMenuChoice_MouseLook:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_MouseLook, KMenuUIChoiceEnum, 0);

	KMenuChoice_MouseLook(void)
	{
		ActionText = "MOUSE LOOK";
		pInteger = &usemlook;
	}
};
IMPLEMENT_CLASS(KMenuChoice_MouseLook);

//==========================================================================
//
//	KMenuChoice_InverseMLook
//
//==========================================================================

class KMenuChoice_InverseMLook:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_InverseMLook, KMenuUIChoiceEnum, 0);

	KMenuChoice_InverseMLook(void)
	{
		ActionText = "INVERSE MLOOK";
		pInteger = &mlookInverseY;
	}
};
IMPLEMENT_CLASS(KMenuChoice_InverseMLook);

//==========================================================================
//
//	KMenuChoice_MouseXSensitivity
//
//==========================================================================

class KMenuChoice_MouseXSensitivity:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_MouseXSensitivity, KMenuUIChoiceSlider, 0);

	KMenuChoice_MouseXSensitivity(void)
	{
		ActionText = "X SENSITIVITY";
		NumTicks = 18;
		EndValue = 17;
		pInteger = &mouseSensitivityX;
	}
};
IMPLEMENT_CLASS(KMenuChoice_MouseXSensitivity);

//==========================================================================
//
//	KMenuChoice_MouseYSensitivity
//
//==========================================================================

class KMenuChoice_MouseYSensitivity:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_MouseYSensitivity, KMenuUIChoiceSlider, 0);

	KMenuChoice_MouseYSensitivity(void)
	{
		ActionText = "Y SENSITIVITY";
		NumTicks = 18;
		EndValue = 17;
		pInteger = &mouseSensitivityY;
	}
};
IMPLEMENT_CLASS(KMenuChoice_MouseYSensitivity);

//==========================================================================
//
//	KMenuScreenMouseOptions
//
//==========================================================================

class KMenuScreenMouseOptions:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenMouseOptions, KMenuScreen, 0);

	KMenuScreenMouseOptions(void)
	{
		ChoiceStartX = 232;
		ChoiceStartY = 170;
	}

	void CreateChoices()
	{
		KMenuUIChoice *Choice;

		Choice = NewWindow(KMenuChoice_MouseInverseY, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_MouseLook, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_InverseMLook, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_MouseXSensitivity, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_MouseYSensitivity, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;
	}
};
IMPLEMENT_CLASS(KMenuScreenMouseOptions);
