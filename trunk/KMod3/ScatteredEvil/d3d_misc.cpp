// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// misc.c : Miscellaneous routines

#include "drD3D.h"

namespace Direct3DDriver {

int Grab(int x, int y, int width, int height, int format, void *buffer)
{
	guard(Grab);
	int				outx, outy, i;
	int				offsets[3], bits[3], masks[3];
	unsigned int	temp;
	DDSURFACEDESC2	dsd;
	RECT			rect = { x, y, x+width, y+height };
	byte			*linestart, *out, *inPtr;
	int				size;

	if(format != DGL_RGB) return DGL_UNSUPPORTED;

	// We need to lock the back buffer and take a copy of its contents.
	memset(&dsd, 0, sizeof(dsd));
	dsd.dwSize = sizeof(dsd);

	while((hr = IDirectDrawSurface4_Lock(sBack, 
		&rect, &dsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_READONLY, NULL)) == DDERR_WASSTILLDRAWING);
	if(FAILED(hr))
	{
		I_Error("drD3D.Grab: Couldn't lock the back buffer.\n");
		return DGL_ERROR;
	}

	if(dsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
	{
		I_Error("drD3D.Grab: Alpha data in the back buffer! How can this be?\n");
	}

	// Unravel the pixel format.
	unpackMask(masks[0] = dsd.ddpfPixelFormat.dwRBitMask, offsets, bits);
	unpackMask(masks[1] = dsd.ddpfPixelFormat.dwGBitMask, offsets+1, bits+1);
	unpackMask(masks[2] = dsd.ddpfPixelFormat.dwBBitMask, offsets+2, bits+2);
	size = dsd.ddpfPixelFormat.dwRGBBitCount/8;

	// The data has to be upside down (OpenGL + Targa compliance).
	for(linestart = (byte*) dsd.lpSurface + dsd.lPitch*(height-1), out=(byte*)buffer, outy=0; 
		outy<height; outy++, linestart -= dsd.lPitch)
	{
		inPtr = linestart;
		for(outx=0; outx<width; outx++, inPtr += size)
		{
			temp = 0;
			memcpy(&temp, inPtr, size);
			for(i=0; i<3; i++)
			{
				out[i] = (temp & masks[i]) >> offsets[i] << (8-bits[i]);
			}
			out += 3;
		}
	}

	IDirectDrawSurface4_Unlock(sBack, &rect);
	return DGL_OK;
	unguard;
}


// Clipping will be performed.
int Project(int num, gl_fc3vertex_t *inVertices, gl_fc3vertex_t *outVertices)
{
	guard(Project);
	D3DLVERTEX			*d3dInVerts, *d3dOutVerts;
	D3DHVERTEX			*d3dHVerts;
	D3DTRANSFORMDATA	trd;
	int					i;
	gl_fc3vertex_t		*outPtr;
	DWORD				offScreen = 0;

	if(num == 0) return 0;

	// Allocate the buffers.
	d3dInVerts = (D3DLVERTEX *)calloc(num, sizeof(D3DLVERTEX));
	d3dOutVerts = (D3DLVERTEX *)calloc(num, sizeof(D3DLVERTEX));
	d3dHVerts = (D3DHVERTEX *)calloc(num, sizeof(D3DHVERTEX));

	memset(&trd, 0, sizeof(trd));
	trd.dwSize = sizeof(trd);
	trd.lpIn = d3dInVerts;
	trd.dwInSize = sizeof(D3DLVERTEX);
	trd.lpOut = d3dOutVerts;
	trd.dwOutSize = sizeof(D3DLVERTEX);
	trd.lpHOut = d3dHVerts;
	trd.dwClip = D3DCLIP_FRONT | D3DCLIP_BACK 
		| D3DCLIP_LEFT | D3DCLIP_RIGHT | D3DCLIP_TOP | D3DCLIP_BOTTOM;

	// Prepare the vertices.
	for(i=0; i<num; i++)
	{
		d3dInVerts[i].x = inVertices[i].pos[VX];
		d3dInVerts[i].y = inVertices[i].pos[VY];
		d3dInVerts[i].z = inVertices[i].pos[VZ];
	}

	// Do the transform.
	if(FAILED(hr = IDirect3DViewport3_TransformVertices(d3dVp,
		num, &trd, D3DTRANSFORM_CLIPPED, &offScreen)))
	{
		I_Error("drD3D.Project: TransformVertices failed. Result: %i.\n", hr&0xffff);
	}

	// Fill in the output vertices.
	for(i=0, outPtr=outVertices; i<num; i++)
		if(!d3dHVerts[i].dwFlags)
		{
			// This vertex wasn't clipped.
			memcpy(outPtr->color, inVertices[i].color, sizeof(inVertices[i].color));
			outPtr->pos[VX] = d3dOutVerts[i].x;
			outPtr->pos[VY] = d3dOutVerts[i].y;
			outPtr->pos[VZ] = d3dOutVerts[i].z;
			outPtr++;
		}

	// We're done, free the buffers.
	free(d3dInVerts);
	free(d3dOutVerts);
	free(d3dHVerts);

	return outPtr - outVertices;
	unguard;
}


int ReadPixels(int *inData, int format, void *pixels)
{
	guard(ReadPixels);
	int		type = inData[0], num, *coords, i;
	float	*fv = (float *)pixels;
	int		returnValue = DGL_OK;
	DDSURFACEDESC2 dsd;
	int		size;
	unsigned int temp, maxDepth;
	int		offset, bits, mask;
	
	if(format != DGL_DEPTH_COMPONENT) return DGL_UNSUPPORTED;

	// Lock the depth buffer so we can read things from it.
	memset(&dsd, 0, sizeof(dsd));
	dsd.dwSize = sizeof(dsd);
	
	while((hr = IDirectDrawSurface4_Lock(sDepth, NULL, &dsd, 
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_READONLY, NULL)) == DDERR_WASSTILLDRAWING);
	if(FAILED(hr))
	{
		I_Error("drD3D.ReadPixels: Couldn't lock the depth buffer. Result:%i\n",
			hr&0xffff);
	}

	size = dsd.ddpfPixelFormat.dwZBufferBitDepth/8;
	mask = dsd.ddpfPixelFormat.dwZBitMask;
	if(!mask) // What?!
		mask = size==2? 0xffff : size==3? 0xffffff : 0xffffffff;
	unpackMask(mask, &offset, &bits);
	for(maxDepth=0, i=0; i<bits; i++) maxDepth |= 1<<i;

	// Check the type.
	switch(type)
	{
	case DGL_SINGLE_PIXELS:
		num = inData[1];
		coords = inData + 2;
		for(i=0; i<num; i++, coords+=2)
		{
			temp = 0;
			memcpy(&temp, (byte*) dsd.lpSurface + coords[VY]*dsd.lPitch + coords[VX]*size, size);
			fv[i] = ((temp & mask) >> offset) / (float) maxDepth;
		}
		break;
	
	case DGL_BLOCK:
		coords = inData + 1;
		returnValue = DGL_UNSUPPORTED;
		break;

	default:
		returnValue = DGL_UNSUPPORTED;
	}
	
	IDirectDrawSurface4_Unlock(sDepth, NULL);
	return returnValue;
	unguard;
}


int Gamma(int set, DGLubyte *data)
{
	guard(Gamma);
	int			i;
	DDGAMMARAMP ramp;

	if(!gammaSupport || !ddGamma) return DGL_UNSUPPORTED;
	if(!data) return DGL_ERROR;
	if(set)
	{
		for(i=0; i<256; i++)
			ramp.red[i] = ramp.green[i] = ramp.blue[i] = data[i]*65535/255;
		IDirectDrawGammaControl_SetGammaRamp(ddGamma, 0, &ramp);	
	}
	else
	{
		// Not supported.
		return DGL_ERROR;
	}
	return DGL_OK;
	unguard;
}

} // namespace Direct3DDriver