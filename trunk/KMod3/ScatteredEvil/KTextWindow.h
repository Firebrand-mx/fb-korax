
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

class KTextWindow:public KWindow
{
	DECLARE_CLASS(KTextWindow, KWindow, 0);

	KTextWindow();
	void CleanUp(void);

protected:
	EHAlign HAlign;
	EVAlign VAlign;
	char *Text;

public:
	virtual void SetText(const char *NewText);
	virtual void AppendText(const char *newText);
	const char *GetText(void) { return Text; }

	void SetTextAlignments(EHAlign newHAlign, EVAlign newVAlign);

	void DrawWindow(KGC *gc);
};
