
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

IMPLEMENT_CLASS(KRootWindow);

KRootWindow *GRootWindow;

//==========================================================================
//
//	KRootWindow::Init
//
//==========================================================================

void KRootWindow::Init(void)
{
	X = 0;
	Y = 0;
	Width = 640;
	Height = 480;
	Super::Init(NULL);
	WindowType = WIN_Root;
	FocusWindow = this;
}

//==========================================================================
//
//	KRootWindow::DescendantRemoved
//
//==========================================================================

void KRootWindow::SetFocus(KWindow *NewFocus)
{
	if (!NewFocus)
	{
		NewFocus = this;
	}
	if (FocusWindow)
	{
		FocusWindow->FocusLeftWindow();
		for (KWindow *w = FocusWindow->Parent; w; w = w->Parent)
		{
			w->FocusLeftDescendant(FocusWindow);
		}
	}
	FocusWindow = NewFocus;
	if (FocusWindow)
	{
		FocusWindow->FocusEnteredWindow();
		for (KWindow *w = FocusWindow->Parent; w; w = w->Parent)
		{
			w->FocusEnteredDescendant(FocusWindow);
		}
	}
}

//==========================================================================
//
//	KRootWindow::DescendantRemoved
//
//==========================================================================

void KRootWindow::DescendantRemoved(KWindow *Descendant)
{
	if (Descendant == FocusWindow)
	{
		SetFocus(this);
	}
	Super::DescendantRemoved(Descendant);
}

//==========================================================================
//
//	KRootWindow::PaintWindows
//
//==========================================================================

void KRootWindow::PaintWindows(KCanvas *Canvas)
{
	DrawTree(Canvas);
}
