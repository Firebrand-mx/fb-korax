// DGL Font Renderer.
// The font must be small enough to fit one texture 
// (not a problem with *real* video cards!).

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "h2def.h"
#include "gl_font.h"
#include <stdio.h>
#include "tga.h"
#include "i_win32.h"

static int		initOk = 0;
static int		numFonts;
static jfrfont_t *fonts;		// The list of fonts.
static int		current;		// Index of the current font.

extern int test3dfx;

// Returns zero if no errors.
int FR_Init()
{
	if(initOk) return -1; // No reinitializations...
	
	numFonts = 0;
	fonts = 0;			// No fonts!
	current = -1;
	initOk = 1;
	return 0;
}

// Destroys the font with the index.
static void FR_DestroyFontIdx(int idx)
{
	jfrfont_t *font = fonts + idx;
	
	gl.DeleteTextures(1, &font->texture);
	memmove(fonts+idx, fonts+idx+1, sizeof(jfrfont_t)*(numFonts-idx-1));
	numFonts--;
	fonts = (jfrfont_t *)realloc(fonts, sizeof(jfrfont_t)*numFonts);
	// -JL- Paranoia
	if (numFonts && !fonts)
		I_Error("FR_DestroyFontIdx: realloc failed");
	if(current == idx) current = -1;
}

void FR_Shutdown()
{
	// Destroy all fonts.
	while(numFonts) FR_DestroyFontIdx(0);
	fonts = 0;
	current = -1;
	initOk = 0;
}

int FR_GetFontIdx(int id)
{
	int i;
	for(i=0; i<numFonts; i++)
		if(fonts[i].id == id) return i;
	return -1;
}

jfrfont_t *FR_GetFont(int id)
{
	int idx = FR_GetFontIdx(id);
	if(idx == -1) return 0;
	return fonts + idx;
}

static int FR_GetMaxId()
{
	int	i, grid=0;
	for(i=0; i<numFonts; i++)
		if(fonts[i].id > grid) grid = fonts[i].id;
	return grid;
}

static int findPow2(int num)
{
	int cumul;
	for(cumul=1; num > cumul; cumul *= 2);
	return cumul;
}

// Prepare a GDI font. Select it as the current font.
int FR_PrepareGDIFont(HFONT hfont)
{
	jfrfont_t	*font;
	int			i, x, y, maxh, bmpWidth=256, bmpHeight=0, imgWidth, imgHeight;
	HDC			hdc;
	HBITMAP		hbmp;
	unsigned int *image;

	// Create a new font.
	fonts = (jfrfont_t *)realloc(fonts, sizeof(jfrfont_t) * ++numFonts);
	// -JL- Paranoia
	if (!fonts)
		I_Error("FR_PrepareGDIFont: realloc failed");
	font = fonts + numFonts-1;
	memset(font, 0, sizeof(jfrfont_t));
	font->id = FR_GetMaxId() + 1;
	current = numFonts - 1;	
		
	// Now we'll create the actual data.
	hdc = CreateCompatibleDC(NULL);
	SetMapMode(hdc, MM_TEXT);
	SelectObject(hdc, hfont);
	// Let's first find out the sizes of all the characters.
	// Then we can decide how large a texture we need.
	for(i=0, x=0, y=0, maxh=0; i<256; i++)
	{
		jfrchar_t *fc = font->chars + i;
		SIZE size;
		char ch[2] = { i, 0 };
		GetTextExtentPoint32(hdc, ch, 1, &size);
		fc->w = size.cx;
		fc->h = size.cy;
		maxh = max(maxh, fc->h);
		x += fc->w;
		if(x >= bmpWidth)	
		{
			x = 0;
			y += maxh;
			maxh = 0;
		}
	}
	bmpHeight = y + maxh;
	hbmp = CreateCompatibleBitmap(hdc, bmpWidth, bmpHeight);
	SelectObject(hdc, hbmp);
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, 0);
	SetTextColor(hdc, 0xffffff);
	// Print all the characters.
	for(i=0, x=0, y=0, maxh=0; i<256; i++)
	{
		jfrchar_t *fc = font->chars + i;
		char ch[2] = { i, 0 };
		if(x+fc->w >= bmpWidth)
		{
			x = 0;
			y += maxh;
			maxh = 0;
		}
		if(i) TextOut(hdc, x, y, ch, 1);
		fc->x = x;
		fc->y = y;
		maxh = max(maxh, fc->h);
		x += fc->w;
	}
	// Now we can make a version that DGL can read.	
	imgWidth = findPow2(bmpWidth);
	imgHeight = findPow2(bmpHeight);
