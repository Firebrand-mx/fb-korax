//==========================================================================
//
//	KMenuChoice_SfxVolume
//
//==========================================================================

class KMenuChoice_SfxVolume:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_SfxVolume, KMenuUIChoiceSlider, 0);

	KMenuChoice_SfxVolume(void)
	{
		ActionText = "SFX VOLUME";
		NumTicks = 18;
		EndValue = 255;
	}

	void LoadSetting(void)
	{
		SetValue(gi.Get(DD_SFX_VOLUME));
	}

	void SaveSetting(void)
	{
		gi.SetSfxVolume(int(GetValue()));
	}
};
IMPLEMENT_CLASS(KMenuChoice_SfxVolume);

//==========================================================================
//
//	KMenuChoice_MidiVolume
//
//==========================================================================

class KMenuChoice_MidiVolume:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_MidiVolume, KMenuUIChoiceSlider, 0);

	KMenuChoice_MidiVolume(void)
	{
		ActionText = "MIDI VOLUME";
		NumTicks = 18;
		EndValue = 255;
	}

	void LoadSetting(void)
	{
		SetValue(gi.Get(DD_MIDI_VOLUME));
	}

	void SaveSetting(void)
	{
		gi.SetMIDIVolume(int(GetValue()));
	}
};
IMPLEMENT_CLASS(KMenuChoice_MidiVolume);

//==========================================================================
//
//	KMenuChoice_CDAudioVolume
//
//==========================================================================

class KMenuChoice_CDAudioVolume:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_CDAudioVolume, KMenuUIChoiceSlider, 0);

	KMenuChoice_CDAudioVolume(void)
	{
		ActionText = "CD VOLUME";
		NumTicks = 18;
		EndValue = 255;
	}

	void LoadSetting(void)
	{
		SetValue(gi.CD(DD_GET_VOLUME, 0));
	}

	void SaveSetting(void)
	{
		gi.CD(DD_SET_VOLUME, int(GetValue()));
	}
};
IMPLEMENT_CLASS(KMenuChoice_CDAudioVolume);

//==========================================================================
//
//	KMenuChoice_MusicDevice
//
//==========================================================================

class KMenuChoice_MusicDevice:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_MusicDevice, KMenuUIChoiceEnum, 0);

	KMenuChoice_MusicDevice(void)
	{
		ActionText = "MUSIC DEVICE";
		EnumText[0] = "NONE";
		EnumText[1] = "MIDI";
		EnumText[2] = "CD";
	}

	void LoadSetting(void)
	{
		SetValue(gi.Get(DD_MUSIC_DEVICE));
	}

	void SaveSetting(void)
	{
		// Setup the music.
		gi.SetMusicDevice(GetValue());
	
		// Restart the song of the current map.
		S_StartSong(gamemap, true);
	}

};
IMPLEMENT_CLASS(KMenuChoice_MusicDevice);

//==========================================================================
//
//	KMenuChoice_3DSounds
//
//==========================================================================

class KMenuChoice_3DSounds:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_3DSounds, KMenuChoice_OnOff, 0);

	KMenuChoice_3DSounds(void)
	{
		ActionText = "3D SOUNDS";
		CVarName = "s_3d";
	}
};
IMPLEMENT_CLASS(KMenuChoice_3DSounds);

//==========================================================================
//
//	KMenuChoice_ReverbVolume
//
//==========================================================================

class KMenuChoice_ReverbVolume:public KMenuUIChoiceSlider
{
	DECLARE_CLASS(KMenuChoice_ReverbVolume, KMenuUIChoiceSlider, 0);

	KMenuChoice_ReverbVolume(void)
	{
		ActionText = "REVERB VOLUME";
		CVarName = "s_reverbVol";
	}

	void SaveSetting(void)
	{
		extern sector_t *listenerSector;

		Super::SaveSetting();

		// This'll force the sound updater to set the reverb.
		listenerSector = NULL;
	}
};
IMPLEMENT_CLASS(KMenuChoice_ReverbVolume);

//==========================================================================
//
//	KMenuChoice_SFXFrequency
//
//==========================================================================

class KMenuChoice_SFXFrequency:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_SFXFrequency, KMenuUIChoiceEnum, 0);

	KMenuChoice_SFXFrequency(void)
	{
		ActionText = "SFX FREQUENCY";
		EnumText[0] = "11 KHZ";
		EnumText[1] = "22 KHZ";
		EnumText[2] = "44 KHZ";
	}

	void LoadSetting(void)
	{
		int val = *(int*) gi.GetCVar("s_resample")->ptr;
		SetValue(val < 3 ? val - 1 : val - 2);
	}

	void SaveSetting(void)
	{
		int val = GetValue();
		*(int*) gi.GetCVar("s_resample")->ptr = val < 2 ? val + 1 : val + 2;
	}

};
IMPLEMENT_CLASS(KMenuChoice_SFXFrequency);

//==========================================================================
//
//	KMenuChoice_SFX16Bit
//
//==========================================================================

class KMenuChoice_SFX16Bit:public KMenuChoice_OnOff
{
	DECLARE_CLASS(KMenuChoice_SFX16Bit, KMenuChoice_OnOff, 0);

	KMenuChoice_SFX16Bit(void)
	{
		ActionText = "16 BIT INTERPOLATION";
		CVarName = "s_16bit";
	}
};
IMPLEMENT_CLASS(KMenuChoice_SFX16Bit);

//==========================================================================
//
//	KMenuScreenSound
//
//==========================================================================

class KMenuScreenSound:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenSound, KMenuScreen, 0);

	KMenuScreenSound(void)
	{
		ChoiceStartX = 230;
		ChoiceStartY = 160;
	}

	void CreateChoices()
	{
		KMenuUIChoice *Choice;

		Choice = NewWindow(KMenuChoice_SfxVolume, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_MidiVolume, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_CDAudioVolume, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_MusicDevice, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_3DSounds, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_ReverbVolume, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_SFXFrequency, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;

		Choice = NewWindow(KMenuChoice_SFX16Bit, this);
		Choice->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
		Items[NumItems++] = Choice;
	}
};
IMPLEMENT_CLASS(KMenuScreenSound);
