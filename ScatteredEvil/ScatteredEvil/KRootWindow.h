
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

class KRootWindow:public KModalWindow
{
	DECLARE_CLASS(KRootWindow, KModalWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KRootWindow);

	friend class KWindow;
	friend class KModalWindow;

	// Input windows
	KWindow *FocusWindow;		// Recipient window for all keyboard events

	void Init(KWindow *AParent) { I_Error("Root cannot have a parent"); }
	virtual void Init(void);

	// KRootWindow interface
	KModalWindow *GetCurrentModal(void)
	{
		KWindow *pChild = GetTopChild();
		while (pChild)
		{
			if (pChild->WindowType >= WIN_Modal)
				break;
			pChild = pChild->GetLowerSibling();
		}
		if (!pChild)
			pChild = this;
		return (KModalWindow *)pChild;
	}

	void SetFocus(KWindow *NewFocus);

	void DescendantRemoved(KWindow *Descendant);

	void PaintWindows(KCanvas *Canvas);
};

extern KRootWindow *GRootWindow;
