
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
: TileColor(255, 255, 255, 255)
, TextColor(255, 255, 255, 255)
{
}

//==========================================================================
//
//	KGC::SetCanvas
//
//==========================================================================

void KGC::SetCanvas(KCanvas *NewCanvas) 
{
	guard(KGC::SetCanvas);
	Canvas = NewCanvas;
	Canvas->SetOrigin(ClipRect.OriginX, ClipRect.OriginY);
	unguard;
}

//==========================================================================
//
//	KGC::SetClipRect
//
//==========================================================================

void KGC::SetClipRect(KClipRect &NewClipRect)
{
	guard(KGC::SetClipRect);
	ClipRect = NewClipRect;
	Canvas->SetOrigin(ClipRect.OriginX, ClipRect.OriginY);
	unguard;
}

//==========================================================================
//
//	KGC::Intersect
//
//==========================================================================

void KGC::Intersect(KClipRect &IntersectClipRect)
{
	guard(KGC::Intersect);
	ClipRect.Intersect(IntersectClipRect);
	unguard;
}

//==========================================================================
//
//	KGC::Intersect
//
//==========================================================================

void KGC::Intersect(float ClipX, float ClipY, float ClipWidth, float ClipHeight)
{
	guard(KGC::Intersect);
	ClipRect.Intersect(ClipX, ClipY, ClipWidth, ClipHeight);
	unguard;
}

//==========================================================================
//
//	KGC::SetTileColor
//
//==========================================================================

void KGC::SetTileColor(FColor newTileColor)
{
	guard(KGC::SetTileColor);
	TileColor = newTileColor;
	unguard;
}

//==========================================================================
//
//	KGC::SetTextColor
//
//==========================================================================

void KGC::SetTextColor(FColor newTextColor)
{
	guard(KGC::SetTextColor);
	TextColor = newTextColor;
	unguard;
}

//==========================================================================
//
//	KGC::SetFont
//
//==========================================================================

void KGC::SetFont(KFont *NewFont)
{
	guard(KGC::SetFont);
	if (NewFont)
		Font = NewFont;
	unguard;
}

//==========================================================================
//
//	KGC::SetHorizontalAlignment
//
//==========================================================================

void KGC::SetHorizontalAlignment(EHAlign newHAlign)
{
	guard(KGC::SetHorizontalAlignment);
	HAlign = newHAlign;
	unguard;
}

//==========================================================================
//
//	KGC::SetVerticalAlignment
//
//==========================================================================

void KGC::SetVerticalAlignment(EVAlign newVAlign)
{
	guard(KGC::SetVerticalAlignment);
	VAlign = newVAlign;
	unguard;
}

//==========================================================================
//
//	KGC::SetAlignments
//
//==========================================================================

void KGC::SetAlignments(EHAlign newHAlign, EVAlign newVAlign)
{
	guard(KGC::SetAlignments);
	HAlign = newHAlign;
	VAlign = newVAlign;
	unguard;
}

//==========================================================================
//
//	KGC::GetAlignments
//
//==========================================================================

void KGC::GetAlignments(EHAlign *pHAlign, EVAlign *pVAlign)
{
	guard(KGC::GetAlignments);
	if (pHAlign) *pHAlign = HAlign;
	if (pVAlign) *pVAlign = VAlign;
	unguard;
}

//==========================================================================
//
//	KGC::DrawIcon
//
//==========================================================================

void KGC::DrawIcon(float x, float y, KTexture *Texture, bool UseOffsets)
{
	guard(KGC::DrawIcon);
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
	unguard;
}

//==========================================================================
//
//	KGC::DrawIconStretch
//
//==========================================================================

void KGC::DrawIconStretch(float x, float y, float width, float height, 
						  KTexture *Texture)
{
	guard(KGC::DrawIconStretch);
	if (!Texture)
	{
		return;
	}
	DrawTile(Texture, x, y, x + width, y + height,
		0, 0, Texture->Width, Texture->Height);
	unguard;
}

//==========================================================================
//
//	KGC::ClipTile
//
//==========================================================================

void KGC::ClipTile(float &x1, float &y1, float &x2, float &y2,
	float &s1, float &t1, float &s2, float &t2)
{
	guard(KGC::ClipTile);
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
	unguard;
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
	guard(KGC::DrawTile);
	ClipTile(x1, y1, x2, y2, s1, t1, s2, t2);
	if (x1 < x2 && y1 < y2)
	{
		Canvas->SetColor(TileColor);
		Canvas->DrawTile(Texture, x1, y1, x2, y2, s1, t1, s2, t2);
	}
	unguard;
}

//==========================================================================
//
//	KGC::DrawText
//
//==========================================================================

void KGC::DrawText(int x, int y, const char *Text)
{
	guard(KGC::DrawText);
	int		c;
	int		i;

	FColor CurColor = TileColor;
	TileColor = TextColor;

	if (HAlign == HALIGN_Center)
		x -= TextWidth(Text) / 2;
	else if (HAlign == HALIGN_Right)
		x -= TextWidth(Text);
	if (VAlign == VALIGN_Center)
		y -= Font->SpaceHeight / 2;
	else if (VAlign == VALIGN_Bottom)
		x -= Font->SpaceHeight;
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

	TileColor = CurColor;
	unguard;
}
