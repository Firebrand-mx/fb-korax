
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// ----------------------------------------------------------------------
// EMove - Movement direction

enum EMove
{
	MOVE_Left,
	MOVE_Right,
	MOVE_Up,
	MOVE_Down
};

// ----------------------------------------------------------------------
//	EWinType - used as a quick way to distinguish window types

enum EWinType
{
	WIN_Normal,
	WIN_Modal,
	WIN_Root
};

// ----------------------------------------------------------------------
//	NewWindow() - Macro to create a new window instance

#define NewWindow(Type, Parent) \
	(Type *)KWindow::StaticCreateWindow(Type::StaticClass(), Parent)

// ----------------------------------------------------------------------
//	KWindow - Base class for all UI objects.

class KWindow:public KObject
{
	DECLARE_CLASS(KWindow, KObject, 0);

	friend class KModalWindow;
	friend class KRootWindow;

private:
	// Quick class reference
	byte WindowType;                // Contains window type

	KClipRect ClipRect;

	KGC *WinGC;

	// Traversal positions (used only when bIsSelectable is True)
	int RowMajorIndex;			// Index into tab group's row-major sorted list
	int ColMajorIndex;			// Index into tab group's column-major sorted list

	// Relatives
	KWindow *Parent;			// Parent window; NULL if this is root
	KWindow *FirstChild;		// "Lowest" child (first one drawn)
	KWindow *LastChild;			// "Highest" child (last one drawn)
	KWindow *PrevSibling;		// Next "lowest" sibling (previous one drawn)
	KWindow *NextSibling;		// Next "highest" sibling (next one drawn)

public:
	float X;
	float Y;
	float Width;
	float Height;

	bool bIsVisible;			// True if the window is visible1
	bool bIsSensitive;			// True if the window can take input
	bool bIsSelectable;			// True if the window can have keyboard focus
	bool bIsInitialized;		// True if the window has been initialized
	bool bBeingDestroyed;		// True if this window is going bye-bye

	FColor TextColor;
	KFont *Font;
	FColor TileColor;

	//
	// KWindow interface
	//

	// Structors
	KWindow();
	virtual void Init(KWindow *AParent);
	virtual void CleanUp(void);
	virtual void Destroy();

	// Ancestral routines
	KRootWindow *GetRootWindow(void);
	KModalWindow *GetModalWindow(void);
	KWindow *GetParent(void);

	// Child routines
	KWindow *GetBottomChild(bool bVisibleOnly = true);
	KWindow *GetTopChild(bool bVisibleOnly = true);

	// Sibling routines
	KWindow *GetLowerSibling(bool bVisibleOnly = true);
	KWindow *GetHigherSibling(bool bVisibleOnly = true);

	// Routines which change order of siblings
	void Raise(void);
	void Lower(void);

	// Visibility routines
	void SetVisibility(bool NewVisibility);
	void Show(void) { SetVisibility(true); }
	void Hide(void) { SetVisibility(false); }
	bool IsVisible(bool bRecurse = true)
	{
		if (bRecurse)
		{
			KWindow *pParent = this;
			while (pParent)
			{
				if (!pParent->bIsVisible)
					break;
				pParent = pParent->Parent;
			}
			return (pParent ? false : true);
		}
		else
			return bIsVisible;
	}

	// Sensitivity routines
	void SetSensitivity(bool NewSensitivity);
	void Enable(void) { SetSensitivity(true); }
	void Disable(void) { SetSensitivity(false); }
	bool IsSensitive(bool bRecurse = true)
	{
		if (bRecurse)
		{
			KWindow *pParent = this;
			while (pParent)
			{
				if (!pParent->bIsSensitive)
					break;
				pParent = pParent->Parent;
			}
			return (pParent ? false : true);
		}
		else
			return bIsSensitive;
	}

	// Selectability routines
	void SetSelectability(bool NewSelectability);
	bool IsSelectable(void) { return bIsSelectable; }
	//bool IsTraversable(bool bCheckModal = true);
	bool IsFocusWindow(void);

	// Default font color calls
	void SetTextColor(FColor newColor);
	FColor GetTextColor(void);

	// Routines to set the default font
	void SetFont(KFont *NewFont);

	// Tile color calls
	void SetTileColor(FColor newColor);
	FColor GetTileColor(void);

	void Move(float NewX, float NewY);
	void SetPos(float NewX, float NewY)
	{
		Move(NewX, NewY);
	}
	void Resize(float NewWidth, float NewHeight);
	void SetSize(float NewWidth, float NewHeight)
	{
		Resize(NewWidth, NewHeight);
	}

	// Keyboard traversal routines
	KWindow *MoveFocus(EMove moveDir, bool bForceWrap = false);
	KWindow *MoveFocusLeft(void)  { return MoveFocus(MOVE_Left);  }
	KWindow *MoveFocusRight(void) { return MoveFocus(MOVE_Right); }
	KWindow *MoveFocusUp(void)    { return MoveFocus(MOVE_Up);    }
	KWindow *MoveFocusDown(void)  { return MoveFocus(MOVE_Down);  }

	// Straight focus setting/getting routines
	bool SetFocusWindow(KWindow *pNewFocusWindow);
	KWindow *GetFocusWindow(void);

	// Slayer of innocent children
	void DestroyAllChildren(void) { KillAllChildren(); }

	//
	//	Event callbacks
	//

	virtual void InitWindow() { }

	virtual void DrawWindow(KGC *) { }
	virtual void PostDrawWindow(KGC *) { }

	virtual void ChildAdded(KWindow *) { }
	virtual void ChildRemoved(KWindow *) { }
	virtual void DescendantAdded(KWindow *) { }
	virtual void DescendantRemoved(KWindow *) { }

	virtual void ConfigurationChanged(void) { }
	virtual void VisibilityChanged(bool bNewVisibility) { }
	virtual void SensitivityChanged(bool bNewSensitivity) { }

	// Called when keyboard input focus is going to this window
	virtual void  FocusEnteredWindow(void) { }

	// Called when keyboard input focus is no longer going to this window
	virtual void  FocusLeftWindow(void) { }

	// Called when keyboard input focus is going to a descendant
	virtual void FocusEnteredDescendant(KWindow *EnterWindow) { }

	// Called when keyboard input focus is no longer going to a descendant
	virtual void FocusLeftDescendant(KWindow *LeaveWindow) { }

	virtual bool RawInputEvent(event_t *event) { return false; }
	virtual bool KeyPressed(int key) { return false; }

	virtual bool ButtonActivated(KWindow *button) { return false; }

	static KTexture *FindTexture(char *Name)
	{
		return KCanvas::FindTexture(Name);
	}

	static KWindow *StaticCreateWindow(KClass *InClass, KWindow *InParent);

protected:
	// Anybody who kills children can't be all bad...
	void KillAllChildren();

private:
	// Child adopter/orphaner routines
	void AddChild(KWindow *);
	void RemoveChild(KWindow *);

	// Insures that the focus window can still have focus
	void CheckFocusWindow(void);

	// Sets clipping on an entire window tree
	void ClipTree();

	// Draws all windows
	void DrawTree(KCanvas *);
};
