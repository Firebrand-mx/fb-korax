//==========================================================================
//
//	KMenuScreenInfo
//
//==========================================================================

class KMenuScreenInfo:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenInfo, KMenuScreen, 0);

	int InfoType;

	KMenuScreenInfo(void)
	{
		bUseSelector = false;
	}

	void DrawWindow(KGC *gc)
	{
		player_t *player;
		player = &players[consoleplayer];
		static char *InfoPages[4][3] ={	"HELP1F", "HELP2", "CREDIT",
										"HELP1C", "HELP2", "CREDIT",
										"HELP1M", "HELP2", "CREDIT",
										"HELP1", "HELP2", "CREDIT"};

		GL_SetFilter(0);
		if (gameexpmod < 0.5)
		{
			gc->GetCanvas()->DrawRawScreen(W_GetNumForName(InfoPages[3][InfoType]));
		}
		else if (player->pclass < 3)
		{
			gc->GetCanvas()->DrawRawScreen(W_GetNumForName(InfoPages[player->pclass][InfoType]));
		}
		else
		{
			gc->GetCanvas()->DrawRawScreen(W_GetNumForName(InfoPages[3][InfoType]));
		}
	}

	bool KeyPressed(int key)
	{
		InfoType = (InfoType + 1) % 3;
		if (key == DDKEY_ESCAPE)
		{
			InfoType = 0;
		}
		if (!InfoType)
		{
			MN_DeactivateMenu();
			SB_state = -1; //refresh the statbar
			//BorderNeedRefresh = true;
			DD_GameUpdate(DDUF_BORDER);
		}
		else
		{
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
		}
		return true; //make the info screen eat the keypress
	}
};
IMPLEMENT_CLASS(KMenuScreenInfo);
