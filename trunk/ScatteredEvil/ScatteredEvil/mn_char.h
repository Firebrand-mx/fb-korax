//==========================================================================
//
//	KMenuScreenChar
//
//==========================================================================

class KMenuScreenChar:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenChar, KMenuScreen, 0);

	KMenuScreenChar(void)
	{
		ChoiceStartX = 280;
		ChoiceStartY = 270;
		CursorPos = 2;
		ButtonDefaults[0] = FButtonDefault("REROLL", MA_Custom, MENU_NONE, 0);
		ButtonDefaults[1] = FButtonDefault("DEFAULT", MA_Custom, MENU_NONE, 1);
		ButtonDefaults[2] = FButtonDefault("OK", MA_Custom, MENU_NONE, 2);
	}

	void ProcessCustomMenuAction(int Key)
	{
		extern int SB_state;
		if (Key == 0)
		{
			MenuPValues = Reroll(MenuPClass);
			return;
		}
		if (Key == 1)
		{
			MenuPValues = Defaultroll(MenuPClass);
			return;
		}

		PlayerClass[consoleplayer] = (pclass_t)MenuPClass;
		G_DeferredNewGame((skill_t)MenuPSkill);
		SB_SetClassData();
		SB_state = -1;
		MN_DeactivateMenu();
	}

	void DrawWindow(KGC *gc)
	{
		char tmp[2][5][13];
		int i;
		gc->DrawText(210, 156, "GENERATE YOUR CHARACTER:");
		sprintf(tmp[0][0],"HEALTH:");
		sprintf(tmp[0][1],"%s",sp_wording[MenuPClass]);
		sprintf(tmp[0][2],"STRENGTH:");
		sprintf(tmp[0][3],"EFFICIENCY:");
		sprintf(tmp[0][4],"SPEED:");
		sprintf(tmp[1][0],"%3d",MenuPValues.val[0]);
		sprintf(tmp[1][1],((MenuPClass!=3)?"%3d":""),MenuPValues.val[1]);
		sprintf(tmp[1][2],"%02d",MenuPValues.val[2]);
		sprintf(tmp[1][3],"%02d",MenuPValues.val[3]);
		sprintf(tmp[1][4],"%02d",MenuPValues.val[4]);
		for (i = 0; i < 5; i++)
		{
			gc->DrawText(234, 184 + i * 16, tmp[0][i]);
			gc->DrawText(354, 184 + i * 16, tmp[1][i]);
		}
	}
};
IMPLEMENT_CLASS(KMenuScreenChar);
