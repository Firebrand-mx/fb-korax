//==========================================================================
//
//	KJournalImageDisplayWindow
//
//==========================================================================

class KJournalImageDisplayWindow:public KWindow
{
	DECLARE_CLASS(KJournalImageDisplayWindow, KWindow, 0);

	KTexture *Texture;
	int PicX;
	int PicY;

	KJournalImageDisplayWindow(void)
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
IMPLEMENT_CLASS(KJournalImageDisplayWindow);

//==========================================================================
//
//	KJournalDescriptionWindow
//
//==========================================================================

class KJournalDescriptionWindow:public KWindow
{
	DECLARE_CLASS(KJournalDescriptionWindow, KWindow, 0);

	char DescStrings[16][64];

	KJournalDescriptionWindow(void)
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
IMPLEMENT_CLASS(KJournalDescriptionWindow);

//==========================================================================
//
//	KJournalPageBaseWindow
//
//==========================================================================

class KJournalPageBaseWindow:public KWindow
{
	DECLARE_CLASS(KJournalPageBaseWindow, KWindow, 0);

	KJournalPageBaseWindow(void)
	{
		X = 160;
		Y = 20;
		Width = 240;
		Height = 200;
	}
};
IMPLEMENT_CLASS(KJournalPageBaseWindow);

//==========================================================================
//
//	KJournalStatsPage
//
//==========================================================================

class KJournalStatsPage:public KJournalPageBaseWindow
{
	DECLARE_CLASS(KJournalStatsPage, KJournalPageBaseWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KJournalStatsPage);

	KTextWindow *winTitle;

	void InitWindow(void)
	{
		Super::InitWindow();
		CreateTitle();
	}

	void CreateTitle(void)
	{
		winTitle = NewWindow(KTextWindow, this);
		winTitle->SetPos(0, 18);
		winTitle->SetSize(240, 12);
		winTitle->SetText("YOUR STATISTICS");
		winTitle->SetFont(KCanvas::YellowFont);
		winTitle->SetTextAlignments(HALIGN_Center, VALIGN_Center);
	}

	void DrawWindow(KGC *gc)
	{
		int i,temp=0;
		int time_mod=*(int*) gi.GetCVar("time_mod")->ptr;
		int months=1;
		int days=1;
		int hours=0;
		int minutes=0;
		int worldTimer;
		int tempMon;
		static char tmp[12][40];
		player_t *player;

		static float height_float [4] =
		{
			{6},{5.5f},{5},{2}
		};

		player = &players[consoleplayer];
		worldTimer = player->worldTimer;

		worldTimer /= 35;
		worldTimer = worldTimer * 60 / time_mod;
		months = worldTimer / 43200;
		worldTimer -= months * 43200;
		days = worldTimer / 1440;
		worldTimer -= days * 1440;
		hours = worldTimer / 60;
		worldTimer -= hours * 60;
		minutes = worldTimer;
		temp = AutoArmorSave[player->pclass]
			+player->armorpoints[ARMOR_ARMOR]+player->armorpoints[ARMOR_SHIELD]
			+player->armorpoints[ARMOR_HELMET]+player->armorpoints[ARMOR_AMULET];
	
		tempMon=player->money/10;

		sprintf(tmp[0],"");
		sprintf(tmp[1],"TODAY IS %d DAY, %d:%d",days,hours,minutes);
		sprintf(tmp[2],"HEIGHT: %.1f FEET",height_float[player->pclass]);

		sprintf(tmp[3],"HEALTH POINTS: %d OF %d", 
			player->plr->mo->health, player->maxhealth);
		sprintf(tmp[4],"STRENGTH: %d",player->strength);
		sprintf(tmp[5],"AGILITY: %d",player->agility);
		sprintf(tmp[6],"SPEED: %d",player->speed);
		sprintf(tmp[7],"DEFENSE: %d",FixedDiv(temp, 5*FRACUNIT)>>FRACBITS);
		sprintf(tmp[8],"YOU'RE LEVEL %d",player->exp_level);
		sprintf(tmp[9],"EXPERIENCE POINTS %d",player->experience);
		sprintf(tmp[10],"YOU NEED %d POINTS FOR LVL %d", 
			player->next_level - player->experience, player->exp_level + 1);
		sprintf(tmp[11],"MONEY: %d PH'ORIND %d FIL-AIRE",
			tempMon, player->money - (tempMon * 10));
		for (i = 0; i < 12; i++)
		{
			gc->DrawText(5, i * 13 + 30, tmp[i]);
		}
	}
};
IMPLEMENT_CLASS(KJournalStatsPage);

//==========================================================================
//
//	KJournalEnemiesPage
//
//==========================================================================

class KJournalEnemiesPage:public KJournalPageBaseWindow
{
	DECLARE_CLASS(KJournalEnemiesPage, KJournalPageBaseWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KJournalEnemiesPage);

