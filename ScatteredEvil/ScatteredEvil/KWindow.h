
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

class KWindow  
{
public:
	// Relatives
	KWindow *Parent;			// Parent window; NULL if this is root
	KWindow *FirstChild;		// "Lowest" child (first one drawn)
	KWindow *LastChild;			// "Highest" child (last one drawn)
	KWindow *PrevSibling;		// Next "lowest" sibling (previous one drawn)
	KWindow *NextSibling;		// Next "highest" sibling (next one drawn)

	// Structors
	KWindow(KWindow *);
	virtual ~KWindow();

	// KWindow interface
	virtual void DrawWindow(KGC *) { }
	virtual void PostDrawWindow(KGC *) { }

	void DrawTree(KCanvas *);

private:

	void AddChild(KWindow *);
	void RemoveChild(KWindow *);
};
