struct MenuRes_t
{
	int	width;
	int height;
};

static MenuRes_t resolutions[] =
{
	320, 240,
	640, 480,
	800, 600,
	1024, 768,
	1152, 864,
	1280, 1024,
	1600, 1200,
	0, 0	// The terminator.
};

static int selRes;

//==========================================================================
//
//	KMenuChoice_Resolution
//
//==========================================================================

class KMenuChoice_Resolution:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_Resolution, KMenuUIChoiceEnum, 0);

	KMenuChoice_Resolution(void)
	{
		ActionText = "RESOLUTION";
		EnumText[0] = "320 x 240";
		EnumText[1] = "640 x 480";
		EnumText[2] = "800 x 600";
		EnumText[3] = "1024 x 768";
		EnumText[4] = "1152 x 864";
		EnumText[5] = "1280 x 1024";
		EnumText[6] = "1600 x 1200";
	}

	void LoadSetting(void)
	{
		// Find the correct resolution.
		selRes = findRes(screenWidth, screenHeight);
		SetValue(selRes);
	}

	void SaveSetting(void)
	{
		selRes = GetValue();
	}

	void DrawWindow(KGC *gc)
	{
		Super::DrawWindow(gc);
		if (selRes == findRes(defResX, defResY))
		{
			gc->DrawText(240, 0, " (DEFAULT)");
		}
	}

	int findRes(int w, int h)
	{
		int i;

		for (i = 0; resolutions[i].width; i++)
			if (resolutions[i].width == w && resolutions[i].height == h)
				return i;
		return -1;
	}
};
IMPLEMENT_CLASS(KMenuChoice_Resolution);

//==========================================================================
//
//	KMenuScreenResolution
//
//==========================================================================

class KMenuScreenResolution:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenResolution, KMenuScreen, 0);

	KMenuScreenResolution(void)
	{
		ChoiceStartX = 248;
		ChoiceStartY = 200;
		Choices[0] = KMenuChoice_Resolution::StaticClass();
		ButtonDefaults[0] = FButtonDefault("MAKE CURRENT", MA_Custom, MENU_NONE, 1);
		ButtonDefaults[1] = FButtonDefault("MAKE DEFAULT", MA_Custom, MENU_NONE, 2);
	}

	void ProcessCustomMenuAction(int Key)
	{
		switch (Key)
		{
		case 1:
			if (I_ChangeResolution(resolutions[selRes].width, resolutions[selRes].height))
				P_SetMessage(&players[consoleplayer], "RESOLUTION CHANGED", true);
			else
				P_SetMessage(&players[consoleplayer], "RESOLUTION CHANGE FAILED", true);
			break;

		case 2:
			defResX = resolutions[selRes].width;
			defResY = resolutions[selRes].height;
			break;
		}
	}
};
IMPLEMENT_CLASS(KMenuScreenResolution);
