//==========================================================================
//
//	KMenuScreenSkill
//
//==========================================================================

class KMenuScreenSkill:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenSkill, KMenuScreen, 0);

	KMenuScreenSkill(void)
	{
		ChoiceStartX = 280;
		ChoiceStartY = 184;
		switch(MenuPClass)
		{
		case PCLASS_FIGHTER:
			ChoiceStartX = 280;
			ButtonDefaults[0] = FButtonDefault("SQUIRE", MA_Custom, MENU_NONE, sk_baby);
			ButtonDefaults[1] = FButtonDefault("KNIGHT", MA_Custom, MENU_NONE, sk_easy);
			ButtonDefaults[2] = FButtonDefault("WARRIOR", MA_Custom, MENU_NONE, sk_medium);
			ButtonDefaults[3] = FButtonDefault("BERSERKER", MA_Custom, MENU_NONE, sk_hard);
			ButtonDefaults[4] = FButtonDefault("TITAN", MA_Custom, MENU_NONE, sk_nightmare);
			break;
		case PCLASS_CLERIC:
			ChoiceStartX = 276;
			ButtonDefaults[0] = FButtonDefault("ALTAR BOY", MA_Custom, MENU_NONE, sk_baby);
			ButtonDefaults[1] = FButtonDefault("ACOLYTE", MA_Custom, MENU_NONE, sk_easy);
			ButtonDefaults[2] = FButtonDefault("PRIEST", MA_Custom, MENU_NONE, sk_medium);
			ButtonDefaults[3] = FButtonDefault("CARDINAL", MA_Custom, MENU_NONE, sk_hard);
			ButtonDefaults[4] = FButtonDefault("POPE", MA_Custom, MENU_NONE, sk_nightmare);
			break;
		case PCLASS_MAGE:
			ChoiceStartX = 272;
			ButtonDefaults[0] = FButtonDefault("APPRENTICE", MA_Custom, MENU_NONE, sk_baby);
			ButtonDefaults[1] = FButtonDefault("ENCHANTER", MA_Custom, MENU_NONE, sk_easy);
			ButtonDefaults[2] = FButtonDefault("SORCERER", MA_Custom, MENU_NONE, sk_medium);
			ButtonDefaults[3] = FButtonDefault("WARLOCK", MA_Custom, MENU_NONE, sk_hard);
			ButtonDefaults[4] = FButtonDefault("ARCHIMAGE", MA_Custom, MENU_NONE, sk_nightmare);
			break;
		case PCLASS_CORVUS:
			ChoiceStartX = 272;
			ButtonDefaults[0] = FButtonDefault("BABY", MA_Custom, MENU_NONE, sk_baby);
			ButtonDefaults[1] = FButtonDefault("EASY", MA_Custom, MENU_NONE, sk_easy);
			ButtonDefaults[2] = FButtonDefault("NORMAL", MA_Custom, MENU_NONE, sk_medium);
			ButtonDefaults[3] = FButtonDefault("HARD", MA_Custom, MENU_NONE, sk_hard);
			ButtonDefaults[4] = FButtonDefault("NIGHTMARE", MA_Custom, MENU_NONE, sk_nightmare);
			break;
		}
	}

	void InitWindow(void)
	{
		Super::InitWindow();
		CycleNextChoice();
		CycleNextChoice();
	}

	void ProcessCustomMenuAction(int Key)
	{
		MenuPValues = Reroll(MenuPClass);
		MenuPSkill = Key;
		SetMenu(MENU_CHAR);
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(234, 156, "CHOOSE SKILL LEVEL:");
	}
};
IMPLEMENT_CLASS(KMenuScreenSkill);
