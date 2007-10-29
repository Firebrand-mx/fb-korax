//==========================================================================
//
//	KSpellImageDisplayWindow
//
//==========================================================================

class KSpellImageDisplayWindow:public KWindow
{
	DECLARE_CLASS(KSpellImageDisplayWindow, KWindow, 0);

	KTexture *Texture;
	int PicX;
	int PicY;

	KSpellImageDisplayWindow(void)
	{
	}

	void InitWindow(void)
	{
		Super::InitWindow();
		Disable();
	}

	void DrawWindow(KGC *gc)
	{
		if (Texture)
		{
			gc->DrawIcon(PicX, PicY, Texture);
		}
	}

	void SetTexture(KTexture *NewTexture)
	{
		Texture = NewTexture;
	}

	void SetPicPos(int NewX, int NewY)
	{
		PicX = NewX;
		PicY = NewY;
	}
};
IMPLEMENT_CLASS(KSpellImageDisplayWindow);

//==========================================================================
//
//	KSpellDescriptionWindow
//
//==========================================================================

class KSpellDescriptionWindow:public KWindow
{
	DECLARE_CLASS(KSpellDescriptionWindow, KWindow, 0);

	char DescStrings[16][64];

	KSpellDescriptionWindow(void)
	{
		Font = KCanvas::SmallFont;
	}

	void InitWindow(void)
	{
		Super::InitWindow();
		Disable();
	}

	void DrawWindow(KGC *gc)
	{
		for (int i = 0; i < 16; i++)
			gc->DrawText(0, i * 13, DescStrings[i]);
	}

	void SetText(char *NewText)
	{
		memset(DescStrings, 0, sizeof(DescStrings));
		int CurLine = 0;
		char *pSrc = NewText;
		char *pDst = DescStrings[0];
		while (*pSrc)
		{
			if (*pSrc == '\n')
			{
				CurLine++;
				pDst = DescStrings[CurLine];
			}
			else
			{
			*pDst = *pSrc;
			pDst++;
		}
		pSrc++;
	}
}
};
IMPLEMENT_CLASS(KSpellDescriptionWindow);

//==========================================================================
//
//	KSpellPageBaseWindow
//
//==========================================================================

class KSpellPageBaseWindow:public KWindow
{
	DECLARE_CLASS(KSpellPageBaseWindow, KWindow, 0);

	KSpellPageBaseWindow(void)
	{
		X = 220;
		Y = 40;
		Width = 640;
		Height = 400;
		bIsSelectable = true;
	}
};
IMPLEMENT_CLASS(KSpellPageBaseWindow);

//==========================================================================
//
//	KSpellPage
//
//==========================================================================

class KSpellPage:public KSpellPageBaseWindow
{
	DECLARE_CLASS(KSpellPage, KSpellPageBaseWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellPage);

	struct FSpellPage
	{
		char *ImageName;
		char *Description;
	};

	KSpellImageDisplayWindow *winIcon;
	KSpellDescriptionWindow *winDesc;

	int ActivePage;
	int NumPages;
	FSpellPage Pages[12];

	void InitWindow(void)
	{
		Super::InitWindow();
		CreateIconDisplay();
		CreateDescWindow();
		CreatePages();
		SetActivePage();
	}

	void CreateIconDisplay(void)
	{
		winIcon = NewWindow(KSpellImageDisplayWindow, this);
		winIcon->SetPos(156, 32);
		winIcon->SetSize(110, 120);
		winIcon->SetPicPos(75, 75);
	}

	void CreateDescWindow(void)
	{
		winDesc = NewWindow(KSpellDescriptionWindow, this);
		winDesc->SetPos(84, 208);
		winDesc->SetSize(640, 80);
	}

	void AddPage(char *ImageName, char *Description)
	{
		Pages[NumPages].ImageName = ImageName;
		Pages[NumPages].Description = Description;
		NumPages++;
	}

	virtual void CreatePages(void)
	{

	}

	void SetActivePage(void)
	{
		player_t *player;
		player = &players[consoleplayer];

		if (player->pclass < 3)
		{
			winIcon->SetTexture(FindTexture(Pages[player->pclass].ImageName));
			winDesc->SetText(Pages[player->pclass].Description);
		}
		else
		{
			winIcon->SetTexture(FindTexture(Pages[2].ImageName));
			winDesc->SetText(Pages[2].Description);
		}
	}
};
IMPLEMENT_CLASS(KSpellPage);

//==========================================================================
//
//	KSpellOnePage
//
//==========================================================================

class KSpellOnePage:public KSpellPage
{
	DECLARE_CLASS(KSpellOnePage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellOnePage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "THIS ANCIENT AND RARELY USED SPELL OF THE LEGION\n"
				"CAUSES THE USER TO EXPLODE INTO A FIT OF RAGE\n"
				"DOUBLING STRENGTH, SPEED AND STAMINA.");
		AddPage("SPLC1", "A SIMPLE HEALING SPELL TAUGHT TO VIRTUALLY EVERY\n"
				"MEMBER OF THE CHURCH, CLENSES AND HEALS WOUNDS\n"
				"AT LITTLE SPIRITUAL COST TO THE CASTER.");
		AddPage("SPLM1", "A BASIC DEFENCIVE SPELL, CAUSES THE CASTER TO\n"
				"PROJECT A STRONG YET HARMLESS IMPULSE IN THE\n"
				"DIRECTION HE IS FACEING, CAUSEING ENEMIES IN\n"
				"THE WAY TO BE REPELLED.");
	}
};
IMPLEMENT_CLASS(KSpellOnePage);