	struct FEnemyInfoPage
	{
		char *Title;
		char *ImageName;
		char *Description;
		char *SideDescription;
	};

	KTextWindow *winTitle;
	KJournalImageDisplayWindow *winIcon;
	KJournalDescriptionWindow *winDesc;
	KJournalDescriptionWindow *winSideDesc;

	int CurrentPage;
	int NumPages;
	FEnemyInfoPage Pages[64];

	void InitWindow(void)
	{
		Super::InitWindow();
		CreateTitle();
		CreateIconDisplay();
		CreateDescWindow();
		CreateSideDescWindow();
		CreatePages();
		SetActivePage();
	}

	void CreateTitle(void)
	{
		winTitle = NewWindow(KTextWindow, this);
		winTitle->SetPos(0, 18);
		winTitle->SetSize(240, 12);
		winTitle->SetFont(KCanvas::YellowFont);
		winTitle->SetTextAlignments(HALIGN_Center, VALIGN_Center);
	}

	void CreateIconDisplay(void)
	{
		winIcon = NewWindow(KJournalImageDisplayWindow, this);
		winIcon->SetPos(0, 20);
		winIcon->SetSize(110, 120);
		winIcon->SetPicPos(55, 109);
	}

	void CreateDescWindow(void)
	{
		winDesc = NewWindow(KJournalDescriptionWindow, this);
		winDesc->SetPos(5, 140);
		winDesc->SetSize(230, 60);
	}

	void CreateSideDescWindow(void)
	{
		winSideDesc = NewWindow(KJournalDescriptionWindow, this);
		winSideDesc->SetPos(118, 35);
		winSideDesc->SetSize(120, 100);
	}

	void AddPage(char *Title, char *ImageName, char *Description, char *SideDesc)
	{
		if (NumPages >= 64)
		{
			gi.Error("Too many enemy pages");
		}
		Pages[NumPages].Title = Title;
		Pages[NumPages].ImageName = ImageName;
		Pages[NumPages].Description = Description;
		Pages[NumPages].SideDescription = SideDesc;
		NumPages++;
	}

	void CreatePages(void)
	{
		AddPage("ETTIN", "ETTNG2", "THESE HORRID CREATURES WERE\n"
			"ONCE PROUD MEMBERS OF THE\n"
			"LEGION. THEY HATE HUMANS FOR\n"
			"BEING SOMETHING THEY CAN'T BE.",
			"HIT POINTS:\n130\n\nATTACK DAMAGE:\n2-16");
		AddPage("AFRIT", "FDMND1", "ESSENTIALLY A FIRE GARGOYLE,\n"
			"IT'S MORE THAN HAPPY TO DOUSE\n"
			"YOU IN FLAMES. AND UNLIKE YOU,\n"
			"THIS BEAST CAN FLY.",
			"HIT POINTS:\n80\n\nATTACK DAMAGE:\nUP TO 8");
		AddPage("CENTAUR", "CENTG8", "THE SPINE OF KORAX' ARMIES,\n"
			"THESE MYTHICAL HORSE-MEN ARE\n"
			"VICIOUS, BRUTAL AND HARD TO\n"
			"KILL. CAN DEFLECT MISSILES.",
			"HIT POINTS:\n200\n\nATTACK DAMAGE:\nUP TO 10");
		AddPage("FOREST CHAOS SERPENT", "DEMNE2E8", "ONCE THEY APPEAR, NO FOREST\n"
			"CAN BE CALLED PEACEFUL ANY\n"
			"MORE. THEIR BREATHING IS FIRE.",
			"HIT POINTS:\n250\n\nATTACK DAMAGE:\n5-40");
		AddPage("WASTELAND CHAOS SERPENT", "DEM2G1", "A BROWN MENACE OF MOST DESERTS\n"
			"ON CRONOS, THESE POISON-\n"
			"BELCHING MONSTERS ARE A TOUGH\n"
			"CHALLENGE FOR ALL BUT THE BEST.",
			"HIT POINTS:\n500\n\nATTACK DAMAGE:\n10-80");
		AddPage("DEATH WYVERN", "DRAGB2B8", "AN UNDEAD DRAGON, ITS THIRST\n"
			"FOR THE BLOOD OF THE LIVING IS\n"
			"SURPASSED ONLY BY ITS HATRED\n"
			"OF ANYTHING BUT ITSELF.",
			"HIT POINTS:\n640\n\nATTACK DAMAGE:\nUP TO 128");
	}

