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
		//ButtonDefaults[3] = FButtonDefault("CORVUS", MA_Custom, MENU_NONE, 3); //Da Corvus man
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
//		spriteinfo_t	sprInfo;
//		int				w, h, alpha;

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
		//int pclass = Items[CursorPos]->option;
		int pclass = CursorPos;
		gc->DrawIcon(334, 148, FindTexture(boxLumpName[pclass]));
		/*if (pclass == 3) //Remi: Corvus, never happens in this source state
		{
			gl.GetIntegerv(DGL_A, &alpha);
			gi.GetSpriteInfo(SPR_PLAY, 0, &sprInfo);	
			gi.Set(DD_TRANSLATED_SPRITE_TEXTURE, DD_TSPR_PARM(sprInfo.lump, pclass, 0));
			gi.Set(DD_SPRITE_SIZE_QUERY, sprInfo.lump);
			w = gi.Get(DD_QUERY_RESULT) >> 16;		// High word.
			h = gi.Get(DD_QUERY_RESULT) & 0xffff;	// Low word.
			gi.GL_DrawRect(162-(sprInfo.offset>>16), 90 - (sprInfo.topOffset>>16), w, h, 1, 1, 1, alpha/255.0f);
			gl.Color4ub(255, 255, 255, alpha);
		}
		else*/
		gc->GetCanvas()->DrawPatch(334+24, 148+12, 
			gi.W_GetNumForName(walkLumpName[pclass])+((MenuTime>>3)&3));
	}
};
IMPLEMENT_CLASS(KMenuScreenClass);

