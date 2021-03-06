//==========================================================================
//
//	KMenuChoice_JoySensitivity
//
//==========================================================================

class KMenuChoice_JoySensitivity:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_JoySensitivity, KMenuUIChoiceSlider, 0);

	KMenuChoice_JoySensitivity(void)
	{
		ActionText = "SENSITIVITY";
		NumTicks = 10;
		StartValue = 1;
		EndValue = 9;
		pInteger = &joySensitivity;
	}
};
IMPLEMENT_CLASS(KMenuChoice_JoySensitivity);

//==========================================================================
//
//	KMenuChoice_JoyLook
//
//==========================================================================

class KMenuChoice_JoyLook:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_JoyLook, KMenuChoice_OnOff, 0);

	KMenuChoice_JoyLook(void)
	{
		ActionText = "JOY LOOK";
		pInteger = &usejlook;
	}
};
IMPLEMENT_CLASS(KMenuChoice_JoyLook);

//==========================================================================
//
//	KMenuChoice_InverseJoyLook
//
//==========================================================================

class KMenuChoice_InverseJoyLook:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_InverseJoyLook, KMenuChoice_OnOff, 0);

	KMenuChoice_InverseJoyLook(void)
	{
		ActionText = "INVERSE LOOK";
		pInteger = &jlookInverseY;
	}
};
IMPLEMENT_CLASS(KMenuChoice_InverseJoyLook);

//==========================================================================
//
//	KMenuChoice_POVLook
//
//==========================================================================

class KMenuChoice_POVLook:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_POVLook, KMenuChoice_OnOff, 0);

	KMenuChoice_POVLook(void)
	{
		ActionText = "POV LOOK";
		pInteger = &povLookAround;
	}
};
IMPLEMENT_CLASS(KMenuChoice_POVLook);

//==========================================================================
//
//	KMenuScreenJoyConfig
//
//==========================================================================

class KMenuScreenJoyConfig:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenJoyConfig, KMenuScreen, 0);

	KMenuScreenJoyConfig(void)
	{
		ChoiceStartX = 240;
		ChoiceStartY = 190;
		Choices[0] = KMenuChoice_JoySensitivity::StaticClass();
		Choices[1] = KMenuChoice_JoyLook::StaticClass();
		Choices[2] = KMenuChoice_InverseJoyLook::StaticClass();
		Choices[3] = KMenuChoice_POVLook::StaticClass();
	}
};
IMPLEMENT_CLASS(KMenuScreenJoyConfig);
