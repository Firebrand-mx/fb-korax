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
		static char *InfoPages[] = { "HELP1", "HELP2", "CREDIT" };

		gi.GL_SetFilter(0);
		gc->GetCanvas()->DrawRawScreen(gi.W_GetNumForName(InfoPages[InfoType]));
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
			gi.Update(DDUF_BORDER);
		}
		else
		{
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
		}
		return true; //make the info screen eat the keypress
	}
};
IMPLEMENT_CLASS(KMenuScreenInfo);
