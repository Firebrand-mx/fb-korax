
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

class KRootWindow:public KWindow
{
	DECLARE_CLASS(KRootWindow, KWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KRootWindow);

	void Init(KWindow *AParent) { gi.Error("Root cannot have a parent"); }
	virtual void Init(void);

	void PaintWindows(KCanvas *Canvas);
};

extern KRootWindow *GRootWindow;
