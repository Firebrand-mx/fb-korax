// DGL Font Renderer. Header file.

#ifndef __OGL_FONT_RENDERER_H__
#define __OGL_FONT_RENDERER_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAX_CHARS	256			// Normal 256 ANSI characters.

// Data for a character.
typedef struct
{
	int	x, y;					// The upper left corner of the character.
	int	w, h;					// The width and height.
} jfrchar_t;

// Data for a font.
typedef struct
{
	int				id;
	unsigned int	texture;	// The name of the texture for this font.
	int				texWidth, texHeight;
	jfrchar_t		chars[MAX_CHARS];	
} jfrfont_t;


int FR_Init();
void FR_Shutdown();
jfrfont_t *FR_GetFont(int id);

// Prepare a GDI font. Select it as the current font.
int FR_PrepareGDIFont(HFONT hfont);

// Change the current font.
void FR_SetFont(int id);
int FR_GetCurrent();

int FR_TextWidth(char *text);
int FR_TextHeight(char *text);

// (x,y) is the upper left corner. Returns the length.
int FR_TextOut(char *text, int x, int y);

#endif // __OGL_FONT_RENDERER_H__