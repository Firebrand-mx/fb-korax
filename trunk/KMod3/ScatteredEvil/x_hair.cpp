
//**************************************************************************
//**
//** X_HAIR.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdlib.h>
#include "h2def.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define MAX_XLINES	16

// TYPES -------------------------------------------------------------------

typedef struct
{
	int				x, y;		// In window coordinates (*not* 320x200!).
} crosspoint_t;

typedef struct
{
	crosspoint_t	a, b;
} crossline_t;

typedef struct
{
	int				numlines;
	crossline_t		lines[MAX_XLINES];
} cross_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int		xhair=0, xhairSize=1;
byte	xhairColor[3] = { 255, 255, 255 };

#define XL(x1,y1,x2,y2) {{x1,y1},{x2,y2}}

cross_t crosshairs[NUM_XHAIRS] = 
{
	// + (open center)
	{ 4, { XL(-5,0, -2,0), XL(0,-5, 0,-2), XL(5,0, 2,0), XL(0,5, 0,2) } },
	// > <
	{ 4, { XL(-7,-5, -2,0), XL(-7,5, -2,0), XL(7,-5, 2,0), XL(7,5, 2,0) } },
	// square
	{ 4, { XL(-3,-3, -3,3), XL(-3,3, 3,3), XL(3,3, 3,-3), XL(3,-3, -3,-3) } },
	// square (open center)
	{ 8, { XL(-4,-4, -4,-2), XL(-4,2, -4,4), XL(-4,4, -2,4), XL(2,4, 4,4),
		XL(4,4, 4,2), XL(4,-2, 4,-4), XL(4,-4, 2,-4), XL(-2,-4, -4,-4) } },
	// diamond
	{ 4, { XL(0,-3, 3,0), XL(3,0, 0,3), XL(0,3, -3,0), XL(-3,0, 0,-3) } },
	// ^
	{ 2, { XL(-4,-4, 0,0), XL(0,0, 4,-4) } }
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void X_Drawer()
{
	// Where to draw the xhair?
	int	centerX = screenWidth / 2;
	int	centerY = (viewwindowy + 2) * screenHeight / 200.0f + viewph / 2.0f;
	int	i;
	float fact = (xhairSize+1)/2.0f;
	byte xcolor[3] = { xhairColor[0], xhairColor[1], xhairColor[2] };
	cross_t *cross;

	// Is there a crosshair to draw?
	if(!xhair) return;

	gl.Disable(DGL_TEXTURING);
	// Push the current matrices.
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PushMatrix();
	gl.LoadIdentity();
	gl.MatrixMode(DGL_PROJECTION);
	gl.PushMatrix();
	gl.LoadIdentity();
	// We need the 1:1 coordinates.
	gl.Ortho(0, 0, screenWidth, screenHeight, -1, 1);

	cross = crosshairs + xhair-1;

	gl.Color3ubv(xcolor);
	gl.Begin(DGL_LINES);
	for(i=0; i<cross->numlines; i++)
	{
		crossline_t *xline = cross->lines + i;
		gl.Vertex2f(fact*xline->a.x + centerX, fact*xline->a.y + centerY);
		gl.Vertex2f(fact*xline->b.x + centerX, fact*xline->b.y + centerY);
	}
	gl.End();

	gl.Enable(DGL_TEXTURING);
	// Pop back the old matrices.
	gl.PopMatrix();
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PopMatrix();
}

int CCmdCrosshair(int argc, char **argv)
{
	if(argc == 1)
	{
		CON_Printf( "Usage:\n  crosshair (num)\n");
		CON_Printf( "  crosshair size (size)\n");
		CON_Printf( "  crosshair color (r) (g) (b)\n");
		CON_Printf( "Num: 0=no crosshair, 1-%d: use crosshair 1...%d\n", NUM_XHAIRS, NUM_XHAIRS);
		CON_Printf( "Size: 1=normal\n");
		CON_Printf( "R, G, B: 0-255\n");
		CON_Printf( "Current values: xhair=%d, size=%d, color=(%d, %d, %d)\n",
			xhair, xhairSize, xhairColor[0], xhairColor[1], xhairColor[2]);
		return true;
	}
	else if(argc == 2) // Choose.
	{
		xhair = strtol(argv[1], NULL, 0);
		if(xhair > NUM_XHAIRS || xhair < 0) 
		{ 
			xhair = 0;
			return false;
		}
		CON_Printf( "Crosshair %d selected.\n", xhair);
	}
	else if(argc == 3) // Size.
	{
		if(stricmp(argv[1], "size")) return false;
		xhairSize = strtol(argv[2], NULL, 0);
		if(xhairSize < 0) xhairSize = 0;
		CON_Printf( "Crosshair size set to %d.\n", xhairSize);
	}
	else if(argc == 5) // Color.
	{
		int i;
		if(stricmp(argv[1], "color")) return false;
		for(i=0; i<3; i++) 
		{
			xhairColor[i] = strtol(argv[2+i], NULL, 0);
			// Clamp.
			if(xhairColor[i] < 0) xhairColor[i] = 0;
			if(xhairColor[i] > 255) xhairColor[i] = 255;
		}
		CON_Printf( "Crosshair color set to (%d, %d, %d).\n", 
			xhairColor[0], xhairColor[1], xhairColor[2]);
	}
	else return false;
	// Success!
	return true;
}