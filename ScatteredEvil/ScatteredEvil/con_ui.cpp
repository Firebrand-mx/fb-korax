#include "h2def.h"
#include "con_local.h"

IMPLEMENT_CLASS(KConChoiceWindow);
IMPLEMENT_CLASS(KConWindow);

//==========================================================================
//
//	KConChoiceWindow::KConChoiceWindow
//
//==========================================================================

KConChoiceWindow::KConChoiceWindow()
{
	Width = 480;
	Height = 10;
}

//==========================================================================
//
//	KConChoiceWindow::InitWindow
//
//==========================================================================

void KConChoiceWindow::InitWindow(void)
{
	WinText = NewWindow(KTextWindow, this);
	WinText->SetPos(10, 0);
	WinText->SetSize(460, 10);
	WinText->SetFont(KCanvas::SmallFont);
}

//==========================================================================
//
//	KConChoiceWindow::SetText
//
//==========================================================================

void KConChoiceWindow::SetText(const char *Text)
{
	WinText->SetText(Text);
}

//==========================================================================
//
//	KConChoiceWindow::FocusEnteredWindow
//
//==========================================================================

void KConChoiceWindow::FocusEnteredWindow(void)
{
	WinText->SetFont(KCanvas::YellowFont);
}

//==========================================================================
//
//	KConChoiceWindow::FocusLeftWindow
//
//==========================================================================

void KConChoiceWindow::FocusLeftWindow(void)
{
	WinText->SetFont(KCanvas::SmallFont);
}

//==========================================================================
//
//	KConWindow::KConWindow
//
//==========================================================================

KConWindow::KConWindow(void)
{
	X = 0;
	Y = 0;
	Width = 640;
	Height = 480;
}

//==========================================================================
//
//	KConWindow::InitWindow
//
//==========================================================================

void KConWindow::InitWindow(void)
{
	Super::InitWindow();
}

//==========================================================================
//
//	KConWindow::Clear
//
//==========================================================================

void KConWindow::Clear(void)
{
	DestroyAllChildren();
	for (int i = 0; i < 10; i++)
		WinChoices[i] = NULL;
	NumChoices = 0;
	CurrentChoice = 0;
}

//==========================================================================
//
//	KConWindow::DisplayText
//
//==========================================================================

void KConWindow::DisplayText(const char *Text)
{
	CreateSpeechWindow();
	WinSpeech->SetText(Text);
	GetRootWindow()->SetFocus(this);
}

//==========================================================================
//
//	KConWindow::DisplayChoice
//
//==========================================================================

void KConWindow::DisplayChoice(KConChoice *Choice)
{
	KConChoiceWindow *newChoice = NewWindow(KConChoiceWindow, this);
	newChoice->SetPos(0, 410 + 10 * NumChoices);
	newChoice->SetText(Choice->ChoiceText);
	newChoice->Choice = Choice;
	WinChoices[NumChoices++] = newChoice;
	if (NumChoices == 1)
	{
		GetRootWindow()->SetFocus(newChoice);
	}
}

//==========================================================================
//
//	KConWindow::KeyPressed
//
//==========================================================================

bool KConWindow::KeyPressed(int key)
{
	switch (key)
	{
	case DDKEY_UPARROW:
		if (NumChoices)
		{
			CurrentChoice--;
			if (CurrentChoice < 0)
				CurrentChoice = NumChoices - 1;
			GetRootWindow()->SetFocus(WinChoices[CurrentChoice]);
		}
		break;

	case DDKEY_DOWNARROW:
		if (NumChoices)
		{
			CurrentChoice++;
			if (CurrentChoice >= NumChoices)
				CurrentChoice = 0;
			GetRootWindow()->SetFocus(WinChoices[CurrentChoice]);
		}
		break;

	case DDKEY_ENTER:
		if (NumChoices)
		{
			ConPlay->PlayChoice(WinChoices[CurrentChoice]->Choice);
		}
		else
		{
			ConPlay->PlayNextEvent();
		}
		break;

	case ' ':
	case DDKEY_ESCAPE:
		if (!NumChoices)
		{
			ConPlay->PlayNextEvent();
		}
		break;
	}
	return true;
}

//==========================================================================
//
//	KConWindow::DescendantRemoved
//
//==========================================================================

void KConWindow::DescendantRemoved(KWindow *Child)
{
	Super::DescendantRemoved(Child);
	if (WinSpeech == Child)
		WinSpeech = NULL;
}

//==========================================================================
//
//	KConWindow::CreateSpeechWindow
//
//==========================================================================

void KConWindow::CreateSpeechWindow(void)
{
	if (!WinSpeech)
	{
		WinSpeech = NewWindow(KTextWindow, this);
		WinSpeech->SetPos(0, 400);
		WinSpeech->SetSize(640, 10);
	}
}