//==========================================================================
//
//	KSpellTwoPage
//
//==========================================================================

class KSpellTwoPage:public KSpellPage
{
	DECLARE_CLASS(KSpellTwoPage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellTwoPage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "");
		AddPage("SPLC2", "A POTENT SPELL THAT ALLOWS THE CASTER TO\n"
				"USE THE SOULS OF HIS FALLEN ENEMIES TO ASSIST\n"
				"HIM IN THE COMMING ONSLAUGHT.");
		AddPage("SPLM2", "THIS SPELL REPLENISHES THE CASTERS MANA POOLS\n"
				"BY A SMALL AMMOUNT, AT LITTLE MAGICAL COST TO\n"
				"THE USER. VERY HANDY FOR USE IN THE FIELD.");
	}
};
IMPLEMENT_CLASS(KSpellTwoPage);

//==========================================================================
//
//	KSpellThreePage
//
//==========================================================================

class KSpellThreePage:public KSpellPage
{
	DECLARE_CLASS(KSpellThreePage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellThreePage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "");
		AddPage("SPLC3", "A HIGHLY DEADLY SPELL, ALLOWS THE CASTER\n"
				"TO BANISH VIRTUALLY ANY LESSER FOE TO\n"
				"ANOTHER DIMENTION WITH BUT A SINGLE WORD.");
		AddPage("SPLM3", "BLAH\n"
				"BLAH\n"
				"BLAH.");
	}
};
IMPLEMENT_CLASS(KSpellThreePage);

//==========================================================================
//
//	KSpellFourPage
//
//==========================================================================

class KSpellFourPage:public KSpellPage
{
	DECLARE_CLASS(KSpellFourPage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellFourPage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "");
		AddPage("SPLC4", "BLAH\n"
				"BLAH\n"
				"BLAH.");
		AddPage("SPLM4", "BLAH\n"
				"BLAH\n"
				"BLAH.");
	}
};
IMPLEMENT_CLASS(KSpellFourPage);

//==========================================================================
//
//	KSpellFivePage
//
//==========================================================================

class KSpellFivePage:public KSpellPage
{
	DECLARE_CLASS(KSpellFivePage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellFivePage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "");
		AddPage("SPLC4", "");
		AddPage("SPLM5", "BLAH\n"
				"BLAH\n"
				"BLAH.");
	}
};
IMPLEMENT_CLASS(KSpellFivePage);

//==========================================================================
//
//	KSpellSixPage
//
//==========================================================================

class KSpellSixPage:public KSpellPage
{
	DECLARE_CLASS(KSpellSixPage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellSixPage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "");
		AddPage("SPLC4", "");
		AddPage("SPLM6", "BLAH\n"
				"BLAH\n"
				"BLAH.");
	}
};
IMPLEMENT_CLASS(KSpellSixPage);

//==========================================================================
//
//	KSpellSevenPage
//
//==========================================================================

class KSpellSevenPage:public KSpellPage
{
	DECLARE_CLASS(KSpellSevenPage, KSpellPage, 0);
	NO_DEFAULT_CONSTRUCTOR(KSpellSevenPage);

	void CreatePages(void)
	{
		AddPage("SPLF1", "");
		AddPage("SPLC4", "");
		AddPage("SPLM7", "AS OF YET, NO ONE HAS SURVIVED THIS BRUTAL\n"
				"SPELL, AND AS SUCH WE CANNOT ASSERTAIN JUST\n"
				"HOW HORRIBLY PAINFUL IT IS. SUFFICE TO SAY,\n"
				"THE POOLS OF BLOOD AND SCREAMS OF AGONY WOULD\n"
				"LEAD ONE TO ASSUME IT LIVES UP TO ITS NAME.");
	}
};
IMPLEMENT_CLASS(KSpellSevenPage);

//==========================================================================
//
//	KSpellScreen
//
//==========================================================================

class KSpellScreen:public KMenuScreen
{
	DECLARE_CLASS(KSpellScreen, KMenuScreen, 0);

	KSpellPageBaseWindow *winSpellPages[7];

	int currentSpell;
	int s;

	KSpellScreen(void)
	{
		X = -120;
		Y = -12;
		bUseSelector = false;
		Font = KCanvas::SmallFont;
	}

