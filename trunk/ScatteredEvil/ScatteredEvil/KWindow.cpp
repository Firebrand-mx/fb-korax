
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

IMPLEMENT_CLASS(KWindow);

//==========================================================================
//
//	KWindow::KWindow
//
//==========================================================================

KWindow::KWindow()
{
	bIsVisible = true;
	Font = KCanvas::SmallFont;
}

//==========================================================================
//
//
//
//==========================================================================

void KWindow::Init(KWindow *AParent)
{
	Parent = AParent;
	if (Parent)
	{
		Parent->AddChild(this);
	}
	WinGC = Spawn<KGC>();
	ClipTree();
	InitWindow();
}

//==========================================================================
//
//	KWindow::Destroy
//
//==========================================================================

void KWindow::Destroy()
{
	KillAllChildren();
	if (Parent)
	{
		Parent->RemoveChild(this);
	}
	if (WinGC)
		WinGC->Destroy();
	Super::Destroy();
}

//==========================================================================
//
//	KWindow::SetVisibility
//
//==========================================================================

void KWindow::SetVisibility(bool NewVisibility)
{
	bIsVisible = NewVisibility;
}

//==========================================================================
//
//	KWindow::Move
//
//==========================================================================

void KWindow::Move(float NewX, float NewY)
{
	X = NewX;
	Y = NewY;
	ClipTree();
}

//==========================================================================
//
//	KWindow::Resize
//
//==========================================================================

void KWindow::Resize(float NewWidth, float NewHeight)
{
	Width = NewWidth;
	Height = NewHeight;
	ClipTree();
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
	ChildAdded(NewChild);
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
	ChildRemoved(InChild);
}

//==========================================================================
//
//	KWindow::KillAllChildren
//
//==========================================================================

void KWindow::KillAllChildren()
{
	while (FirstChild)
	{
		FirstChild->Destroy();
	}
}

//==========================================================================
//
//	KWindow::DrawTree
//
//==========================================================================

void KWindow::DrawTree(KCanvas *Canvas)
{
	if (!bIsVisible || !ClipRect.HasArea())
	{
		//	Nowhere to draw.
		return;
	}
	WinGC->SetCanvas(Canvas);
	WinGC->SetClipRect(ClipRect);
	WinGC->SetFont(Font);
	DrawWindow(WinGC);
	for (KWindow *c = FirstChild; c; c = c->NextSibling)
	{
		c->DrawTree(Canvas);
	}
	WinGC->SetClipRect(ClipRect);
	PostDrawWindow(WinGC);
}

//==========================================================================
//
//	KWindow::ClipTree
//
//==========================================================================

void KWindow::ClipTree()
{
	if (Parent)
	{
		ClipRect = KClipRect(Parent->ClipRect.OriginX + X, 
			Parent->ClipRect.OriginY + Y, Width, Height);
		ClipRect.Intersect(Parent->ClipRect);
	}
	else
	{
		ClipRect = KClipRect(X, Y, Width, Height);
	}
	for (KWindow *c = FirstChild; c; c = c->NextSibling)
	{
		c->ClipTree();
	}
}

//==========================================================================
//
//	KWindow::StaticCreateWindow
//
//==========================================================================

KWindow *KWindow::StaticCreateWindow(KClass *InClass, KWindow *InParent)
{
	KWindow *Win;

	Win = (KWindow *)StaticSpawnObject(InClass);
	Win->Init(InParent);
	return Win;
}
