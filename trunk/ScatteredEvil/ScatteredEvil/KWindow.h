
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// ----------------------------------------------------------------------
//	EWinType - used as a quick way to distinguish window types

enum EWinType
{
	WIN_Normal   = 0,
	WIN_TabGroup = 1,
	WIN_Modal    = 2,
	WIN_Root     = 3
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

	friend class KRootWindow;

private:
	// Quick class reference
	byte WindowType;                // Contains window type

	KClipRect ClipRect;

	KGC *WinGC;

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

	bool bIsVisible;
	bool bIsSensitive;

	KFont *Font;

	//
	// KWindow interface
	//

	// Structors
	KWindow();
	virtual void Init(KWindow *AParent);
	virtual void Destroy();

	// Ancestral routines
	KRootWindow *GetRootWindow(void);
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

	void SetFont(KFont *NewFont);

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

	//
	//	Event callbacks
	//

	virtual void InitWindow() { }

	virtual void DrawWindow(KGC *) { }
	virtual void PostDrawWindow(KGC *) { }

	virtual void ChildAdded(KWindow *) { }
	virtual void ChildRemoved(KWindow *) { }

	virtual void VisibilityChanged(bool bNewVisibility) { }
	virtual void SensitivityChanged(bool bNewSensitivity) { }

	virtual bool RawInputEvent(event_t *event) { return false; }
	virtual bool KeyPressed(int key) { return false; }

	virtual bool ButtonActivated(KWindow *button) { return false; }

	static KTexture *FindTexture(char *Name)
	{
		return KCanvas::FindTexture(Name);
	}

	static KWindow *StaticCreateWindow(KClass *InClass, KWindow *InParent);

protected:
	void KillAllChildren();

private:

	void AddChild(KWindow *);
	void RemoveChild(KWindow *);

	void DrawTree(KCanvas *);

	void ClipTree();
};
