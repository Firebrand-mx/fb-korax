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
		X = 160;
		Y = 20;
		Width = 240;
		Height = 200;
		bIsSelectable = true;
	}
};
IMPLEMENT_CLASS(KSpellPageBaseWindow);

//==========================================================================
//
//	KSpellScreen
//
//==========================================================================

class KSpellScreen:public KMenuScreen
{
	DECLARE_CLASS(KSpellScreen, KMenuScreen, 0);

	KSpellPageBaseWindow *winSpellPages[3];

	int currentSpell;

	KSpellScreen(void)
	{
		bUseSelector = false;
		Font = KCanvas::SmallFont;
	}

	void InitWindow(void)
	{
		Super::InitWindow();

		winSpellPages[0] = NewWindow(KSpellPageBaseWindow, this);

		winSpellPages[1] = NewWindow(KSpellPageBaseWindow, this);
		winSpellPages[1]->Hide();

		winSpellPages[2] = NewWindow(KSpellPageBaseWindow, this);
		winSpellPages[2]->Hide();
	}

	void DrawWindow(KGC *gc)
	{
		static char* Spell[3]=
		{
			{"OFFENSIVE"},
			{"DEFENSIVE"},
			{"MISC"}
		};
		static int coords[3][2]=
		{
			{100,140},
			{150,140},
			{200,160}
		};
		int i;

		GL_SetFilter(0);
		//Draw back
		GCanvas->DrawRawScreen(W_GetNumForName("HELP1"));
		//Draw Menu, and actual one
		for (i=0;i<3;i++)
			MN_DrTextA(Spell[i],coords[i][1],coords[i][0]);
		MN_DrTextAYellow(Spell[currentSpell],coords[currentSpell][1],coords[currentSpell][0]);
	}

/*	bool KeyPressed(int key)
	{
		if (key == DDKEY_ESCAPE || key == 's')
		{
			PopMenu();
			return true;
		}
		if (key == DDKEY_PGDN)
		{
			winSpellPages[currentSpell]->Hide();
			currentSpell = currentSpell == 2 ? 0 : currentSpell + 1;
			winSpellPages[currentSpell]->Show();
			SetFocusWindow(winSpellPages[currentSpell]);
			return true;
		}
		if (key == DDKEY_PGUP)
		{
			winSpellPages[currentSpell]->Hide();
			currentSpell = currentSpell == 0 ? 2 : currentSpell - 1;
			winSpellPages[currentSpell]->Show();
			SetFocusWindow(winSpellPages[currentSpell]);
			return true;
		}
		return Super::KeyPressed(key);
	}*/
};
IMPLEMENT_CLASS(KSpellScreen);