	void InitWindow(void)
	{
		SetStartSpell();
		Super::InitWindow();

		winSpellPages[0] = NewWindow(KSpellOnePage, this);
		winSpellPages[0]->Hide();

		winSpellPages[1] = NewWindow(KSpellTwoPage, this);
		winSpellPages[1]->Hide();

		winSpellPages[2] = NewWindow(KSpellThreePage, this);
		winSpellPages[2]->Hide();

		winSpellPages[3] = NewWindow(KSpellFourPage, this);
		winSpellPages[3]->Hide();

		winSpellPages[4] = NewWindow(KSpellFivePage, this);
		winSpellPages[4]->Hide();

		winSpellPages[5] = NewWindow(KSpellSixPage, this);
		winSpellPages[5]->Hide();

		winSpellPages[6] = NewWindow(KSpellSevenPage, this);
		winSpellPages[6]->Hide();

		DisplayStartSpell();
	}

	void DrawWindow(KGC *gc)
	{
		int i;
		player_t *player;
		player = &players[consoleplayer];
		static char* Spell[7];
		static int coords[7][2];

		s = 1;
		if (player->pclass == 0)
		{
			Spell[0] = "BERSERKER";
			coords[0][0] = 100;
			coords[0][1] = 140;
		}
		else if (player->pclass == 1)
		{
			Spell[0] = "HEALING";
			coords[0][0] = 100;
			coords[0][1] = 140;
			if (player->exp_level > 4)
			{
				Spell[1] = "SPIRITS WITHIN";
				coords[1][0] = 150;
				coords[1][1] = 140;
				s++;
			}
			if (player->exp_level > 6)
			{
				Spell[2] = "BANISHMENT";
				coords[2][0] = 200;
				coords[2][1] = 140;
				s++;
			}
			if (player->exp_level > 8)
			{
				Spell[3] = "WRATH OF THE GODS";
				coords[3][0] = 250;
				coords[3][1] = 140;
				s++;
			}
		}
		else
		{
			Spell[0] = "REPULSION";
			coords[0][0] = 100;
			coords[0][1] = 140;
			if (player->exp_level > 2)
			{
				Spell[1] = "MANA CREATION";
				coords[1][0] = 150;
				coords[1][1] = 140;
				s++;
			}
			if (player->exp_level > 4)
			{
				Spell[2] = "DEFENCE";
				coords[2][0] = 200;
				coords[2][1] = 140;
				s++;
			}
			if (player->exp_level > 6)
			{
				Spell[3] = "SPEED";
				coords[3][0] = 250;
				coords[3][1] = 140;
				s++;
			}
			if (player->exp_level > 7)
			{
				Spell[4] = "SUMMON MONSTER";
				coords[4][0] = 300;
				coords[4][1] = 140;
				s++;
			}
			if (player->exp_level > 8)
			{
				Spell[5] = "POSSESSION";
				coords[5][0] = 350;
				coords[5][1] = 140;
				s++;
			}
			if (player->exp_level > 9)
			{
				Spell[6] = "HORRIBLE PAIN";
				coords[6][0] = 400;
				coords[6][1] = 140;
				s++;
			}
		}

		GL_SetFilter(0);
		//Draw back
		GCanvas->DrawRawScreen(W_GetNumForName("SPLBOOK"));
		//Draw Menu, and actual one
		for (i=0;i<s;i++)
			MN_DrTextA(Spell[i],coords[i][1],coords[i][0]);
		MN_DrTextAYellow(Spell[currentSpell],coords[currentSpell][1],coords[currentSpell][0]);
	}

	bool KeyPressed(int key)
	{
		player_t *player;
		player = &players[consoleplayer];

		if (key == DDKEY_ESCAPE || key == 'z')
		{
			PopMenu();
			return true;
		}
		if (key == DDKEY_ENTER)
		{
			if (player->pclass == 0)
			{
				player->currentspell = currentSpell + 12;
			}
			else if (player->pclass == 1)
			{
				player->currentspell = currentSpell + 8;
			}
			else
			{
				player->currentspell = currentSpell + 1;
			}
			PopMenu();
			return true;
		}
		if (key == 's')
		{
			winSpellPages[currentSpell]->Hide();
			currentSpell = currentSpell == (s-1) ? 0 : currentSpell + 1;
			winSpellPages[currentSpell]->Show();
			SetFocusWindow(winSpellPages[currentSpell]);
			return true;
		}
		if (key == 'w')
		{
			winSpellPages[currentSpell]->Hide();
			currentSpell = currentSpell == 0 ? (s-1) : currentSpell - 1;
			winSpellPages[currentSpell]->Show();
			SetFocusWindow(winSpellPages[currentSpell]);
			return true;
		}
		return Super::KeyPressed(key);
	}

	void SetStartSpell(void)
	{
		player_t *player;
		player = &players[consoleplayer];

		if (player->currentspell > 0)
		{
			if (player->pclass == 0)
			{
				currentSpell = player->currentspell - 12;
			}
			else if (player->pclass == 1)
			{
				currentSpell = player->currentspell - 8;
			}
			else
			{
				currentSpell = player->currentspell - 1;
			}
		}
	}

	void DisplayStartSpell(void)
	{
		winSpellPages[currentSpell]->Show();
		SetFocusWindow(winSpellPages[currentSpell]);
	}
};
IMPLEMENT_CLASS(KSpellScreen);