
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

//
//	Base class for all UI objects.
//

#define NewWindow(Type, Parent) \
	(Type *)KWindow::StaticCreateWindow(Type::StaticClass(), Parent)

class KWindow:public KObject
{
	friend class KRootWindow;

	DECLARE_CLASS(KWindow, KObject, 0);

	// Relatives
	KWindow *Parent;			// Parent window; NULL if this is root
	KWindow *FirstChild;		// "Lowest" child (first one drawn)
	KWindow *LastChild;			// "Highest" child (last one drawn)
	KWindow *PrevSibling;		// Next "lowest" sibling (previous one drawn)
	KWindow *NextSibling;		// Next "highest" sibling (next one drawn)

	float	X;
	float	Y;
	float	Width;
	float	Height;

	bool	bIsVisible;

	KClipRect ClipRect;

	KGC		*WinGC;

	KFont	*Font;

	// Structors
	KWindow();
	virtual void Init(KWindow *AParent);
	virtual void Destroy();

	// KWindow interface
	virtual void InitWindow() { }

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

	virtual void DrawWindow(KGC *) { }
	virtual void PostDrawWindow(KGC *) { }

	virtual void ChildAdded(KWindow *) { }
	virtual void ChildRemoved(KWindow *) { }

	virtual bool KeyPressed(int key) { return false; }

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
