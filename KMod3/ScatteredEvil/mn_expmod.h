//==========================================================================
//
//	KMenuScreenExpMod
//
//==========================================================================

class KMenuScreenExpMod:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenExpMod, KMenuScreen, 0);

	KMenuScreenExpMod(void)
	{
		ChoiceStartX = 280;
		ChoiceStartY = 184;
		ButtonDefaults[0] = FButtonDefault("50%", MA_Custom, MENU_NONE, 50);
		ButtonDefaults[1] = FButtonDefault("100%", MA_Custom, MENU_NONE, 100);
		ButtonDefaults[2] = FButtonDefault("125%", MA_Custom, MENU_NONE, 125);
		ButtonDefaults[3] = FButtonDefault("150%", MA_Custom, MENU_NONE, 150);
		ButtonDefaults[4] = FButtonDefault("200%", MA_Custom, MENU_NONE, 200);
	}

	void ProcessCustomMenuAction(int Key)
	{
		MenuExpMod = Key;
		SetMenu(MENU_CHAR);
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(234, 156, "CHOOSE EXPERIENCE MODIFIER:");
	}
};
IMPLEMENT_CLASS(KMenuScreenExpMod);