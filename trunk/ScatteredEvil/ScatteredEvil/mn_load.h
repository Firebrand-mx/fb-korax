//==========================================================================
//
//	KMenuLoadGameSlot
//
//==========================================================================

class KMenuLoadGameSlot:public KMenuUIFileSlot
{
	DECLARE_CLASS(KMenuLoadGameSlot, KMenuUIFileSlot, 0);

	KMenuLoadGameSlot(void)
	{
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_ENTER:
			SCLoadGame(SlotIndex);
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuLoadGameSlot);

//==========================================================================
//
//	KMenuScreenLoadGame
//
//==========================================================================

class KMenuScreenLoadGame:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenLoadGame, KMenuScreen, 0);

	KMenuScreenLoadGame(void)
	{
		ChoiceStartX = 230;
		ChoiceStartY = 170;
	}

	void CreateChoices()
	{
		for (int i = 0; i < 6; i++)
		{
			KMenuLoadGameSlot *It = NewWindow(KMenuLoadGameSlot, this);
			It->PrepareSlot(i);
			It->SetPos(ChoiceStartX, ChoiceStartY + i * itemHeight);
		}
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(320 - Font->TextWidth("LOAD GAME")/2, 150, "LOAD GAME");
	}
};
IMPLEMENT_CLASS(KMenuScreenLoadGame);

