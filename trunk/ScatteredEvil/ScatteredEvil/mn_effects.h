//==========================================================================
//
//	KMenuChoice_FPSCounter
//
//==========================================================================

class KMenuChoice_FPSCounter:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_FPSCounter, KMenuChoice_OnOff, 0);

	KMenuChoice_FPSCounter(void)
	{
		ActionText = "FPS COUNTER";
		pInteger = &translucentIceCorpse;
	}
};
IMPLEMENT_CLASS(KMenuChoice_FPSCounter);

//==========================================================================
//
//	KMenuChoice_FrozenThings
//
//==========================================================================

class KMenuChoice_FrozenThings:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_FrozenThings, KMenuUIChoiceEnum, 0);

	KMenuChoice_FrozenThings(void)
	{
		ActionText = "FROZEN THINGS";
		EnumText[0] = "SOLID";
		EnumText[1] = "TRANSLUCENT";
		pInteger = &translucentIceCorpse;
	}
};
IMPLEMENT_CLASS(KMenuChoice_FrozenThings);

//==========================================================================
//
//	KMenuChoice_DynLights
//
//==========================================================================

class KMenuChoice_DynLights:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_DynLights, KMenuChoice_OnOff, 0);

	KMenuChoice_DynLights(void)
	{
		ActionText = "DYNAMIC LIGHTS";
		CVarName = "dynlights";
	}
};
IMPLEMENT_CLASS(KMenuChoice_DynLights);

//==========================================================================
//
//	KMenuChoice_DynLightBlend
//
//==========================================================================

class KMenuChoice_DynLightBlend:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_DynLightBlend, KMenuUIChoiceEnum, 0);

	KMenuChoice_DynLightBlend(void)
	{
		ActionText = "DYNLIGHT BLENDING";
		EnumText[0] = "MULTIPLY";
		EnumText[1] = "ADD";
		EnumText[2] = "NONE";
		EnumText[3] = "DON'T RENDER";
		CVarName = "dlblend";
	}
};
IMPLEMENT_CLASS(KMenuChoice_DynLightBlend);

//==========================================================================
//
//	KMenuChoice_LightsOnSprites
//
//==========================================================================

class KMenuChoice_LightsOnSprites:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_LightsOnSprites, KMenuChoice_OnOff, 0);

	KMenuChoice_LightsOnSprites(void)
	{
		ActionText = "LIGHTS ON SPRITES";
		CVarName = "sprlight";
	}
};
IMPLEMENT_CLASS(KMenuChoice_LightsOnSprites);

//==========================================================================
//
//	KMenuChoice_DynLightIntensity
//
//==========================================================================

class KMenuChoice_DynLightIntensity:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_DynLightIntensity, KMenuUIChoiceSlider, 0);

	KMenuChoice_DynLightIntensity(void)
	{
		ActionText = "DYNLIGHT INTENSITY";
		CVarName = "dlfactor";
	}
};
IMPLEMENT_CLASS(KMenuChoice_DynLightIntensity);

//==========================================================================
//
//	KMenuChoice_LensFlares
//
//==========================================================================

class KMenuChoice_LensFlares:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_LensFlares, KMenuUIChoiceEnum, 0);

	KMenuChoice_LensFlares(void)
	{
		ActionText = "LENS FLARES";
		EnumText[0] = "OFF";
		EnumText[1] = "1";
		EnumText[2] = "2";
		EnumText[3] = "3";
		EnumText[4] = "4";
		EnumText[5] = "5";
		CVarName = "flares";
	}
};
IMPLEMENT_CLASS(KMenuChoice_LensFlares);

//==========================================================================
//
//	KMenuChoice_FlareIntensity
//
//==========================================================================

class KMenuChoice_FlareIntensity:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_FlareIntensity, KMenuUIChoiceSlider, 0);

	KMenuChoice_FlareIntensity(void)
	{
		ActionText = "FLARE INTENSITY";
		CVarName = "flareintensity";
	}
};
IMPLEMENT_CLASS(KMenuChoice_FlareIntensity);

//==========================================================================
//
//	KMenuChoice_FlareSize
//
//==========================================================================

class KMenuChoice_FlareSize:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_FlareSize, KMenuUIChoiceSlider, 0);

	KMenuChoice_FlareSize(void)
	{
		ActionText = "FLARE SIZE";
		CVarName = "flaresize";
	}
};
IMPLEMENT_CLASS(KMenuChoice_FlareSize);

//==========================================================================
//
//	KMenuChoice_SprAlign
//
//==========================================================================

class KMenuChoice_SprAlign:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_SprAlign, KMenuUIChoiceEnum, 0);

	KMenuChoice_SprAlign(void)
	{
		ActionText = "ALIGN SPRITES TO";
		EnumText[0] = "CAMERA";
		EnumText[1] = "VIEW PLANE";
		EnumText[2] = "CAMERA (R)";
		EnumText[3] = "VIEW PLANE (R)";
		CVarName = "spralign";
	}
};
IMPLEMENT_CLASS(KMenuChoice_SprAlign);

//==========================================================================
//
//	KMenuChoice_SprBlend
//
//==========================================================================

class KMenuChoice_SprBlend:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_SprBlend, KMenuChoice_OnOff, 0);

	KMenuChoice_SprBlend(void)
	{
		ActionText = "SPRITE BLENDING";
		CVarName = "sprblend";
	}
};
IMPLEMENT_CLASS(KMenuChoice_SprBlend);

//==========================================================================
//
//	KMenuScreenEffects
//
//==========================================================================

class KMenuScreenEffects:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenEffects, KMenuScreen, 0);

	KMenuScreenEffects(void)
	{
		ChoiceStartX = 220;
		ChoiceStartY = 155;
		Choices[0] = KMenuChoice_FPSCounter::StaticClass();
		Choices[1] = KMenuChoice_FrozenThings::StaticClass();
		Choices[2] = KMenuChoice_DynLights::StaticClass();
		Choices[3] = KMenuChoice_DynLightBlend::StaticClass();
		Choices[4] = KMenuChoice_LightsOnSprites::StaticClass();
		Choices[5] = KMenuChoice_DynLightIntensity::StaticClass();
		Choices[6] = KMenuChoice_LensFlares::StaticClass();
		Choices[7] = KMenuChoice_FlareIntensity::StaticClass();
		Choices[8] = KMenuChoice_FlareSize::StaticClass();
		Choices[9] = KMenuChoice_SprAlign::StaticClass();
		Choices[10] = KMenuChoice_SprBlend::StaticClass();
	}
};
IMPLEMENT_CLASS(KMenuScreenEffects);
