//==========================================================================
//
//	KMenuChoice_SkyDetail
//
//==========================================================================

class KMenuChoice_SkyDetail:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_SkyDetail, KMenuUIChoiceSlider, 0);

	KMenuChoice_SkyDetail(void)
	{
		ActionText = "SKY DETAIL";
		NumTicks = 5;
		StartValue = 3;
		EndValue = 7;
	}

	void LoadSetting(void)
	{
		SetValue(skyDetail);
	}

	void SaveSetting(void)
	{
		R_SkyParams(DD_SKY, DD_COLUMNS, int(GetValue()));
	}
};
IMPLEMENT_CLASS(KMenuChoice_SkyDetail);

//==========================================================================
//
//	KMenuChoice_Mipmapping
//
//==========================================================================

class KMenuChoice_Mipmapping:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_Mipmapping, KMenuUIChoiceEnum, 0);

	KMenuChoice_Mipmapping(void)
	{
		ActionText = "MIPMAPPING";
		EnumText[0] = "N";
		EnumText[1] = "L";
		EnumText[2] = "N, MIP N";
		EnumText[3] = "L, MIP N";
		EnumText[4] = "N, MIP L";
		EnumText[5] = "L, MIP L";
	}

	void LoadSetting(void)
	{
		SetValue(mipmapping);
	}

	void SaveSetting(void)
	{
		GL_TextureFilterMode(DD_TEXTURES, GetValue());
	}
};
IMPLEMENT_CLASS(KMenuChoice_Mipmapping);

//==========================================================================
//
//	KMenuChoice_SmoothGfx
//
//==========================================================================

class KMenuChoice_SmoothGfx:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_SmoothGfx, KMenuChoice_OnOff, 0);

	KMenuChoice_SmoothGfx(void)
	{
		ActionText = "SMOOTH GFX";
	}

	void LoadSetting(void)
	{
		SetValue(linearRaw);
	}

	void SaveSetting(void)
	{
		GL_TextureFilterMode(DD_RAWSCREENS, GetValue());
	}

};
IMPLEMENT_CLASS(KMenuChoice_SmoothGfx);

//==========================================================================
//
//	KMenuChoice_UpdateBorders
//
//==========================================================================

class KMenuChoice_UpdateBorders:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_UpdateBorders, KMenuChoice_OnOff, 0);

	KMenuChoice_UpdateBorders(void)
	{
		ActionText = "UPDATE BORDERS";
		CVarName = "borderupd";
	}
};
IMPLEMENT_CLASS(KMenuChoice_UpdateBorders);

//==========================================================================
//
//	KMenuChoice_TexQuality
//
//==========================================================================

class KMenuChoice_TexQuality:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_TexQuality, KMenuUIChoiceEnum, 0);

	KMenuChoice_TexQuality(void)
	{
		ActionText = "TEX QUALITY";
		EnumText[0] = "0 - MINIMUM";
		EnumText[1] = "1 - VERY LOW";
		EnumText[2] = "2 - LOW";
		EnumText[3] = "3 - POOR";
		EnumText[4] = "4 - AVERAGE";
		EnumText[5] = "5 - GOOD";
		EnumText[6] = "6 - HIGH";
		EnumText[7] = "7 - VERY HIGH";
		EnumText[8] = "8 - MAXIMUM";
		CVarName = "r_texquality";
	}
};
IMPLEMENT_CLASS(KMenuChoice_TexQuality);

//==========================================================================
//
//	KMenuScreenGraphics
//
//==========================================================================

class KMenuScreenGraphics:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenGraphics, KMenuScreen, 0);

	KMenuScreenGraphics(void)
	{
		ChoiceStartX = 218;
		ChoiceStartY = 150;
		Choices[0] = KMenuChoice_SkyDetail::StaticClass();
		Choices[1] = KMenuChoice_Mipmapping::StaticClass();
		Choices[2] = KMenuChoice_SmoothGfx::StaticClass();
		Choices[3] = KMenuChoice_UpdateBorders::StaticClass();
		Choices[4] = KMenuChoice_TexQuality::StaticClass();
		ButtonDefaults[0] = FButtonDefault("FORCE TEX RELOAD", MA_Custom);
		ButtonDefaults[1] = FButtonDefault("EFFECTS...", MA_Menu, MENU_EFFECTS);
		ButtonDefaults[2] = FButtonDefault("RESOLUTION...", MA_Menu, MENU_RESOLUTION);
	}

	void ProcessCustomMenuAction(int Key)
	{
		GL_ClearTextureMemory();
		P_SetMessage(&players[consoleplayer], "ALL TEXTURES DELETED", true);
	}
};
IMPLEMENT_CLASS(KMenuScreenGraphics);
