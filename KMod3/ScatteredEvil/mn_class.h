//==========================================================================
//
//	KMenuScreenClass
//
//==========================================================================

class KMenuScreenClass:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenClass, KMenuScreen, 0);

	KMenuScreenClass(void)
	{
		ChoiceStartX = 226;
		ChoiceStartY = 206;
		ButtonDefaults[0] = FButtonDefault("FIGHTER", MA_Custom, MENU_NONE, 0);
		ButtonDefaults[1] = FButtonDefault("CLERIC", MA_Custom, MENU_NONE, 1);
		ButtonDefaults[2] = FButtonDefault("MAGE", MA_Custom, MENU_NONE, 2);
	}

	void ProcessCustomMenuAction(int Key)
	{
		if (netgame)
		{
			P_SetMessage(&players[consoleplayer],
				"YOU CAN'T START A NEW GAME FROM WITHIN A NETGAME!", true);
			return;
		}
		MenuPClass = Key;
		SetMenu(MENU_SKILL);
	}

	void DrawWindow(KGC *gc)
	{
		static char *boxLumpName[4] =
		{
			"m_fbox",
			"m_cbox",
			"m_mbox",
			"m_cbox"
		};
		static char *walkLumpName[3] =
		{
			"m_fwalk1",
			"m_cwalk1",
			"m_mwalk1"
		};

		gc->DrawText(194, 164, "CHOOSE CLASS:");
		int pclass = 0;
		if (CurrentItem == WinButtons[1])
			pclass = 1;
		else if (CurrentItem == WinButtons[2])
			pclass = 2;
		gc->DrawIcon(334, 148, FindTexture(boxLumpName[pclass]));
		gc->GetCanvas()->DrawPatch(334+24, 148+12, 
			W_GetNumForName(walkLumpName[pclass])+((MenuTime>>3)&3));
	}
};
IMPLEMENT_CLASS(KMenuScreenClass);

