
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

IMPLEMENT_CLASS(KTextWindow);

//==========================================================================
//
//	KTextWindow::KTextWindow
//
//==========================================================================

KTextWindow::KTextWindow()
{
	guard(KTextWindow::KTextWindow);
	Font = KCanvas::SmallFont;
	bIsSensitive = false;
	unguard;
}

//==========================================================================
//
//	KTextWindow::KTextWindow
//
//==========================================================================

void KTextWindow::CleanUp(void)
{
	if (Text)
	{
		delete Text;
	}
	Super::CleanUp();
}

//==========================================================================
//
//	KTextWindow::SetText
//
//==========================================================================

void KTextWindow::SetText(const char *NewText)
{
	guard(KTextWindow::SetText);
	if (Text)
	{
		delete Text;
	}
	Text = new char[strlen(NewText) + 1];
	strcpy(Text, NewText);
	unguard;
}

//==========================================================================
//
//	KTextWindow::AppendText
//
//==========================================================================

void KTextWindow::AppendText(const char *NewText)
{
	guard(KTextWindow::SetText);
	if (Text)
	{
		char *OldText = Text;
		Text = new char[strlen(OldText) + strlen(NewText) + 1];
		strcpy(Text, OldText);
		strcat(Text, NewText);
		delete OldText;
	}
	else
	{
		SetText(NewText);
	}
	unguard;
}

//==========================================================================
//
//	KTextWindow::SetTextAlignments
//
//==========================================================================

void KTextWindow::SetTextAlignments(EHAlign newHAlign, EVAlign newVAlign)
{
	guard(KTextWindow::SetTextAlignments);
	HAlign = newHAlign;
	VAlign = newVAlign;
	unguard;
}

//==========================================================================
//
//	KTextWindow::DrawWindow
//
//==========================================================================

void KTextWindow::DrawWindow(KGC *gc)
{
	guard(KTextWindow::DrawWindow);
	if (Text)
	{
		gc->SetAlignments(HAlign, VAlign);
		int TextX = 0;
		int TextY = 0;
		if (HAlign == HALIGN_Center)
			TextX = Width / 2;
		else if (HAlign == HALIGN_Right)
			TextX = Width;
		if (VAlign == VALIGN_Center)
			TextY = Height / 2;
		else if (VAlign == VALIGN_Bottom)
			TextY = Height;
		gc->DrawText(TextX, TextY, Text);
	}
	unguard;
}
