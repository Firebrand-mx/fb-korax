
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

//==========================================================================
//
//	KWindow::KWindow
//
//==========================================================================

KWindow::KWindow(KWindow *InParent) : 
	Parent(InParent), FirstChild(NULL), LastChild(NULL), 
	PrevSibling(NULL), NextSibling(NULL)
{
	if (Parent)
	{
		Parent->AddChild(this);
	}
}

//==========================================================================
//
//	KWindow::~KWindow
//
//==========================================================================

KWindow::~KWindow()
{
	if (Parent)
	{
		Parent->RemoveChild(this);
	}
}

//==========================================================================
//
//	KWindow::AddChild
//
//==========================================================================

void KWindow::AddChild(KWindow *NewChild)
{
	NewChild->PrevSibling = LastChild;
	NewChild->NextSibling = NULL;
	if (LastChild)
	{
		LastChild->NextSibling = NewChild;
	}
	else
	{
		FirstChild = NewChild;
	}
	LastChild = NewChild;
}

//==========================================================================
//
//	KWindow::RemoveChild
//
//==========================================================================

void KWindow::RemoveChild(KWindow *InChild)
{
	if (InChild->PrevSibling)
	{
		InChild->PrevSibling->NextSibling = InChild->NextSibling;
	}
	else
	{
		FirstChild = InChild->NextSibling;
	}
	if (InChild->NextSibling)
	{
		InChild->NextSibling->PrevSibling = InChild->PrevSibling;
	}
	else
	{
		LastChild = InChild->PrevSibling;
	}
	InChild->PrevSibling = NULL;
	InChild->NextSibling = NULL;
	InChild->Parent = NULL;
}

//==========================================================================
//
//	KWindow::DrawTree
//
//==========================================================================

void KWindow::DrawTree(KCanvas *Canvas)
{
	KGC dummyGC;

	DrawWindow(&dummyGC);
	for (KWindow *c = FirstChild; c; c = c->NextSibling)
	{
		c->DrawTree(Canvas);
	}
	PostDrawWindow(&dummyGC);
}
