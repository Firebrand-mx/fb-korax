//==========================================================================
//
//	KMenuSaveGameSlot
//
//==========================================================================

class KMenuSaveGameSlot:public KMenuUIFileSlot
{
	DECLARE_CLASS(KMenuSaveGameSlot, KMenuItem_t, 0);

	bool FileMenuKeySteal;

	KMenuSaveGameSlot(void)
	{
	}

	bool KeyPressed(int key)
	{
		char *ptr;

		if (FileMenuKeySteal)
		{ 
			// Editing file names
			char *textBuffer;

			textBuffer = &_SlotText[currentSlot][slotptr];
			if (key == DDKEY_BACKSPACE)
			{
				if(slotptr)
				{
					*textBuffer-- = 0;
					*textBuffer = ASCII_CURSOR;
					slotptr--;
				}
				return true;
			}
			if (key == DDKEY_ESCAPE)
			{
				memset(_SlotText[currentSlot], 0, SLOTTEXTLEN+2);
				strcpy(_SlotText[currentSlot], oldSlotText);
				_SlotStatus[currentSlot]--;
				MN_DeactivateMenu();
				return true;
			}
			if (key == DDKEY_ENTER)
			{
				_SlotText[currentSlot][slotptr] = 0; // clear the cursor
				SCSaveGame(currentSlot);
				FileMenuKeySteal = false;
				return true;
			}
			if (slotptr < SLOTTEXTLEN && key != DDKEY_BACKSPACE)
			{
				if((key >= 'a' && key <= 'z'))
				{
					*textBuffer++ = key-32;
					*textBuffer = ASCII_CURSOR;
					slotptr++;
					return(true);
				}
				if(((key >= '0' && key <= '9') || key == ' '
					|| key == ',' || key == '.' || key == '-')
					&& !shiftdown)
				{
					*textBuffer++ = key;
					*textBuffer = ASCII_CURSOR;
					slotptr++;
					return(true);
				}
				if(shiftdown && key == '1')
				{
					*textBuffer++ = '!';
					*textBuffer = ASCII_CURSOR;
					slotptr++;
					return(true);
				}
			}
			return true;
		}
		switch (key)
		{
		case DDKEY_ENTER:
			FileMenuKeySteal = true;
			strcpy(oldSlotText, _SlotText[SlotIndex]);
			ptr = _SlotText[SlotIndex];
			while (*ptr)
			{
				ptr++;
			}
			*ptr = '[';
			*(ptr + 1) = 0;
			_SlotStatus[SlotIndex]++;
			currentSlot = SlotIndex;
			slotptr = ptr - _SlotText[SlotIndex];
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuSaveGameSlot);

//==========================================================================
//
//	KMenuScreenSaveGame
//
//==========================================================================

class KMenuScreenSaveGame:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenSaveGame, KMenuScreen, 0);

	KMenuScreenSaveGame(void)
	{
		ChoiceStartX = 230;
		ChoiceStartY = 170;
	}

	void CreateChoices()
	{
		for (int i = 0; i < 6; i++)
		{
			KMenuSaveGameSlot *Slot = NewWindow(KMenuSaveGameSlot, this);
			Slot->PrepareSlot(i);
			Slot->SetPos(ChoiceStartX, ChoiceStartY + i * itemHeight);
		}
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(320 - Font->TextWidth("SAVE GAME")/2, 150, "SAVE GAME");
	}
};
IMPLEMENT_CLASS(KMenuScreenSaveGame);

