#include "h2def.h"
#include "con_local.h"

IMPLEMENT_CLASS(KConChoiceWindow);
IMPLEMENT_CLASS(KConWindow);
IMPLEMENT_CLASS(KConWindowFirst);

//==========================================================================
//
//	KConChoiceWindow::FocusEnteredWindow
//
//==========================================================================

void KConChoiceWindow::FocusEnteredWindow(void)
{
	SetFont(KCanvas::YellowFont);
	Super::FocusEnteredWindow();
}

//==========================================================================
//
//	KConChoiceWindow::FocusLeftWindow
//
//==========================================================================

void KConChoiceWindow::FocusLeftWindow(void)
{
	SetFont(KCanvas::SmallFont);
	Super::FocusLeftWindow();
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
}

//==========================================================================
//
//	KConWindow::AppendText
//
//==========================================================================

void KConWindow::AppendText(const char *Text)
{
	CreateSpeechWindow();
	WinSpeech->AppendText(Text);
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
	newChoice->SetSize(480, 10);
	newChoice->SetText(Choice->ChoiceText);
	newChoice->Choice = Choice;
	WinChoices[NumChoices++] = newChoice;
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
		MoveFocusUp();
		break;

	case DDKEY_DOWNARROW:
		MoveFocusDown();
		break;

	case DDKEY_ENTER:
		if (!NumChoices)
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

bool KConWindow::ButtonActivated(KWindow *button)
{
	KConChoiceWindow *Choice;

	Choice = Cast<KConChoiceWindow>(button);
	if (Choice)
	{
		ConPlay->PlayChoice(Choice->Choice);
		return true;
	}
	return false;
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

//==========================================================================
//
//	KConWindowFirst::KConWindowFirst
//
//==========================================================================

KConWindowFirst::KConWindowFirst(void)
{
	X = 0;
	Y = 400;
	Width = 640;
	Height = 80;
}

//==========================================================================
//
//	KConWindowFirst::InitWindow
//
//==========================================================================

void KConWindowFirst::InitWindow(void)
{
	Super::InitWindow();
}

//==========================================================================
//
//	KConWindowFirst::Clear
//
//==========================================================================

void KConWindowFirst::Clear(void)
{
	DestroyAllChildren();
}

//==========================================================================
//
//	KConWindowFirst::DisplayText
//
//==========================================================================

void KConWindowFirst::DisplayText(const char *Text)
{
	if (!WinSpeech)
	{
		WinSpeech = NewWindow(KTextWindow, this);
		WinSpeech->SetSize(640, 10);
	}
	WinSpeech->SetText(Text);
}

//==========================================================================
//
//	KConWindowFirst::AppendText
//
//==========================================================================

void KConWindowFirst::AppendText(const char *Text)
{
	//	Make sure we have a text window
	if (!WinSpeech)
		return;

	WinSpeech->AppendText(Text);
}

//==========================================================================
//
//	KConWindowFirst::DescendantRemoved
//
//==========================================================================

void KConWindowFirst::DescendantRemoved(KWindow *Child)
{
	Super::DescendantRemoved(Child);
	if (WinSpeech == Child)
		WinSpeech = NULL;
}
