//==========================================================================
//
//	KUpdatingLevelDesc
//
//==========================================================================

class KUpdatingLevelDesc:public KWindow
{
	DECLARE_CLASS(KUpdatingLevelDesc, KMenuItem_t, 0);

	char leveldesc[4][40];

	KUpdatingLevelDesc(void)
	{
		Font = KCanvas::SmallFont;
	}

	void DrawWindow(KGC *gc)
	{
		for (int i = 0; i < 4; i++)
			gc->DrawText(0, i * 13, leveldesc[i]);
	}
};
IMPLEMENT_CLASS(KUpdatingLevelDesc);

//==========================================================================
//
//	KUpdatingHeader
//
//==========================================================================

class KUpdatingHeader:public KWindow
{
	DECLARE_CLASS(KUpdatingHeader, KWindow, 0);

	KUpdatingHeader(void)
	{
		Width = 320;
		Height = 10;
		Font = KCanvas::SmallFont;
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(0, 0, "POINTS:");
		gc->DrawText(90, 0, "OLD");
		gc->DrawText(120, 0, " NEW ");
	}
};
IMPLEMENT_CLASS(KUpdatingHeader);

//==========================================================================
//
//	KUpdatingRemaining
//
//==========================================================================

class KUpdatingRemaining:public KWindow
{
	DECLARE_CLASS(KUpdatingRemaining, KWindow, 0);

	unsigned int Value;

	KUpdatingRemaining(void)
	{
		Width = 320;
		Height = 10;
		Font = KCanvas::SmallFont;
	}

	void DrawWindow(KGC *gc)
	{
		char tmp[16];

		gc->DrawText(0, 0, "REMAINING:");
		sprintf(tmp, "%02d", Value);
		gc->DrawText(120, 0, tmp);
	}
};
IMPLEMENT_CLASS(KUpdatingRemaining);

//==========================================================================
//
//	KUpdatingScroller
//
//==========================================================================

class KUpdatingScroller:public KMenuUIChoice
{
	DECLARE_CLASS(KUpdatingScroller, KMenuUIChoice, 0);

	int StartValue;
	int Value;
	int MaxValue;
	KUpdatingRemaining *WinNav;

	KUpdatingScroller(void)
	{
		MaxValue = -1;
	}
	void FocusEnteredWindow(void)
	{
		Super::FocusEnteredWindow();
		SetFont(KCanvas::YellowFont);
	}
	void FocusLeftWindow(void)
	{
		Super::FocusLeftWindow();
		SetFont(KCanvas::SmallFont);
	}

	bool KeyPressed(int key)
	{
		if (!players[consoleplayer].berserkTics)
		{ 
			//Remi: Player can't mess with stats while berserking		
			if (key == DDKEY_LEFTARROW)
			{
				if (StartValue < Value)
				{
					Value--;
					WinNav->Value++;
				}
				return true;
			}
			if (key == DDKEY_RIGHTARROW)
			{
				if (WinNav->Value > 0 && (Value < MaxValue || MaxValue == -1)) 
				{
					Value++;
					WinNav->Value--;
				}
				return true;
			}
		}
		return Super::KeyPressed(key);
	}

	void DrawWindow(KGC *gc)
	{
		char tmp[16];

		Super::DrawWindow(gc);
		sprintf(tmp,"%02d", StartValue);
		gc->DrawText(90, 0, tmp);
		if (bIsSensitive)
			sprintf(tmp, "- %02d +", Value);
		else
			sprintf(tmp, "  %3d  ", Value);
		gc->DrawText(120, 0, tmp);
	}
};
IMPLEMENT_CLASS(KUpdatingScroller);

//==========================================================================
//
//	KUpdatingOkButton
//
//==========================================================================

class KUpdatingOkButton:public KMenuUIButton
{
	DECLARE_CLASS(KUpdatingOkButton, KMenuUIButton, 0);

	KUpdatingOkButton(void)
	{
		Font = KCanvas::SmallFont;
	}
	void FocusEnteredWindow(void)
	{
		Super::FocusEnteredWindow();
		SetFont(KCanvas::YellowFont);
	}
	void FocusLeftWindow(void)
	{
		Super::FocusLeftWindow();
		SetFont(KCanvas::SmallFont);
	}
};
IMPLEMENT_CLASS(KUpdatingOkButton);

//==========================================================================
//
//	KUpdatingScreen
//
//==========================================================================

class KUpdatingScreen:public KMenuScreen
{
	DECLARE_CLASS(KUpdatingScreen, KMenuScreen, 0);

	KUpdatingScroller *WinHealth;
	KUpdatingScroller *WinSPPower;
	KUpdatingScroller *WinStrength;
	KUpdatingScroller *WinEfficiency;
	KUpdatingScroller *WinSpeed;
	KUpdatingRemaining *WinNav;
	KUpdatingOkButton *WinOkButton;

