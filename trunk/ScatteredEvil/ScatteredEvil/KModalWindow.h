
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

class KModalWindow : public KWindow  
{
	DECLARE_CLASS(KModalWindow, KWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KModalWindow);

	friend class KWindow;
	friend class KRootWindow;

	void Init(KWindow *AParent);
	void CleanUp(void);
};
