
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
	ClipTree();
	WinGC = Spawn<KGC>();
	InitWindow();
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
