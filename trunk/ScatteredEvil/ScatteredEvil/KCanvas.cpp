
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

KCanvas *GCanvas;

KTexture *KCanvas::Textures;
int KCanvas::NumTextures;

KFont *KCanvas::SmallFont;
KFont *KCanvas::YellowFont;
KFont *KCanvas::BigFont;

IMPLEMENT_CLASS(KCanvas);

//==========================================================================
//
//	KFont::KFont
//
//==========================================================================

KFont::KFont(char *Name, int InSpaceWidth, int InSpaceHeight) 
	: SpaceWidth(InSpaceWidth)
	, SpaceHeight(InSpaceHeight)
{
	int		i;
	char   	buffer[12];

	memset(Chars, 0, sizeof(Chars));
	for (i = 0; i < 96; i++)
	{
		sprintf(buffer, "%s%02d", Name, i);
		if (gi.W_CheckNumForName(buffer) >= 0)
		{
			Chars[i] = KCanvas::FindTexture(buffer);
			if ((i + 32 >= 'a') && (i + 32 <= 'z') && !Chars[i + 'A' - 'a'])
			{
				Chars[i + 'A' - 'a'] = Chars[i];
			}
			if ((i + 32 >= 'A') && (i + 32 <= 'Z') && !Chars[i + 'a' - 'A'])
			{
				Chars[i + 'a' - 'A'] = Chars[i];
			}
		}
	}
	if (!Chars['_' - 32] && Chars['[' - 32])
	{
		Chars['_' - 32] = Chars['[' - 32];
	}
	if (!Chars['\\' - 32] && Chars['/' - 32])
	{
		Chars['\\' - 32] = Chars['/' - 32];
	}
}

//==========================================================================
//
//	KFont::KFont
//
//==========================================================================

int KFont::TextWidth(const char *Text)
{
	int		c;
	int		w;
	int		i;

	w = 0;
	for (i = 0; Text[i]; i++)
	{
		c = Text[i] - 32;

		if (c < 0)
		{
			continue;
		}
		if (c >= 96 || !Chars[c])
		{
			w += SpaceWidth;
			continue;
		}
		w += Chars[c]->Width - 1;
	}
	return w;
}

//==========================================================================
//
//	KCanvas::KCanvas
//
//==========================================================================

KCanvas::KCanvas()
{
}

//==========================================================================
//
//	KCanvas::StaticInit
//
//==========================================================================

void KCanvas::StaticInit(void)
{
	GCanvas = Spawn<KCanvas>();
	SmallFont = new KFont("FONTA", 5, 7);
	YellowFont = new KFont("FONTAY", 5, 7);
	BigFont = new KFont("FONTB", 5, 10);
}

//==========================================================================
//
//	KCanvas::FindTexture
//
//==========================================================================

KTexture *KCanvas::FindTexture(char *Name)
{
	for (int i = 0; i < NumTextures; i++)
	{
		if (!stricmp(Name, Textures[i].Name))
			return &Textures[i];
	}
	NumTextures++;
	Textures = (KTexture *)realloc(Textures, NumTextures * sizeof(KTexture));
	KTexture &T = Textures[NumTextures - 1];
	strcpy(T.Name, Name);
	T.LumpNum = gi.W_GetNumForName(Name);
	patch_t *patch = (patch_t *)gi.W_CacheLumpNum(T.LumpNum, PU_CACHE);
	T.Width = SHORT(patch->width);
	T.Height = SHORT(patch->height);
	T.XOffset = SHORT(patch->leftoffset);
	T.YOffset = SHORT(patch->topoffset);
	return &Textures[NumTextures - 1];
}

//==========================================================================
//
//	KCanvas::DrawTile
//
//==========================================================================

void KCanvas::DrawTile(KTexture *Texture,
	float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2)
{
	gi.GL_SetPatch(Texture->LumpNum);

	x1 += OrgX;
	x2 += OrgX;
	y1 += OrgY;
	y2 += OrgY;
	s1 /= Texture->Width;
	s2 /= Texture->Width;
	t1 /= Texture->Height;
	t2 /= Texture->Height;

	gl.Begin(DGL_QUADS);
	gl.TexCoord2f(s1, t1);
	gl.Vertex2f(x1, y1);
	gl.TexCoord2f(s2, t1);
	gl.Vertex2f(x2, y1);
	gl.TexCoord2f(s2, t2);
	gl.Vertex2f(x2, y2);
	gl.TexCoord2f(s1, t2);
	gl.Vertex2f(x1, y2);
	gl.End();
}

//==========================================================================
//
//	KCanvas::DrawText
//
//==========================================================================

void KCanvas::DrawText(int x, int y, const char *Text)
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

//==========================================================================
//
//
//
//==========================================================================
