
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

IMPLEMENT_CLASS(KModalWindow);

//==========================================================================
//
//	KModalWindow::KModalWindow
//
//==========================================================================

KModalWindow::KModalWindow()
{
	WindowType = WIN_Modal;
	bIsSelectable = true;
	bWrapFocus = true;
	PreferredFocus = this;
}

//==========================================================================
//
//	KModalWindow::CleanUp
//
//==========================================================================

void KModalWindow::CleanUp(void)
{
	Super::CleanUp();
}

//==========================================================================
//
//	KModalWindow::AddWindowToTables
//
//==========================================================================

void KModalWindow::AddWindowToTables(KWindow *pWindow)
{
	guard(KModalWindow::AddWindowToTables);
	if (WindowListCount >= 256)
	{
		I_Error("Too many selectable windows");
	}
	RowMajorWindowList[WindowListCount] = pWindow;
	ColMajorWindowList[WindowListCount] = pWindow;
	WindowListCount++;
	ResortWindowTables();
	if (PreferredFocus == this)
	{
		PreferredFocus = pWindow;
	}
	unguard;
}

//==========================================================================
//
//	KModalWindow::RemoveWindowFromTables
//
//==========================================================================

void KModalWindow::RemoveWindowFromTables(KWindow *pWindow)
{
	guard(KModalWindow::RemoveWindowFromTables);
	int i;

	WindowListCount--;
	for (i = pWindow->RowMajorIndex; i < WindowListCount; i++)
	{
		RowMajorWindowList[i] = RowMajorWindowList[i + 1];
		RowMajorWindowList[i]->RowMajorIndex = i;
	}
	for (i = pWindow->ColMajorIndex; i < WindowListCount; i++)
	{
		ColMajorWindowList[i] = ColMajorWindowList[i + 1];
		ColMajorWindowList[i]->ColMajorIndex = i;
	}
	if (PreferredFocus == pWindow)
	{
		if (WindowListCount)
			PreferredFocus = RowMajorWindowList[0];
		else
			PreferredFocus = this;
		if (IsCurrentModal())
		{
			GetRootWindow()->SetFocus(PreferredFocus);
		}
	}
	unguard;
}

//==========================================================================
//
//	KModalWindow::ResortWindowTables
//
//==========================================================================

void KModalWindow::ResortWindowTables(void)
{
	guard(KModalWindow::ResortWindowTables);
	int i;

	// Set indexes of the windows
	for (i = 0; i < WindowListCount; i++)
	{
		RowMajorWindowList[i]->RowMajorIndex = i;
		ColMajorWindowList[i]->ColMajorIndex = i;
	}
	unguard;
}

//==========================================================================
//
//	KModalWindow::IsCurrentModal
//
//==========================================================================

bool KModalWindow::IsCurrentModal(void)
{
	guard(KModalWindow::IsCurrentModal);
	return GetRootWindow()->GetCurrentModal() == this;
	unguard;
}

//==========================================================================
//
//
//
//==========================================================================