	void SetActivePage(void)
	{
		winTitle->SetText(Pages[CurrentPage].Title);
		winIcon->SetTexture(FindTexture(Pages[CurrentPage].ImageName));
		winDesc->SetText(Pages[CurrentPage].Description);
		winSideDesc->SetText(Pages[CurrentPage].SideDescription);
	}

	bool KeyPressed(int key)
	{
		if (key == DDKEY_LEFTARROW)
		{
			if (CurrentPage > 0)
				CurrentPage--;
			SetActivePage();
			return true;
		}
		if (key == DDKEY_RIGHTARROW)
		{
			if (CurrentPage < NumPages - 1)
				CurrentPage++;
			SetActivePage();
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KJournalEnemiesPage);

//==========================================================================
//
//	KJournalItemsPage
//
//==========================================================================

class KJournalItemsPage:public KJournalPageBaseWindow
{
	DECLARE_CLASS(KJournalItemsPage, KJournalPageBaseWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KJournalItemsPage);

	struct FJournalItemPage
	{
		char *Title;
		char *ImageName;
		char *Description;
	};

	KTextWindow *winTitle;
	KJournalImageDisplayWindow *winIcon;
	KJournalDescriptionWindow *winDesc;

	int CurrentPage;
	int NumPages;
	FJournalItemPage Pages[64];

	void InitWindow(void)
	{
		Super::InitWindow();
		CreateTitle();
		CreateIconDisplay();
		CreateDescWindow();
		CreatePages();
		SetActivePage();
	}

	void CreateTitle(void)
	{
		winTitle = NewWindow(KTextWindow, this);
		winTitle->SetPos(0, 18);
		winTitle->SetSize(240, 12);
		winTitle->SetFont(KCanvas::YellowFont);
		winTitle->SetTextAlignments(HALIGN_Center, VALIGN_Center);
	}

	void CreateIconDisplay(void)
	{
		winIcon = NewWindow(KJournalImageDisplayWindow, this);
		winIcon->SetPos(0, 20);
		winIcon->SetSize(240, 85);
		winIcon->SetPicPos(105, 79);
	}

	void CreateDescWindow(void)
	{
		winDesc = NewWindow(KJournalDescriptionWindow, this);
		winDesc->SetPos(5, 110);
		winDesc->SetSize(230, 90);
	}

	void AddPage(char *Title, char *ImageName, char *Description)
	{
		if (NumPages >= 64)
		{
			gi.Error("Too many item pages");
		}
		Pages[NumPages].Title = Title;
		Pages[NumPages].ImageName = ImageName;
		Pages[NumPages].Description = Description;
		NumPages++;
	}

	void CreatePages(void)
	{
		AddPage("TORCH", "TRCHB0", "THIS SIMPLE ITEM LETS YOU\n"
			"SEE BETTER IN DARK PLACES.\n"
			"ONCE LIT, IT'LL BE USED UP\n"
			"WITHIN A SHORT TIME.");
		AddPage("HEALTH VIAL", "PTN1A0", "FOUND FLOATING IN THE AIR IN\n"
			"SOME PLACES, THIS SMALL VIAL\n"
			"RESTORES A SMALL PART OF YOUR\n"
			"LIFE FORCE, HEALING WOUNDS IN\n"
			"THE PROCESS.");
		AddPage("QUARTZ FLASK", "PTN2A0", "FOUND FLOATING IN THE AIR IN\n"
			"SOME PLACES, THIS SMALL VIAL\n"
			"RESTORES A LARGE PART OF YOUR\n"
			"LIFE FORCE, HEALING WOUNDS IN\n"
			"THE PROCESS.");
		AddPage("BLUE MANA", "MAN1H0", "THIS MAGICAL ITEM INCREASES\n"
			"THE AMOUNT OF BLUE MANA YOUR\n"
			"WEAPONS HAVE AT THEIR DISPOSAL.");
		AddPage("GREEN MANA", "MAN2P0", "THIS MAGICAL ITEM INCREASES\n"
			"THE AMOUNT OF GREEN MANA YOUR\n"
			"WEAPONS HAVE AT THEIR DISPOSAL.");
		AddPage("COMBINED MANA", "MAN3H0", "THIS MAGICAL ITEM INCREASES\n"
			"THE AMOUNT OF ALL MANA YOUR\n"
			"WEAPONS HAVE AT THEIR DISPOSAL.");
		AddPage("KRATER OF MIGHT", "BMANA0", "THIS MAGICAL ITEM INCREASES\n"
			"THE AMOUNT OF ALL YOUR MANA\n"
			"TO THE MAXIMUM.");
		AddPage("MESH ARMOR", "ARM1A0", "THIS LIGHT ARMOR IS MOST\n"
			"USEFUL TO THE FIGHTER AND\n"
			"LEAST USEFUL TO THE MAGE.");
		AddPage("FALCON SHIELD", "ARM2A0", "FROM ALL THE HEROES, A CLERIC\n"
			"TAKES OUT THE MOST PROTECTION\n"
			"FROM THIS HOLY SHIELD.");
		AddPage("AMULET OF WARDING", "ARM4A0", "FROM THIS MAGICAL DEFENSIVE\n"
			"ITEM, A MAGE PROFITS MOST AND\n"
			"A FIGHTER THE LEAST.");
		AddPage("PLATINUM HELM", "ARM3A0", "THIS HELM OFFERS ONLY A\n"
			"SLIGHT PROTECTION AGAINST\n"
			"MONSTERS, BUT IT'S STILL\n"
			"BETTER TO BE ON THE SAFE SIDE.");
		AddPage("DRAGONSKIN BRACERS", "BRACC0", "UNLIKE TRADITIONAL ARMOR,\n"
			"THESE MAGICAL BRACERS WILL\n"
			"LOOSE THEIR POWER AFTER A\n"
			"SHORT TIME, REGARDLESS WHETHER\n"
			"THEY TAKE ANY DAMAGE.");
		AddPage("FLECHETTE", "THRWA0", "A MAGICAL BOMB. THROW IT TO\n"
			"USE. PERFORMS DIFFERENTLY IN\n"
			"THE HANDS OF HEROES FROM\n"
			"DIFFERENT ORDERS.");
		AddPage("DISC OF REPULSION", "BLSTA0", "WHEN YOU USE THIS ARTIFACT,\n"
			"EVERYTHING WITHIN A CERTAIN\n"
			"RADIUS OF YOUR BODY IS DEFLECTED\n"
			"AWAY, INCLUDING MONSTERS, SPELLS,\n"
			"AND EVEN PROJECTILES.");
		AddPage("GOLD BAR", "GOLDA0", "TYPICALLY FOUND IN MINES, THIS\n"
			"TREASURE WILL SELL FOR A VERY\n"
			"HIGH PRICE IN ANY TOWN.");
		AddPage("SILVER BAR", "SILVA0", "TYPICALLY FOUND IN MINES, THIS\n"
			"TREASURE WILL SELL FOR A HIGH\n"
			"PRICE IN ANY TOWN.");
		AddPage("HUMAN SKULL", "RADEI0", "USED AT THE MAKING OF HOLY\n"
			"ARTIFACTS THAT OFFER DEFENSE\n"
			"AGAINST THE UNDEAD, THIS SKULL\n"
			"WILL FETCH A GOOD PRICE IN ANY\n"
			"TEMPLE.");
		AddPage("ANIMAL SKULL", "ASKUA0", "AN ESSENTIAL INGREDIENT FOR\n"
			"MANY A SPELL OR MAGICAL POTION,\n"
			"ANY MAGE WILL PAY YOU A GOOD\n"
			"PRICE FOR SUCH AN ITEM.");
		AddPage("BOOTS OF SPEED", "SPEDB0", "PUTTING THESE BOOTS ON WILL\n"
			"GIVE YOU AN IMMENSE SPEED BOOST,\n"
			"UNTIL THEY WEAR OUT AFTER A\n"
			"SHORT TIME.");
	}

	void SetActivePage(void)
	{
		winTitle->SetText(Pages[CurrentPage].Title);
		winIcon->SetTexture(FindTexture(Pages[CurrentPage].ImageName));
		winDesc->SetText(Pages[CurrentPage].Description);
	}

	bool KeyPressed(int key)
	{
		if (key == DDKEY_LEFTARROW)
		{
			if (CurrentPage > 0)
				CurrentPage--;
			SetActivePage();
			return true;
		}
		if (key == DDKEY_RIGHTARROW)
		{
			if (CurrentPage < NumPages - 1)
				CurrentPage++;
			SetActivePage();
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KJournalItemsPage);

//==========================================================================
//
//	KJournalScreen
//
//==========================================================================

class KJournalScreen:public KMenuScreen
{
	DECLARE_CLASS(KJournalScreen, KMenuScreen, 0);

	KJournalPageBaseWindow *winJournalPages[5];

	int currentJournal;

	KJournalScreen(void)
	{
		bUseSelector = false;
		Font = KCanvas::SmallFont;
	}

	void InitWindow(void)
	{
		Super::InitWindow();

		winJournalPages[0] = NewWindow(KJournalStatsPage, this);

		winJournalPages[1] = NewWindow(KJournalEnemiesPage, this);
		winJournalPages[1]->Hide();

		winJournalPages[2] = NewWindow(KJournalItemsPage, this);
		winJournalPages[2]->Hide();

		winJournalPages[3] = NewWindow(KJournalPageBaseWindow, this);
		winJournalPages[3]->Hide();

		winJournalPages[4] = NewWindow(KJournalPageBaseWindow, this);
		winJournalPages[4]->Hide();
	}

	void DrawWindow(KGC *gc)
	{
		static char* journal[5]=
		{
			{"STATS"},
			{"BESTIARY"},
			{"THINGS"},
			{"MAP"},
			{"NOTES"}
		};
		static int coords[5][2]=
		{
			{21,18},
			{64,20},
			{103,28},
			{140,22},
			{176,22}
		};
		int i;

		gi.GL_SetFilter(0);
		//Draw back
		GCanvas->DrawRawScreen(gi.W_GetNumForName("JOURNAL"));
		//Draw Menu, and actual one
		for (i=0;i<5;i++)
			MN_DrTextA(journal[i],coords[i][1],coords[i][0]);
		MN_DrTextAYellow(journal[currentJournal],coords[currentJournal][1],coords[currentJournal][0]);
	}

	bool KeyPressed(int key)
	{
		if (key == DDKEY_ESCAPE || key == DDKEY_TAB)
		{
			PopMenu();
			return true;
		}
		if (key == DDKEY_DOWNARROW)
		{
			winJournalPages[currentJournal]->Hide();
			currentJournal = currentJournal == 4 ? 0 : currentJournal + 1;
			winJournalPages[currentJournal]->Show();
			GetRootWindow()->SetFocus(winJournalPages[currentJournal]);
			return true;
		}
		if (key == DDKEY_UPARROW)
		{
			winJournalPages[currentJournal]->Hide();
			currentJournal = currentJournal == 0 ? 4 : currentJournal - 1;
			winJournalPages[currentJournal]->Show();
			GetRootWindow()->SetFocus(winJournalPages[currentJournal]);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KJournalScreen);
