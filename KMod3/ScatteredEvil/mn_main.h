//==========================================================================
//
//	KMenuScreenMain
//
//==========================================================================

class KMenuScreenMain:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenMain, KMenuScreen, 0);

	int MauloBaseLump;

	KMenuScreenMain(void)
	{
		ChoiceStartX = 270;
		ChoiceStartY = 196;
		ButtonDefaults[0] = FButtonDefault("NEW GAME", MA_MenuNC, MENU_CLASS, 1);
		ButtonDefaults[1] = FButtonDefault("LOAD GAME", MA_MenuNC, MENU_LOAD, 2);
		ButtonDefaults[2] = FButtonDefault("SAVE GAME", MA_Menu, MENU_SAVE);
		ButtonDefaults[3] = FButtonDefault("OPTIONS", MA_Menu, MENU_OPTIONS);
		ButtonDefaults[4] = FButtonDefault("INFO", MA_Menu, MENU_INFO);
		ButtonDefaults[5] = FButtonDefault("QUIT GAME", MA_Quit);
		MauloBaseLump = W_GetNumForName("FBULA0");
	}

	void DrawWindow(KGC *gc)
	{
		int frame;

		frame = (MenuTime / 5) % 7;
	
		gc->DrawIcon(248, 140, FindTexture("M_HTIC"));

		gc->GetCanvas()->DrawPatch(197, 220, MauloBaseLump + (frame + 2) % 7);
		gc->GetCanvas()->DrawPatch(438, 220, MauloBaseLump + frame);
	}
};
IMPLEMENT_CLASS(KMenuScreenMain);
