// drOpenGL.dll
// The Doomsday graphics library driver for OpenGL
//
// texture.c : Texture routines

#include <stdio.h>
#include <stdlib.h>
#include "drOpenGL.h"
#include "../DoomsDay/tga.h"

rgba_t	palette[256];
int		usePalTex = DGL_FALSE;
int		dumpTextures = DGL_FALSE;

// The color table extension.
PFNGLCOLORTABLEEXTPROC glColorTableEXT = NULL;


void loadPalette()
{
	byte	paldata[256*3];
	int		i;

	if(usePalTex == DGL_FALSE) return;

	memset(paldata, 0, 256*3);
	// Prepare the color table (RGBA -> RGB).
	for(i=0; i<256; i++)
	{
		memcpy(paldata + i*3, palette[i].color, 3);
	}
//	glColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, 
//		paldata);
}


int enablePalTexExt(int enable)
{
	if(!paltexExtAvailable) return DGL_FALSE;
	if(enable && usePalTex || !enable && !usePalTex) return DGL_TRUE;

	if(!enable && usePalTex)
	{
		usePalTex = DGL_FALSE;
//		glDisable(GL_SHARED_TEXTURE_PALETTE_EXT);
		glColorTableEXT = NULL;
		return DGL_TRUE;
	}	

	usePalTex = DGL_FALSE;

	if((glColorTableEXT = (PFNGLCOLORTABLEEXTPROC) wglGetProcAddress("glColorTableEXT")) == NULL)
	{
		gim.Message("drOpenGL.enablePalTexExt: getProcAddress failed.\n");
		return DGL_FALSE;
	}

//	glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
	usePalTex = DGL_TRUE;
	loadPalette();
	return DGL_TRUE;
}	


int Power2(int num)
{
	int cumul;
	for(cumul=1; num > cumul; cumul <<= 1);
	return cumul;
}


DGLuint NewTexture(void)
{
	DGLuint texName;

	// Generate a new texture name and bind it.
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, currentTex = texName);
	return texName;
}


// Width and height must be powers of two.
int LoadTexture(int format, int width, int height, int mipmap, void *data)
{
	byte *bdata = (byte *)data;

	// Can't operate on the null texture.
	if(!currentTex || mipmap < 0) return DGL_FALSE;

	// Check that the texture dimensions are valid.
	if(width != Power2(width) || height != Power2(height))
		return DGL_FALSE;

	if(width > maxTexSize || height > maxTexSize)
		return DGL_FALSE;

	if(usePalTex && format == DGL_COLOR_INDEX_8) // Paletted texture?
	{
		glTexImage2D(GL_TEXTURE_2D, mipmap, GL_COLOR_INDEX8_EXT, width, height, 
			0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, data);
	}
	else // Use true color textures.
	{
		int alphachannel = (format==DGL_RGBA) || (format==DGL_COLOR_INDEX_8_PLUS_A8);
		int i, colorComps = alphachannel? 4 : 3;
		int numPixels = width * height;
		byte *buffer;
		byte *pixel;
		int needFree = DGL_FALSE;

		if(format == DGL_RGBA)
		{
			buffer = (byte *)data;
		}
		else 
		{
			buffer = (byte *)malloc(numPixels * 4);
			needFree = DGL_TRUE;
			for(i=0, pixel=buffer; i<numPixels; i++, pixel+=4)
			{
				if(format == DGL_RGB)
				{
					memcpy(pixel, (byte*) data + i*3, 3);
					pixel[CA] = 0xff;
				}
				else if(format == DGL_COLOR_INDEX_8)
				{
					memcpy(pixel, palette[bdata[i]].color, 3);
					pixel[CA] = 0xff;
				}
				else if(format == DGL_COLOR_INDEX_8_PLUS_A8)
				{	
					memcpy(pixel, palette[bdata[i]].color, 3);
					pixel[CA] = bdata[numPixels+i];
				}
				else if(format == DGL_LUMINANCE)
				{
					pixel[CR] = pixel[CG] = pixel[CB] = bdata[i];
					pixel[CA] = 0xff;
				}
			}
		}
		glTexImage2D(GL_TEXTURE_2D, mipmap, colorComps, width, height, 0, 
			GL_RGBA, GL_UNSIGNED_BYTE, buffer);

		if(dumpTextures && !mipmap)
		{
			char fn[100];
			sprintf(fn, "%03ic%iw%03ih%03i.tga",
				currentTex, colorComps, width, height);
			saveTGA24_rgba8888(fn, width, height, buffer);
		}

		if(needFree) free(buffer);
	}
	return DGL_OK;
}


void DeleteTextures(int num, DGLuint *names)
{
	int		i;

	if(!num || !names) return;
	for(i=0; i<num; i++)
		if(currentTex == names[i]) 
		{
			currentTex = 0;
			break;		
		}
	glDeleteTextures(num, names);
}

	
void TexParam(int pname, int param)
{
	GLenum mlevs[] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR };

	glTexParameteri(GL_TEXTURE_2D, pname==DGL_MIN_FILTER? GL_TEXTURE_MIN_FILTER
		: pname==DGL_MAG_FILTER? GL_TEXTURE_MAG_FILTER
		: pname==DGL_WRAP_S? GL_TEXTURE_WRAP_S
		: GL_TEXTURE_WRAP_T,
		
		(param>=DGL_NEAREST && param<=DGL_LINEAR_MIPMAP_LINEAR)? mlevs[param-DGL_NEAREST]
		: param==DGL_CLAMP? GL_CLAMP
		: GL_REPEAT);
}


void GetTexParameterv(int level, int pname, int *v)
{
	switch(pname)
	{
	case DGL_WIDTH:
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, v);
		break;

	case DGL_HEIGHT:
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, v);
		break;
	}
}


void Palette(int format, void *data)
{
	unsigned char	*ptr = (byte *)data;
	int				i, size = (format==DGL_RGBA? 4 : 3);

	for(i=0; i<256; i++, ptr += size)
	{
		palette[i].color[CR] = ptr[CR];
		palette[i].color[CG] = ptr[CG];
		palette[i].color[CB] = ptr[CB];
		palette[i].color[CA] = format==DGL_RGBA? ptr[CA] : 0xff;
	}
	loadPalette();
}


int	Bind(DGLuint texture)
{
	DGLuint	oldtex = currentTex;

	// Do we need to change it?
	if(texture != currentTex)
		glBindTexture(GL_TEXTURE_2D, currentTex = texture);
	return oldtex;
}
