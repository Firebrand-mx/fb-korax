
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
	WindowType = WIN_Normal;
	bIsVisible = true;
	bIsSensitive = true;
	TileColor = FColor(255, 255, 255, 255);
	TextColor = FColor(255, 255, 255, 255);
	Font = KCanvas::SmallFont;
}

//==========================================================================
//
//	KWindow::Init
//
//==========================================================================

void KWindow::Init(KWindow *AParent)
{
	guard(KWindow::Init);
	Parent = AParent;
	if (Parent)
	{
		Parent->AddChild(this);
	}
	WinGC = Spawn<KGC>();
	ClipTree();
	if (bIsSelectable)
		GetModalWindow()->AddWindowToTables(this);
	InitWindow();
	bIsInitialized = true;
	unguard;
}

//==========================================================================
//
//	KWindow::CleanUp
//
//==========================================================================

void KWindow::CleanUp(void)
{
	if (bIsSelectable)
		GetModalWindow()->RemoveWindowFromTables(this);
}

//==========================================================================
//
//	KWindow::Destroy
//
//==========================================================================

void KWindow::Destroy()
{
	guard(KWindow::Destroy);
	bBeingDestroyed = true;
	KillAllChildren();
	CleanUp();
	if (Parent)
	{
		Parent->RemoveChild(this);
	}
	if (WinGC)
		WinGC->Destroy();
	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	KWindow::GetRootWindow
//
//==========================================================================

KRootWindow *KWindow::GetRootWindow(void)
{
	guard(KWindow::GetRootWindow);
	KWindow *win = this;
	while (win->WindowType < WIN_Root)
	{
		win = win->Parent;
	}
	return (KRootWindow *)win;
	unguard;
}

//==========================================================================
//
//	KWindow::GetModalWindow
//
//==========================================================================

KModalWindow *KWindow::GetModalWindow(void)
{
	guard(KWindow::GetModalWindow);
	KWindow *win = this;
	while (win->WindowType < WIN_Modal)
	{
		win = win->Parent;
	}
	return (KModalWindow *)win;
	unguard;
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
	guard(KWindow::GetBottomChild);
	KWindow *win = FirstChild;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->NextSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	KWindow::GetTopChild
//
//==========================================================================

KWindow *KWindow::GetTopChild(bool bVisibleOnly)
{
	guard(KWindow::GetTopChild);
	KWindow *win = LastChild;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->PrevSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	KWindow::GetLowerSibling
//
//==========================================================================

KWindow *KWindow::GetLowerSibling(bool bVisibleOnly)
{
	guard(KWindow::GetLowerSibling);
	KWindow *win = PrevSibling;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->PrevSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	KWindow::GetHigherSibling
//
//==========================================================================

KWindow *KWindow::GetHigherSibling(bool bVisibleOnly)
{
	guard(KWindow::GetHigherSibling);
	KWindow *win = NextSibling;
	if (bVisibleOnly)
	{
		while (win && !win->bIsVisible)
		{
			win = win->NextSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	KWindow::Raise
//
//==========================================================================

void KWindow::Raise(void)
{
	guard(KWindow::Raise);
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
	unguard;
}

//==========================================================================
//
//	KWindow::Lower
//
//==========================================================================

void KWindow::Lower(void)
{
	guard(KWindow::Lower);
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
	unguard;
}

//==========================================================================
//
//	KWindow::SetVisibility
//
//==========================================================================

void KWindow::SetVisibility(bool NewVisibility)
{
	guard(KWindow::SetVisibility);
	if (bIsVisible != NewVisibility)
	{
		bIsVisible = NewVisibility;
		VisibilityChanged(NewVisibility);
	}
	unguard;
}

//==========================================================================
//
//	KWindow::SetSensitivity
//
//==========================================================================

void KWindow::SetSensitivity(bool NewSensitivity)
{
	guard(KWindow::SetSensitivity);
	if (bIsSensitive != NewSensitivity)
	{
		bIsSensitive = NewSensitivity;
		SensitivityChanged(NewSensitivity);
	}
	unguard;
}

//==========================================================================
//
//	KWindow::SetSelectability
//
//==========================================================================

void KWindow::SetSelectability(bool NewSelectability)
{
	guard(KWindow::SetSelectability);
	if (bIsSelectable != NewSelectability)
	{
		bIsSelectable = NewSelectability;
		if (bIsSelectable)
			GetModalWindow()->AddWindowToTables(this);
		else
			GetModalWindow()->RemoveWindowFromTables(this);
	}
	unguard;
}

//==========================================================================
//
//	KWindow::IsFocusWindow
//
//==========================================================================

bool KWindow::IsFocusWindow(void)
{
	guard(KWindow::IsFocusWindow);
	return GetRootWindow()->FocusWindow == this;
	unguard;
}

//==========================================================================
//
//	KWindow::SetTextColor
//
//==========================================================================

void KWindow::SetTextColor(FColor newColor)
{
	guard(KWindow::SetTextColor);
	TextColor = newColor;
	unguard;
}

//==========================================================================
//
//	KWindow::GetTextColor
//
//==========================================================================

FColor KWindow::GetTextColor(void)
{
	guard(KWindow::GetTextColor);
	return TextColor;
	unguard;
}

//==========================================================================
//
//	KWindow::SetFont
//
//==========================================================================

void KWindow::SetFont(KFont *NewFont)
{
	guard(KWindow::SetFont);
	if (NewFont)
	{
		Font = NewFont;
	}
	unguard;
}

//==========================================================================
//
//	KWindow::SetTileColor
//
//==========================================================================

void KWindow::SetTileColor(FColor newColor)
{
	guard(KWindow::SetTileColor);
	TileColor = newColor;
	unguard;
}

//==========================================================================
//
//	KWindow::GetTileColor
//
//==========================================================================

FColor KWindow::GetTileColor(void)
{
	guard(KWindow::GetTileColor);
	return TileColor;
	unguard;
}

//==========================================================================
//
//	KWindow::Move
//
//==========================================================================

void KWindow::Move(float NewX, float NewY)
{
	guard(KWindow::Move);
	X = NewX;
	Y = NewY;
	ClipTree();
	GetModalWindow()->ResortWindowTables();
	unguard;
}

//==========================================================================
//
//	KWindow::Resize
//
//==========================================================================

void KWindow::Resize(float NewWidth, float NewHeight)
{
	guard(KWindow::Resize);
	Width = NewWidth;
	Height = NewHeight;
	ClipTree();
	unguard;
}

//==========================================================================
//
//	KWindow::AddChild
//
//==========================================================================

void KWindow::AddChild(KWindow *NewChild)
{
	guard(KWindow::AddChild);
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
	unguard;
}

//==========================================================================
//
//	KWindow::RemoveChild
//
//==========================================================================

void KWindow::RemoveChild(KWindow *InChild)
{
	guard(KWindow::RemoveChild);
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
	unguard;
}

//==========================================================================
//
//	KWindow::KillAllChildren
//
//==========================================================================

void KWindow::KillAllChildren()
{
	guard(KWindow::KillAllChildren);
	while (FirstChild)
	{
		FirstChild->Destroy();
	}
	unguard;
}

//==========================================================================
//
//	KWindow::DrawTree
//
//==========================================================================

void KWindow::DrawTree(KCanvas *Canvas)
{
	guard(KWindow::DrawTree);
	if (!bIsVisible || !ClipRect.HasArea())
	{
		//	Nowhere to draw.
		return;
	}
	WinGC->SetCanvas(Canvas);
	WinGC->SetClipRect(ClipRect);
	WinGC->SetTileColor(TileColor);
	WinGC->SetTextColor(TextColor);
	WinGC->SetFont(Font);
	DrawWindow(WinGC);
	for (KWindow *c = FirstChild; c; c = c->NextSibling)
	{
		c->DrawTree(Canvas);
	}
	WinGC->SetClipRect(ClipRect);
	PostDrawWindow(WinGC);
	unguard;
}

//==========================================================================
//
//	KWindow::ClipTree
//
//==========================================================================

void KWindow::ClipTree()
{
	guard(KWindow::ClipTree);
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
	unguard;
}

//==========================================================================
//
//	KWindow::StaticCreateWindow
//
//==========================================================================

KWindow *KWindow::StaticCreateWindow(KClass *InClass, KWindow *InParent)
{
	guard(KWindow::StaticCreateWindow);
	KWindow *Win;

	Win = (KWindow *)StaticSpawnObject(InClass);
	Win->Init(InParent);
	return Win;
	unguard;
}
