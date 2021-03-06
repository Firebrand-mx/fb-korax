
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

enum EHAlign
{
	HALIGN_Left,
	HALIGN_Center,
	HALIGN_Right,
	HALIGN_Full
};

enum EVAlign
{
	VALIGN_Top,
	VALIGN_Center,
	VALIGN_Bottom,
	VALIGN_Full
};

struct FColor
{
	byte r;
	byte g;
	byte b;
	byte a;

	FColor()
	{ }
	FColor(byte InR, byte InG, byte InB, byte InA = 255)
	: r(InR), g(InG), b(InB), a(InA)
	{ }
};

//==========================================================================
//
//	KTexture
//
//==========================================================================

struct KTexture
{
	char Name[12];
	int LumpNum;

	int Width;
	int Height;
	int XOffset;
	int YOffset;
};

//==========================================================================
//
//	KFont
//
//==========================================================================

class KFont
{
	//	Friends
	friend class KCanvas;
	friend class KGC;

private:
	KTexture *Chars[96];
	int SpaceWidth;
	int SpaceHeight;
public:
	KFont(char *Name, int InSpaceWidth, int InSpaceHeight);
	int TextWidth(const char *Text);
};

//==========================================================================
//
//	KCanvas
//
//==========================================================================

class KCanvas:public KObject
{
	DECLARE_CLASS(KCanvas, KObject, 0);

private:
	static KTexture **Textures;
	static int NumTextures;

public:
	KCanvas();

	int OrgX, OrgY;		//	Origin for drawing.
	KFont *Font;		//	Current font.

	static KFont *SmallFont;
	static KFont *YellowFont;
	static KFont *BigFont;

	static void StaticInit(void);
	static KTexture *FindTexture(char *Name);

	void SetColor(FColor &c)
	{
		gl.Color4ub(c.r, c.g, c.b, c.a);
	}

	void DrawPatch1(int x, int y, int lumpnum)
	{
		GL_DrawPatch(x + OrgX, y + OrgY, lumpnum);
	}

	void DrawFuzzPatch(int x, int y, int lumpnum)
	{
		GL_DrawFuzzPatch(x + OrgX, y + OrgY, lumpnum);
	}

	void DrawPatch(int x, int y, int lumpnum)
	{
		GL_DrawPatch_CS(x + OrgX, y + OrgY, lumpnum);
	}

	void DrawRawScreen(int lumpnum)
	{
		GL_DrawRawScreen(lumpnum);
	}

	void DrawRect(float x, float y, float w, float h, 
		float r, float g, float b, float a)
	{
		GL_DrawRect(x + OrgX, y + OrgY, w, h, r, g, b, a);
	}

	void DrawIcon(int x, int y, KTexture *Icon)
	{
		GL_DrawPatch_CS(x + OrgX, y + OrgY, Icon->LumpNum);
	}
	void DrawTile(KTexture *Texture,
		float x1, float y1, float x2, float y2,
		float s1, float t1, float s2, float t2);

	void SetOrigin(int NewX, int NewY)
	{
		OrgX = NewX;
		OrgY = NewY;
	}
	void SetFont(KFont *NewFont)
	{
		Font = NewFont;
	}

	void DrawText(int x, int y, const char *Text);
	int TextWidth(const char *Text)
	{
		return Font->TextWidth(Text);
	}
};

extern KCanvas *GCanvas;
