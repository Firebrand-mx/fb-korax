#include "h2def.h"
#include "con_local.h"

IMPLEMENT_CLASS(KFilledRectWindow);
IMPLEMENT_CLASS(KConChoiceWindow);
IMPLEMENT_CLASS(KConWindow);
IMPLEMENT_CLASS(KConWindowFirst);

//==========================================================================
//
//	KFilledRectWindow::DrawWindow
//
//==========================================================================

void KFilledRectWindow::DrawWindow(KGC *gc)
{
	gl.Disable(DGL_TEXTURING);
	gc->GetCanvas()->DrawRect(0, 0, Width, Height, TileColor.r / 255.0,
		TileColor.g / 255.0, TileColor.b / 255.0, TileColor.a / 255.0);
	gl.Enable(DGL_TEXTURING);
}

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

	UpperConWindow = NewWindow(KFilledRectWindow, this);
	UpperConWindow->SetSize(Width, 50);
	UpperConWindow->SetTileColor(FColor(0, 0, 0, 255));

	LowerConWindow = NewWindow(KFilledRectWindow, this);
	LowerConWindow->SetPos(0, Height - 100);
	LowerConWindow->SetSize(Width, 100);
	LowerConWindow->SetTileColor(FColor(0, 0, 0, 255));

	R_SetViewSize(0, 50, 640, 330);
}

//==========================================================================
//
//	KConWindow::CleanUp
//
//==========================================================================

extern int screenblocks;

void KConWindow::CleanUp(void)
{
	Super::CleanUp();
	R_SetViewSize(screenblocks);
}

//==========================================================================
//
//	KConWindow::Clear
//
//==========================================================================

void KConWindow::Clear(void)
{
	LowerConWindow->DestroyAllChildren();
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
	KConChoiceWindow *newChoice = NewWindow(KConChoiceWindow, LowerConWindow);
	newChoice->SetPos(0, 10 + 10 * NumChoices);
	newChoice->SetSize(640, 10);
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
		WinSpeech = NewWindow(KTextWindow, LowerConWindow);
		WinSpeech->SetPos(0, 0);
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
	LowerConWindow = NewWindow(KFilledRectWindow, this);
	LowerConWindow->SetSize(Width, 10);
	LowerConWindow->SetTileColor(FColor(0, 127, 0, 127));
}

//==========================================================================
//
//	KConWindowFirst::Clear
//
//==========================================================================

void KConWindowFirst::Clear(void)
{
	LowerConWindow->DestroyAllChildren();
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
		WinSpeech = NewWindow(KTextWindow, LowerConWindow);
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
