// drD3D.dll
// Direct3D driver for Doomsday Graphics Library
//
// viewport.c : Handing the viewport and scissor

#include "drD3D.h"

namespace Direct3DDriver {

int		useScissor = DGL_FALSE;


void enableScissor(int enable) 
{
	useScissor = enable;
	IDirect3DDevice3_SetCurrentViewport(d3dDevice, enable? d3dScissor : d3dVp);
}

// ------------------------------------------------------------------------


void Viewport(int x, int y, int width, int height)
{
	guard(Viewport);
	vpData.dwX = x;
	vpData.dwY = y;
	vpData.dwWidth = width;
	vpData.dwHeight = height;
	IDirect3DViewport3_SetViewport2(d3dVp, &vpData);
	unguard;
}


void Scissor(int x, int y, int width, int height)
{
	guard(Scissor);
	float sx, sy, sw, sh;	// The relative coordinates of the scissor box.

	// The scissor box is always smaller than the viewport. Otherwise, 
	// what would be the point of having one? We assume this is the 
	// case without checking.
	scissorData.dwX = x;
	scissorData.dwY = y;
	scissorData.dwWidth = width;
	scissorData.dwHeight = height;

	// Determining the boundaries of the space is a bit more difficult
	// task. We'll base our calculations on the current viewport.
	// First we get the relative (0..1) coordinates of the clipper box.
	sx = (x - vpData.dwX) / (float) vpData.dwWidth;
	sy = (y - vpData.dwY) / (float) vpData.dwHeight;
	sw = width / (float) vpData.dwWidth;
	sh = height / (float) vpData.dwHeight;
	// Use the relative coordinates to calculate what portion of the
	// viewport clip volume should we use.
	scissorData.dvClipX = vpData.dvClipX + vpData.dvClipWidth * sx;
	scissorData.dvClipY = vpData.dvClipY - vpData.dvClipHeight * sy;
	scissorData.dvClipWidth = vpData.dvClipWidth * sw;
	scissorData.dvClipHeight = vpData.dvClipHeight * sh;

	// Update the viewport.
	IDirect3DViewport3_SetViewport2(d3dScissor, &scissorData);
	unguard;
}

} // namespace Direct3DDriver