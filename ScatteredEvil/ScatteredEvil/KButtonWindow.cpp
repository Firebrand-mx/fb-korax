
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include "h2def.h"
#include "mn_def.h"

IMPLEMENT_CLASS(KButtonWindow);

//==========================================================================
//
//	KButtonWindow::KButtonWindow
//
//==========================================================================

KButtonWindow::KButtonWindow()
{
	bIsSelectable = true;
	Info[0].Texture = NULL;
	Info[1].Texture = NULL;
	Info[2].Texture = NULL;
	Info[3].Texture = NULL;
	Info[4].Texture = NULL;
	Info[5].Texture = NULL;
	Info[0].TileColor = FColor(255, 255, 255);
	Info[1].TileColor = FColor(255, 255, 255);
	Info[2].TileColor = FColor(255, 255, 255);
	Info[3].TileColor = FColor(255, 255, 255);
	Info[4].TileColor = FColor(255, 255, 255);
	Info[5].TileColor = FColor(255, 255, 255);
	Info[0].TextColor = FColor(255, 255, 255);
	Info[1].TextColor = FColor(255, 255, 255);
	Info[2].TextColor = FColor(255, 255, 255);
	Info[3].TextColor = FColor(255, 255, 255);
	Info[4].TextColor = FColor(255, 255, 255);
	Info[5].TextColor = FColor(255, 255, 255);
	CurTexture = NULL;
	CurTileColor = FColor(255, 255, 255);
	CurTextColor = FColor(255, 255, 255);
}

//==========================================================================
//
//	KButtonWindow::ActivateButton
//
//==========================================================================

void KButtonWindow::ActivateButton(int key)
{
	guard(KButtonWindow::ActivateButton);
	bButtonPressed = true;
	ChangeButtonAppearance();
	PressButton(key);
	unguard;
}

//==========================================================================
//
//	KButtonWindow::SetButtonTextures
//
//==========================================================================

void KButtonWindow::SetButtonTextures(KTexture *normal,
									  KTexture *pressed,
									  KTexture *normalFocus,
									  KTexture *pressedFocus,
									  KTexture *normalInsensitive,
									  KTexture *pressedInsensitive)
{
	guard(KButtonWindow::SetButtonTextures);
	Info[0].Texture = normal;
	Info[1].Texture = pressed;
	Info[2].Texture = normalFocus;
	Info[3].Texture = pressedFocus;
	Info[4].Texture = normalInsensitive;
	Info[5].Texture = pressedInsensitive;
	ChangeButtonAppearance();
	unguard;
}

//==========================================================================
//
//	KButtonWindow::SetButtonColors
//
//==========================================================================

void KButtonWindow::SetButtonColors(FColor normal,
									FColor pressed,
									FColor normalFocus,
									FColor pressedFocus,
									FColor normalInsensitive,
									FColor pressedInsensitive)
{
	guard(KButtonWindow::SetButtonColors);
	Info[0].TileColor = normal;
	Info[1].TileColor = pressed;
	Info[2].TileColor = normalFocus;
	Info[3].TileColor = pressedFocus;
	Info[4].TileColor = normalInsensitive;
	Info[5].TileColor = pressedInsensitive;
	ChangeButtonAppearance();
	unguard;
}

//==========================================================================
//
//	KButtonWindow::SetTextColors
//
//==========================================================================

void KButtonWindow::SetTextColors(FColor normal,
								  FColor pressed,
								  FColor normalFocus,
								  FColor pressedFocus,
								  FColor normalInsensitive,
								  FColor pressedInsensitive)
{
	guard(KButtonWindow::SetTextColors);
	Info[0].TextColor = normal;
	Info[1].TextColor = pressed;
	Info[2].TextColor = normalFocus;
	Info[3].TextColor = pressedFocus;
	Info[4].TextColor = normalInsensitive;
	Info[5].TextColor = pressedInsensitive;
	ChangeButtonAppearance();
	unguard;
}

//==========================================================================
//
//	KButtonWindow::PressButton
//
//==========================================================================

void KButtonWindow::PressButton(int key)
{
	guard(KButtonWindow::PressButton);
	GetParent()->ButtonActivated(this);
	unguard;
}

//==========================================================================
//
//	KButtonWindow::SensitivityChanged
//
//==========================================================================

void KButtonWindow::SensitivityChanged(bool bNewSensitivity)
{
	guard(KButtonWindow::SensitivityChanged);
	Super::SensitivityChanged(bNewSensitivity);
	ChangeButtonAppearance();
	unguard;
}

//==========================================================================
//
//	KButtonWindow::FocusEnteredWindow
//
//==========================================================================

void KButtonWindow::FocusEnteredWindow(void)
{
	guard(KButtonWindow::FocusEnteredWindow);
	Super::FocusEnteredWindow();
	ChangeButtonAppearance();
	unguard;
}

//==========================================================================
//
//	KButtonWindow::FocusLeftWindow
//
//==========================================================================

void KButtonWindow::FocusLeftWindow(void)
{
	guard(KButtonWindow::FocusLeftWindow);
	Super::FocusLeftWindow();
	ChangeButtonAppearance();
	unguard;
}

//==========================================================================
//
//	KButtonWindow::DrawWindow
//
//==========================================================================

void KButtonWindow::DrawWindow(KGC *gc)
{
	guard(KButtonWindow::DrawWindow);
	gc->SetTileColor(CurTileColor);
	gc->SetTextColor(CurTextColor);
	if (CurTexture)
	{
		gc->DrawIcon(0, 0, CurTexture);
	}
	Super::DrawWindow(gc);
	unguard;
}

//==========================================================================
//
//	KButtonWindow::KeyPressed
//
//==========================================================================

bool KButtonWindow::KeyPressed(int key)
{
	guard(KButtonWindow::KeyPressed);
	bool retval;

	retval = Super::KeyPressed(key);
	if ((key == DDKEY_ENTER) || (key == ' '))
	{
		PressButton(key);
		retval = true;
	}
	return retval;
	unguard;
}

//==========================================================================
//
//	KButtonWindow::KButtonWindow
//
//==========================================================================

void KButtonWindow::ChangeButtonAppearance(void)
{
	int StateIndex;

	if (bIsSensitive)
	{
		if (!IsFocusWindow())
		{
			if (!bButtonPressed)
				StateIndex = 0;
			else
				StateIndex = 1;
		}
		else
		{
			if (!bButtonPressed)
				StateIndex = 2;
			else
				StateIndex = 3;
		}
	}
	else
	{
		if (!bButtonPressed)
			StateIndex = 4;
		else
			StateIndex = 5;
	}
	CurTexture = Info[StateIndex].Texture;
	CurTileColor = Info[StateIndex].TileColor;
	CurTextColor = Info[StateIndex].TextColor;
}