	KUpdatingScreen(void)
	{
		bUseSelector = false;
		Font = KCanvas::SmallFont;
	}

	void CreateChoices(void)
	{
		player_t &player = players[consoleplayer];
		int CurX = 260;
		int CurY = 200;

		KUpdatingLevelDesc *Desc = NewWindow(KUpdatingLevelDesc, this);
		Desc->SetPos(170, 120);
		Desc->SetSize(300, 52);
		sprintf(Desc->leveldesc[0],"YOU'RE LEVEL  %d",player.exp_level);
		sprintf(Desc->leveldesc[1],"EXPERIENCE POINTS: %d",player.experience);
		sprintf(Desc->leveldesc[2],"YOU NEED %d POINTS FOR LEVEL %d",player.next_level-player.experience,player.exp_level+1);
		sprintf(Desc->leveldesc[3],"SILVER: %d",player.money);
		Desc->Disable();

		KUpdatingHeader *Hdr = NewWindow(KUpdatingHeader, this);
		Hdr->SetPos(CurX, CurY - 26);

		WinHealth = NewWindow(KUpdatingScroller, this);
		WinHealth->SetPos(CurX, CurY);
		WinHealth->ActionText = "HEALTH:";
		WinHealth->StartValue = player.maxhealth_old;
		WinHealth->Value = player.maxhealth;
		WinHealth->Disable();
		CurY += 13;

		WinSPPower = NewWindow(KUpdatingScroller, this);
		WinSPPower->SetPos(CurX, CurY);
		WinSPPower->ActionText = sp_wording[player.pclass];
		WinSPPower->StartValue = player.sp_power_old;
		WinSPPower->Value = player.maxsp_power;
		WinSPPower->Disable();
		CurY += 13;

		WinStrength = NewWindow(KUpdatingScroller, this);
		WinStrength->SetPos(CurX, CurY);
		WinStrength->ActionText = "STRENGTH:";
		WinStrength->StartValue = player.strength;
		WinStrength->Value = player.strength;
		CurY += 13;

		WinEfficiency = NewWindow(KUpdatingScroller, this);
		WinEfficiency->SetPos(CurX, CurY);
		WinEfficiency->ActionText = "EFFICIENCY:";
		WinEfficiency->StartValue = player.agility;
		WinEfficiency->Value = player.agility;
		CurY += 13;

		WinSpeed = NewWindow(KUpdatingScroller, this);
		WinSpeed->SetPos(CurX, CurY);
		WinSpeed->ActionText = "SPEED:";
		WinSpeed->StartValue = player.speed;
		WinSpeed->Value = player.speed;

		switch (player.pclass)
		{
		case PCLASS_FIGHTER:
			WinStrength->MaxValue = -1;
			WinEfficiency->MaxValue = 5;
			WinSpeed->MaxValue = 30;
			break;
		case PCLASS_CLERIC:
			WinStrength->MaxValue = 60;
			WinEfficiency->MaxValue = 40;
			WinSpeed->MaxValue = 20;
			break;
		case PCLASS_MAGE:
			WinStrength->MaxValue = 5;
			WinEfficiency->MaxValue = -1;
			WinSpeed->MaxValue = 10;
			break;
		}

		CurY += 26;
		WinNav = NewWindow(KUpdatingRemaining, this);
		WinNav->SetPos(CurX, CurY);
		WinNav->Value = player.av_points;
		WinStrength->WinNav = WinNav;
		WinEfficiency->WinNav = WinNav;
		WinSpeed->WinNav = WinNav;

		CurY += 26;
		WinOkButton = NewWindow(KUpdatingOkButton, this);
		WinOkButton->SetPos(CurX + 120, CurY);
		WinOkButton->SetButtonText("OK");
	}

	void AcceptUpdating(void)
	{
		if ((WinStrength->StartValue <= WinStrength->Value)&&
			(WinEfficiency->StartValue <= WinEfficiency->Value)&&
			(WinSpeed->StartValue <= WinSpeed->Value)&&
			(players[consoleplayer].av_points >= WinNav->Value))
		{
			players[consoleplayer].av_points = WinNav->Value;
			players[consoleplayer].maxhealth_old = players[consoleplayer].maxhealth;
			players[consoleplayer].sp_power_old = players[consoleplayer].maxsp_power;
			players[consoleplayer].strength = WinStrength->Value;
			players[consoleplayer].agility = WinEfficiency->Value;
			players[consoleplayer].speed = WinSpeed->Value;
		}
	}

	bool ButtonActivated(KWindow *ButtonPressed)
	{
		if (ButtonPressed == WinOkButton)
		{
			AcceptUpdating();
			PopMenu();
			return true;
		}
		return Super::ButtonActivated(ButtonPressed);
	}

	bool KeyPressed(int key)
	{
		if (key == DDKEY_ESCAPE || key == 'u')
		{
			PopMenu();
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KUpdatingScreen);
