
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

	friend class KWindow;
	friend class KRootWindow;

	// Structors
	KModalWindow(void);
	void CleanUp(void);

	// Sorted lists of selectable children
	KWindow *RowMajorWindowList[64];
	KWindow *ColMajorWindowList[64];
	int WindowListCount;

	// Option to enable wrapping when moving focus
	bool bWrapFocus;

	// Absolute position of first child in tab group
	//float firstAbsX;
	//float firstAbsY;

	// KModalWindow interface
	void AddWindowToTables(KWindow *pWindow);
	void RemoveWindowFromTables(KWindow *pWindow);
	void ResortWindowTables(void);

	bool IsCurrentModal(void);

/*
	// KWindow interface callbacks
	void ConfigurationChanged(void);
	void ParentRequestedPreferredSize(bool bWidthSpecified, float &preferredWidth,
									  bool bHeightSpecified, float &preferredHeight);
	void ChildRequestedVisibilityChange(KWindow *pChild, bool bNewVisibility);
	bool ChildRequestedReconfiguration(KWindow *pChild);
*/

/*
private:
	KWindow *PreferredFocus;

public:
	// KWindow interface callbacks
	void VisibilityChanged(bool bNewVisibility);
	void DescendantRemoved(KWindow *pDescendant);
	void Tick(float deltaSeconds);
	bool KeyPressed(int key);
*/
};
