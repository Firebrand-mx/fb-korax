//==========================================================================
//
//	KMenuScreenOptions
//
//==========================================================================

class KMenuScreenOptions:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenOptions, KMenuScreen, 0);

	KMenuScreenOptions(void)
	{
		ChoiceStartX = 270;
		ChoiceStartY = 180;
		ButtonDefaults[0] = FButtonDefault("END GAME", MA_EndGame);
		ButtonDefaults[1] = FButtonDefault("GAMEPLAY...", MA_Menu, MENU_GAMEPLAY);
		ButtonDefaults[2] = FButtonDefault("GRAPHICS...", MA_Menu, MENU_GRAPHICS);
		ButtonDefaults[3] = FButtonDefault("SOUND...", MA_Menu, MENU_SOUND);
		ButtonDefaults[4] = FButtonDefault("CONTROLS...", MA_Menu, MENU_CONTROLS);
		ButtonDefaults[5] = FButtonDefault("MOUSE OPTIONS...", MA_Menu, MENU_MOUSEOPTS);
		ButtonDefaults[6] = FButtonDefault("JOYSTICK OPTIONS...", MA_Menu, MENU_JOYCONFIG);
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(314-Font->TextWidth("OPTIONS")/2, 150, "OPTIONS");
	}
};
IMPLEMENT_CLASS(KMenuScreenOptions);
