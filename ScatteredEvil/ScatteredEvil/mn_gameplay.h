//==========================================================================
//
//	KMenuChoice_Messages
//
//==========================================================================

class KMenuChoice_Messages:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_Messages, KMenuChoice_OnOff, 0);

	KMenuChoice_Messages(void)
	{
		ActionText = "MESSAGES";
		pInteger = &messageson;
	}
};
IMPLEMENT_CLASS(KMenuChoice_Messages);

//==========================================================================
//
//	KMenuChoice_AllwaysRun
//
//==========================================================================

class KMenuChoice_AllwaysRun:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_AllwaysRun, KMenuChoice_OnOff, 0);

	KMenuChoice_AllwaysRun(void)
	{
		ActionText = "ALWAYS RUN";
		pInteger = &alwaysRun;
	}
};
IMPLEMENT_CLASS(KMenuChoice_AllwaysRun);

//==========================================================================
//
//	KMenuChoice_Lookspring
//
//==========================================================================

class KMenuChoice_Lookspring:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_Lookspring, KMenuChoice_OnOff, 0);

	KMenuChoice_Lookspring(void)
	{
		ActionText = "LOOKSPRING";
		pInteger = &lookSpring;
	}
};
IMPLEMENT_CLASS(KMenuChoice_Lookspring);

//==========================================================================
//
//	KMenuChoice_AutoAim
//
//==========================================================================

class KMenuChoice_AutoAim:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_AutoAim, KMenuChoice_OnOff, 0);

	KMenuChoice_AutoAim(void)
	{
		ActionText = "NO AUTOAIM";
		pInteger = &noAutoAim;
	}
};
IMPLEMENT_CLASS(KMenuChoice_AutoAim);

//==========================================================================
//
//	KMenuChoice_Crosshair
//
//==========================================================================

class KMenuChoice_Crosshair:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_Crosshair, KMenuUIChoiceEnum, 0);

	KMenuChoice_Crosshair(void)
	{
		ActionText = "CROSSHAIR";
		EnumText[0] = "NONE";
		EnumText[1] = "CROSS";
		EnumText[2] = "ANGLES";
		EnumText[3] = "SQUARE";
		EnumText[4] = "OPEN SQUARE";
		EnumText[5] = "DIAMOND";
		EnumText[6] = "V";
		pInteger = &xhair;
	}
};
IMPLEMENT_CLASS(KMenuChoice_Crosshair);

//==========================================================================
//
//	KMenuChoice_CrosshairSize
//
//==========================================================================

class KMenuChoice_CrosshairSize:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_CrosshairSize, KMenuUIChoiceSlider, 0);

	KMenuChoice_CrosshairSize(void)
	{
		ActionText = "CROSSHAIR SIZE";
		NumTicks = 10;
		EndValue = 9;
		pInteger = &xhairSize;
	}
};
IMPLEMENT_CLASS(KMenuChoice_CrosshairSize);

//==========================================================================
//
//	KMenuChoice_ScreenSize
//
//==========================================================================

class KMenuChoice_ScreenSize:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_ScreenSize, KMenuUIChoiceSlider, 0);

	KMenuChoice_ScreenSize(void)
	{
		ActionText = "SCREEN SIZE";
		NumTicks = 9;
		StartValue = 3;
		EndValue = 11;
		pInteger = &screenblocks;
	}

	void SaveSetting(void)
	{
		screenblocks = int(GetValue());
		R_SetViewSize(screenblocks);
	}
};
IMPLEMENT_CLASS(KMenuChoice_ScreenSize);

//==========================================================================
//
//	KMenuScreenGameplay
//
//==========================================================================

class KMenuScreenGameplay:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenGameplay, KMenuScreen, 0);

	KMenuScreenGameplay(void)
	{
		ChoiceStartX = 224;
		ChoiceStartY = 165;
		Choices[0] = KMenuChoice_Messages::StaticClass();
		Choices[1] = KMenuChoice_AllwaysRun::StaticClass();
		Choices[2] = KMenuChoice_Lookspring::StaticClass();
		Choices[3] = KMenuChoice_AutoAim::StaticClass();
		Choices[4] = KMenuChoice_Crosshair::StaticClass();
		Choices[5] = KMenuChoice_CrosshairSize::StaticClass();
		Choices[6] = KMenuChoice_ScreenSize::StaticClass();
	}
};
IMPLEMENT_CLASS(KMenuScreenGameplay);
