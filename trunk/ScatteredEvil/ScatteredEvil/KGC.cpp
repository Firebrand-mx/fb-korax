
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include "h2def.h"
#include "mn_def.h"

IMPLEMENT_CLASS(KGC);

//==========================================================================
//
//	KGC::KGC
//
//==========================================================================

KGC::KGC()
{
}

//==========================================================================
//
//	KGC::SetCanvas
//
//==========================================================================

void KGC::SetCanvas(KCanvas *NewCanvas) 
{ 
	Canvas = NewCanvas;
	Canvas->SetOrigin(ClipRect.OriginX, ClipRect.OriginY);
}

//==========================================================================
//
//	KGC::SetClipRect
//
//==========================================================================

void KGC::SetClipRect(KClipRect &NewClipRect)
{
	ClipRect = NewClipRect;
	Canvas->SetOrigin(ClipRect.OriginX, ClipRect.OriginY);
}

//==========================================================================
//
//	KGC::Intersect
//
//==========================================================================

void KGC::Intersect(KClipRect &IntersectClipRect)
{
	ClipRect.Intersect(IntersectClipRect);
}

//==========================================================================
//
//	KGC::Intersect
//
//==========================================================================

void KGC::Intersect(float ClipX, float ClipY, float ClipWidth, float ClipHeight)
{
	ClipRect.Intersect(ClipX, ClipY, ClipWidth, ClipHeight);
}

//==========================================================================
//
//	KGC::DrawIcon
//
//==========================================================================

void KGC::DrawIcon(float x, float y, KTexture *Texture, bool UseOffsets)
{
	if (!Texture)
	{
		return;
	}
	if (UseOffsets)
	{
		x -= Texture->XOffset;
		y -= Texture->YOffset;
	}
	DrawTile(Texture, x, y, x + Texture->Width, y + Texture->Height,
		0, 0, Texture->Width, Texture->Height);
}

//==========================================================================
//
//	KGC::DrawIconStretch
//
//==========================================================================

void KGC::DrawIconStretch(float x, float y, float width, float height, 
						  KTexture *Texture)
{
	if (!Texture)
	{
		return;
	}
	DrawTile(Texture, x, y, x + width, y + height,
		0, 0, Texture->Width, Texture->Height);
}

//==========================================================================
//
//	KGC::ClipTile
//
//==========================================================================

void KGC::ClipTile(float &x1, float &y1, float &x2, float &y2,
	float &s1, float &t1, float &s2, float &t2)
{
	float Clip;

	Clip = ClipRect.ClipX;
	if (x1 < Clip)
	{
		s1 = s1 + (Clip - x1) / (x2 - x1) * (s2 - s1);
		x1 = Clip;
	}
	Clip = ClipRect.ClipX + ClipRect.ClipWidth;
	if (x2 > Clip)
	{
		s2 = s2 + (Clip - x2) / (x2 - x1) * (s2 - s1);
		x2 = Clip;
	}
	Clip = ClipRect.ClipY;
	if (y1 < Clip)
	{
		t1 = t1 + (Clip - y1) / (y2 - y1) * (t2 - t1);
		y1 = Clip;
	}
	Clip = ClipRect.ClipY + ClipRect.ClipHeight;
	if (y2 > Clip)
	{
		t2 = t2 + (Clip - y2) / (y2 - y1) * (t2 - t1);
		y2 = Clip;
	}
}

//==========================================================================
//
//	KGC::DrawTile
//
//==========================================================================

void KGC::DrawTile(KTexture *Texture,
	float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2)
{
	ClipTile(x1, y1, x2, y2, s1, t1, s2, t2);
	if (x1 < x2 && y1 < y2)
	{
		Canvas->DrawTile(Texture, x1, y1, x2, y2, s1, t1, s2, t2);
	}
}

//==========================================================================
//
//	KGC::DrawText
//
//==========================================================================

void KGC::DrawText(int x, int y, const char *Text)
{
	int		c;
	int		i;
		
	for (i = 0; Text[i]; i++)
	{
		c = Text[i] - 32;
		if (c < 0)
		{
			continue;
		}
		if (c >= 96 || !Font->Chars[c])
		{
			x += Font->SpaceWidth;
			continue;
		}
		DrawIcon(x, y, Font->Chars[c]);
		x += Font->Chars[c]->Width - 1;
	}
}
