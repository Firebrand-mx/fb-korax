
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
	bIsSensitive = true;
	Font = KCanvas::SmallFont;
}

//==========================================================================
//
//	KWindow::Init
//
//==========================================================================

void KWindow::Init(KWindow *AParent)
{
	WindowType = WIN_Normal;
	Parent = AParent;
	if (Parent)
	{
		Parent->AddChild(this);
	}
	WinGC = Spawn<KGC>();
	ClipTree();
	InitWindow();
	bIsInitialized = true;
}

//==========================================================================
//
//	KWindow::CleanUp
//
//==========================================================================

void KWindow::CleanUp(void)
{
}

//==========================================================================
//
//	KWindow::Destroy
//
//==========================================================================

void KWindow::Destroy()
{
	bBeingDestroyed = true;
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
//	KWindow::GetRootWindow
//
//==========================================================================

KRootWindow *KWindow::GetRootWindow(void)
{
	KWindow *win = this;
	while (win->WindowType < WIN_Root)
	{
		win = win->Parent;
	}
	return (KRootWindow *)win;
}

//==========================================================================
//
//	KWindow::GetModalWindow
//
//==========================================================================

KModalWindow *KWindow::GetModalWindow(void)
{
	KWindow *win = this;
	while (win->WindowType < WIN_Modal)
	{
		win = win->Parent;
	}
	return (KModalWindow *)win;
}

//==========================================================================
//
//	KWindow::GetParent
//
//==========================================================================

KWindow *KWindow::GetParent(void)
{
	return Parent;
}

//==========================================================================
//
//	KWindow::GetBottomChild
//
//==========================================================================

KWindow *KWindow::GetBottomChild(bool bVisibleOnly)
{
	KWindow *win = FirstChild;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->NextSibling;
		}
	}
	return win;
}

//==========================================================================
//
//	KWindow::GetTopChild
//
//==========================================================================

KWindow *KWindow::GetTopChild(bool bVisibleOnly)
{
	KWindow *win = LastChild;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->PrevSibling;
		}
	}
	return win;
}

//==========================================================================
//
//	KWindow::GetLowerSibling
//
//==========================================================================

KWindow *KWindow::GetLowerSibling(bool bVisibleOnly)
{
	KWindow *win = PrevSibling;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->PrevSibling;
		}
	}
	return win;
}

//==========================================================================
//
//	KWindow::GetHigherSibling
//
//==========================================================================

KWindow *KWindow::GetHigherSibling(bool bVisibleOnly)
{
	KWindow *win = NextSibling;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->NextSibling;
		}
	}
	return win;
}

//==========================================================================
//
//	KWindow::Raise
//
//==========================================================================

void KWindow::Raise(void)
{
	if (!Parent)
	{
		gi.Error("Can't raise root window");
	}
	if (Parent->LastChild == this)
	{
		//	Already there
		return;
	}
	//	Unlink from current location
	if (PrevSibling)
	{
		PrevSibling->NextSibling = NextSibling;
	}
	else
	{
		Parent->FirstChild = NextSibling;
	}
	NextSibling->PrevSibling = PrevSibling;
	//	Link on top
	PrevSibling = Parent->LastChild;
	NextSibling = NULL;
	Parent->LastChild->NextSibling = this;
	Parent->LastChild = this;
}

//==========================================================================
//
//	KWindow::Lower
//
//==========================================================================

void KWindow::Lower(void)
{
	if (!Parent)
	{
		gi.Error("Can't lower root window");
	}
	if (Parent->FirstChild == this)
	{
		//	Already there
		return;
	}
	//	Unlink from current location
	PrevSibling->NextSibling = NextSibling;
	if (NextSibling)
	{
		NextSibling->PrevSibling = PrevSibling;
	}
	else
	{
		Parent->LastChild = PrevSibling;
	}
	//	Link on bottom
	PrevSibling = NULL;
	NextSibling = Parent->FirstChild;
	Parent->FirstChild->PrevSibling = this;
	Parent->FirstChild = this;
}

//==========================================================================
//
//	KWindow::SetVisibility
//
//==========================================================================

void KWindow::SetVisibility(bool NewVisibility)
{
	if (bIsVisible != NewVisibility)
	{
		bIsVisible = NewVisibility;
		VisibilityChanged(NewVisibility);
	}
}

//==========================================================================
//
//	KWindow::SetSensitivity
//
//==========================================================================

void KWindow::SetSensitivity(bool NewSensitivity)
{
	if (bIsSensitive != NewSensitivity)
	{
		bIsSensitive = NewSensitivity;
		SensitivityChanged(NewSensitivity);
	}
}

//==========================================================================
//
//	KWindow::SetFont
//
//==========================================================================

void KWindow::SetFont(KFont *NewFont)
{
	if (NewFont)
	{
		Font = NewFont;
	}
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
	for (KWindow *w = this; w; w = w->Parent)
	{
		w->DescendantAdded(NewChild);
	}
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
	for (KWindow *w = this; w; w = w->Parent)
	{
		w->DescendantRemoved(InChild);
	}
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
