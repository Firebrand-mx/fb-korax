
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

class KCanvas  
{
public:
	KCanvas();
	virtual ~KCanvas();

	int OrgX, OrgY;		//	Origin for drawing.

	void DrawPatch1(int x, int y, int lumpnum)
	{
		gi.GL_DrawPatch(x + OrgX, y + OrgY, lumpnum);
	}

	void DrawFuzzPatch(int x, int y, int lumpnum)
	{
		gi.GL_DrawFuzzPatch(x + OrgX, y + OrgY, lumpnum);
	}

	void DrawPatch(int x, int y, int lumpnum)
	{
		gi.GL_DrawPatchCS(x + OrgX, y + OrgY, lumpnum);
	}

	void DrawRawScreen(int lumpnum)
	{
		gi.GL_DrawRawScreen(lumpnum);
	}

	void DrawRect(float x, float y, float w, float h, 
		float r, float g, float b, float a)
	{
		gi.GL_DrawRect(x + OrgX, y + OrgY, w, h, r, g, b, a);
	}

	void SetOrigin(int NewX, int NewY)
	{
		OrgX = NewX;
		OrgY = NewY;
	}
};

extern KCanvas *GCanvas;