//	printf( "font: %d x %d\n", imgWidth, imgHeight);
	image = (unsigned int *)malloc(4*imgWidth*imgHeight);
	// -JL- Paranoia
	if (!image)
		I_Error("FR_PrepareGDIFont: malloc failed");
	memset(image, 0, 4*imgWidth*imgHeight);
	for(y=0; y<bmpHeight; y++)
		for(x=0; x<bmpWidth; x++)
			if(GetPixel(hdc, x, y))
				image[x + y*imgWidth] = 0xffffffff;

	if(test3dfx) saveTGA24_rgba8888("jhxfont.tga", bmpWidth, bmpHeight, (unsigned char*)image);

	font->texWidth = imgWidth;
	font->texHeight = imgHeight;

	// Create the DGL texture.
	font->texture = gl.NewTexture();
	gl.TexImage(DGL_RGBA, imgWidth, imgHeight, 0, image);
	gl.TexParameter(DGL_MIN_FILTER, DGL_LINEAR);
	gl.TexParameter(DGL_MAG_FILTER, DGL_NEAREST);

	// We no longer need these.
	free(image);
	DeleteObject(hbmp);
	DeleteDC(hdc);
	return 0;
}

// Change the current font.
void FR_SetFont(int id)
{
	int idx = FR_GetFontIdx(id);	
	if(idx == -1) return;	// No such font.
	current = idx;
}

int FR_TextWidth(char *text)
{
	int i, width = 0, len = strlen(text);
	jfrfont_t *cf;

	if(current == -1) return 0;
	
	// Just add them together.
	for(cf=fonts+current, i=0; i<len; i++)
		width += cf->chars[text[i]].w;
	
	return width;
}

int FR_TextHeight(char *text)
{
	int i, height = 0, len;
	jfrfont_t *cf;

	if(current == -1 || !text) return 0;

	// Find the greatest height.
	for(len=strlen(text), cf=fonts+current, i=0; i<len; i++)
		height = max(height, cf->chars[text[i]].h);

	return height;
}

// (x,y) is the upper left corner.
void FR_TextOut(char *text, int x, int y)
{
	int i, len;
	jfrfont_t *cf;

	if(!text) return;
	len = strlen(text);

	// Check the font.
	if(current == -1) return;	// No selected font.
	cf = fonts + current;

	// Set the texture.
	gl.Bind(cf->texture);

	// Print it.
	gl.Begin(DGL_QUADS);
	for(i=0; i<len; i++)
	{
		jfrchar_t *ch = cf->chars + text[i];
		float texw = (float)cf->texWidth, texh = (float)cf->texHeight;

		// Upper left.
		gl.TexCoord2f(ch->x/texw, ch->y/texh);
		gl.Vertex2f(x, y);
		// Upper right.
		gl.TexCoord2f((ch->x+ch->w)/texw, ch->y/texh);
		gl.Vertex2f(x+ch->w+.01f, y);
		// Lower right.
		gl.TexCoord2f((ch->x+ch->w)/texw, (ch->y+ch->h)/texh);
		gl.Vertex2f(x+ch->w+.01f, y+ch->h+.01f);
		// Lower left.
		gl.TexCoord2f(ch->x/texw, (ch->y+ch->h)/texh);
		gl.Vertex2f(x, y+ch->h+.01f);
		// Move on.
		x += ch->w;
	}
	if(test3dfx)
	{
		gl.TexCoord2f(0, 0);
		gl.Vertex2f(320, 0);
		gl.TexCoord2f(1, 0);
		gl.Vertex2f(640, 0);
		gl.TexCoord2f(1, 1);
		gl.Vertex2f(640, 160);
		gl.TexCoord2f(0, 1);
		gl.Vertex2f(320, 160);
	}
	gl.End();
}

int FR_GetCurrent()
{
	if(current == -1) return 0;
	return fonts[current].id;
}