
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

//==========================================================================
//
//	KClipRect - clipping rectangle class
//
//==========================================================================

struct KClipRect
{
	// Default constructor
	KClipRect()
	{ 
		OriginX    = 0;
		OriginY    = 0; 
		ClipX      = 0;
		ClipY      = 0; 
		ClipWidth  = 0;
		ClipHeight = 0;
	}

	// Other convenience constructors
	KClipRect(float NewClipX,     float NewClipY,
	          float NewClipWidth, float NewClipHeight)
	{
		OriginX    = NewClipX;
		OriginY    = NewClipY;

		ClipX      = 0;
		ClipY      = 0;
		ClipWidth  = NewClipWidth;
		ClipHeight = NewClipHeight;
	}

	KClipRect(float NewOriginX,   float NewOriginY,
	          float NewClipX,     float NewClipY,
	          float NewClipWidth, float NewClipHeight)
	{
		OriginX    = NewOriginX;
		OriginY    = NewOriginY;
		ClipX      = NewClipX;
		ClipY      = NewClipY;
		ClipWidth  = NewClipWidth;
		ClipHeight = NewClipHeight;
	}

	KClipRect(KClipRect &Rect1, KClipRect &Rect2)
	{
		OriginX    = Rect1.OriginX;
		OriginY    = Rect1.OriginY;
		ClipX      = Rect1.ClipX;
		ClipY      = Rect1.ClipY;
		ClipWidth  = Rect1.ClipWidth;
		ClipHeight = Rect1.ClipHeight;
		Intersect(Rect2);
	}

	// Public methods
	void SetOrigin(float NewOriginX, float NewOriginY)
	{
		ClipX   += (OriginX - NewOriginX);
		ClipY   += (OriginY - NewOriginY);
		OriginX =  NewOriginX;
		OriginY =  NewOriginY;
	}

	void MoveOrigin(float NewDeltaX, float NewDeltaY)
	{
		SetOrigin(OriginX + NewDeltaX, OriginY + NewDeltaY);
	}

	void Intersect(float NewClipX,     float NewClipY,
	               float NewClipWidth, float NewClipHeight)
	{
		KClipRect TempRect(NewClipX + OriginX, NewClipY + OriginY, 
			NewClipWidth, NewClipHeight);
		Intersect(TempRect);
	}

	void Intersect(KClipRect &NewRect)
	{
		float fromX1, fromY1;
		float fromX2, fromY2;
		float toX1,   toY1;
		float toX2,   toY2;

		// Convert everything to absolute coordinates
		fromX1 = ClipX         + OriginX;
		fromY1 = ClipY         + OriginY;
		fromX2 = NewRect.ClipX + NewRect.OriginX;
		fromY2 = NewRect.ClipY + NewRect.OriginY;
		toX1   = fromX1        + ClipWidth;
		toY1   = fromY1        + ClipHeight;
		toX2   = fromX2        + NewRect.ClipWidth;
		toY2   = fromY2        + NewRect.ClipHeight;

		// Clip
		if (fromX1 < fromX2)
			fromX1 = fromX2;
		if (fromY1 < fromY2)
			fromY1 = fromY2;
		if (toX1 > toX2)
			toX1 = toX2;
		if (toY1 > toY2)
			toY1 = toY2;

		// Reconvert to origin of this object
		ClipX      = fromX1 - OriginX;
		ClipY      = fromY1 - OriginY;
		ClipWidth  = toX1   - fromX1;
		ClipHeight = toY1   - fromY1;
	}

	bool HasArea(void)
	{
		return ((ClipWidth > 0) && (ClipHeight > 0));
	}

	// Members
	float OriginX;		// X origin of rectangle, in absolute coordinates
	float OriginY;		// Y origin of rectangle, in absolute coordinates

	float ClipX;		// Leftmost edge of rectangle relative to origin
	float ClipY;		// Topmost edge of rectangle relative to origin
	float ClipWidth;	// Width of rectangle
	float ClipHeight;	// Height of rectangle
};

//==========================================================================
//
//	KGC
//
//==========================================================================

class KGC:public KObject
{
	DECLARE_CLASS(KGC, KObject, 0);

	friend class KWindow;

	KGC();

private:
	KCanvas		*Canvas;

	KClipRect	ClipRect;

	FColor		TileColor;

	FColor		TextColor;
	KFont		*Font;
/*		
		FLOAT     textVSpacing;*/
	EHAlign		HAlign;
	EVAlign		VAlign;
/*		BITFIELD  bWordWrap:1;

		BITFIELD  bFree:1;
		int       gcCount;
		XGC       *gcStack;
		XGC       *gcFree;
		XGC       *gcOwner;*/
public:
	void SetCanvas(KCanvas *NewCanvas);
	KCanvas *GetCanvas(void) { return Canvas; }

	void SetClipRect(KClipRect &NewClipRect);
	void Intersect(KClipRect &IntersectClipRect);
	void Intersect(float ClipX, float ClipY,
		float ClipWidth, float ClipHeight);
	KClipRect GetClipRect(void) { return ClipRect; }

	void SetTileColor(FColor newTileColor);
	FColor GetTileColor(void) { return TileColor; }

	void SetTextColor(FColor newTextColor);
	FColor GetTextColor(void) { return TextColor; }

	void SetFont(KFont *NewFont);
	KFont *GetFont(void) { return Font; }

	void SetHorizontalAlignment(EHAlign newHAlign);
	EHAlign GetHorizontalAlignment(void) { return HAlign; }

	void SetVerticalAlignment(EVAlign newVAlign);
	EVAlign GetVerticalAlignment(void) { return VAlign; }

	void SetAlignments(EHAlign newHAlign, EVAlign newVAlign);
	void GetAlignments(EHAlign *pHAlign = NULL, EVAlign *pVAlign = NULL);

	void DrawIcon(float x, float y, KTexture *Texture, bool UseOffsets = true);
	void DrawIconStretch(float x, float y, float width, float height, KTexture *Tex);

	void DrawText(int x, int y, const char *Text);
	int TextWidth(const char *Text)
	{
		return Font->TextWidth(Text);
	}

protected:
	void ClipTile(float &x1, float &y1, float &x2, float &y2,
		float &s1, float &t1, float &s2, float &t2);
	void DrawTile(KTexture *Texture,
		float x1, float y1, float x2, float y2,
		float s1, float t1, float s2, float t2);
};
