
//**************************************************************************
//**
//** GL_TEX.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdlib.h>
#include <math.h>
#include "gl_s3tc.h"
#include "dd_def.h"
#include "i_win32.h"
#include "gl_def.h"
#include "console.h"

// MACROS ------------------------------------------------------------------

#define TEXQ_BEST	8
#define NUM_FLARES	3

#define RGB18(r, g, b) ((r)+((g)<<6)+((b)<<12))

// TYPES -------------------------------------------------------------------

// A translated sprite.
typedef struct
{
	int				patch;
	DGLuint			tex;
	unsigned char	*table;
} transspr_t;

// Sprite color (for dynamic lights, if the sprite is fullbright).
typedef struct
{
	unsigned char	rgb[3];
} rgbcol_t;

// Sky texture topline colors.
typedef struct
{
	int				texidx;
	unsigned char	rgb[3];
} skycol_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int maxTexSize;		// Maximum supported texture size.
extern int ratioLimit;
extern int test3dfx;
extern boolean palettedTextureExtAvailable;
extern boolean s3tcAvailable;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean filloutlines = true;
boolean	paletted = false;	// Use GL_EXT_paletted_texture (cvar)
boolean load8bit = false;	// Load textures as 8 bit? (with paltex)
//boolean r_s3tc = false;		// Use GL_S3_s3tc (cvar)
//boolean useS3TC = false;	// GL_S3_s3tc is inited and in use

// Convert a 18-bit RGB (666) value to a playpal index. Too big?
byte	pal18to8[262144];	

int		mipmapping=3, linearRaw=1, texQuality = TEXQ_BEST; 
int		filterSprites = true;

int		pallump;
int		mynumlumps;

// Properties of the current texture.
float	texw=1, texh=1;
int		texmask=0;	
DGLuint	curtex = 0;

texsize_t *lumptexsizes;	// Sizes for all the lumps. 

skycol_t *skytop_colors = NULL;
int num_skytop_colors = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// Texture names for all lumps. Although the list will contain names for 
// ALL lumps, only the graphics entries that aren't flats, wall textures or 
// sprites are used.
static DGLuint		*lumptexnames, *lumptexnames2;	// Support for two parts.
static int			*rawlumps, numrawlumps;	// Raw screen lumps (just lump numbers).
	
static DGLuint		*flattexnames, *texnames, *spritenames;
static rgbcol_t		*spritecolors;
static char			*texmasked;		// 1 if the texture is masked.

// The translated sprites.
static transspr_t	*transsprites;
static int			numtranssprites;

static DGLuint		dltexname;	// Name of the dynamic light texture.
static DGLuint		flaretexnames[NUM_FLARES];

static boolean		gammaSupport = false;

static int glmode[6] = // Indexed by 'mipmapping'.
{ 
	DGL_NEAREST,
	DGL_LINEAR,
	DGL_NEAREST_MIPMAP_NEAREST,
	DGL_LINEAR_MIPMAP_NEAREST,
	DGL_NEAREST_MIPMAP_LINEAR,
	DGL_LINEAR_MIPMAP_LINEAR
};


// CODE --------------------------------------------------------------------

// Finds the power of 2 that is equal to or greater than 
// the specified number.
int CeilPow2(int num)
{
	int cumul;
	for(cumul=1; num > cumul; cumul <<= 1);
	return cumul;
}

// Finds the power of 2 that is less than or equal to
// the specified number.
int FloorPow2(int num)
{
	int fl = CeilPow2(num);
	if(fl > num) fl >>= 1;
	return fl;
}

// Finds the power of 2 that is nearest the specified number.
// In ambiguous cases, the smaller number is returned.
int RoundPow2(int num)
{
	int cp2 = CeilPow2(num), fp2 = FloorPow2(num);
	return (cp2-num >= num-fp2)? fp2 : cp2;
}

// Weighted rounding. Weight determines the point where 
// the number is still rounded down.
int WeightPow2(int num, float weight)
{
	int		fp2 = FloorPow2(num);
	float	frac = (num - fp2) / (float) fp2;

	if(frac <= weight) return fp2; else return (fp2<<1);
}


// Copies a rectangular region of the source buffer to the destination
// buffer. Doesn't perform clipping, so be careful.
// Yeah, 13 parameters.
static void pixBlt(byte *src, int srcWidth, int srcHeight, byte *dest, 
				   int destWidth, int destHeight,
				   int alpha, int srcRegX, int srcRegY, int destRegX, int destRegY,
				   int regWidth, int regHeight)
{
	int	y;	// Y in the copy region.
	int srcNumPels = srcWidth*srcHeight;
	int destNumPels = destWidth*destHeight;

	for(y=0; y<regHeight; y++) // Copy line by line.
	{
		// The color index data.
		memcpy(dest + destRegX + (y+destRegY)*destWidth,
			src + srcRegX + (y+srcRegY)*srcWidth, 
			regWidth);

		if(alpha)
		{
			// Alpha channel data.
			memcpy(dest + destNumPels + destRegX + (y+destRegY)*destWidth,
				src + srcNumPels + srcRegX + (y+srcRegY)*srcWidth,	
				regWidth);
		}
	}
}

// Prepare the pal18to8 table.
static void LookupPal18to8(byte *palette)
{
	int				r, g, b, i;
	byte			palRGB[3];
	unsigned int	diff, smallestDiff, closestIndex;
		
	// This is a time-consuming operation.
	for(r=0; r<64; r++)
		for(g=0; g<64; g++)
			for(b=0; b<64; b++)
			{
				// We must find the color index that most closely
				// resembles this RGB combination.
				smallestDiff = -1;
				for(i=0; i<256; i++)
				{
					memcpy(palRGB, palette + 3*i, 3);
					diff = (palRGB[0]-(r<<2))*(palRGB[0]-(r<<2)) 
						+ (palRGB[1]-(g<<2))*(palRGB[1]-(g<<2)) 
						+ (palRGB[2]-(b<<2))*(palRGB[2]-(b<<2));
					if(diff < smallestDiff) 
					{
						smallestDiff = diff;
						closestIndex = i;
					}
				}
				pal18to8[RGB18(r, g, b)] = closestIndex;
			}

	if(M_CheckParm("-dump_pal18to8"))
	{
		FILE *file = fopen("Pal18to8.lmp", "wb");
		fwrite(pal18to8, sizeof(pal18to8), 1, file);								
		fclose(file);
	}
}

static void LoadPalette()
{
	byte	*playpal = (byte *)W_CacheLumpNum(pallump=W_GetNumForName("PLAYPAL"), PU_CACHE);
	byte	paldata[256 * 3];
	int		i, c;
	int		gammalevel = gammaSupport? 0 : usegamma;

	memset(paldata, 0, 256*3);
	// Prepare the color table.
	for(i=0; i<256; i++)
	{
		// Adjust the values for the appropriate gamma level.
		for(c=0; c<3; c++) paldata[i*3 + c] = gammatable[gammalevel][playpal[i*3 + c]];
	}
	gl.Palette(DGL_RGB, paldata);
}

// Initializes the paletted texture extension.
// Returns true if successful.
int GL_InitPalettedTexture()
{
	int		i;

	// Do we need to generate a pal18to8 table?
	if(M_CheckParm("-dump_pal18to8"))
		LookupPal18to8((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE));

	// Should the extension be used?
	if(!paletted && !M_CheckParm("-paltex")) 
		return true;

	gl.Enable(DGL_PALETTED_TEXTURES);

	// Check if the operation was a success.
	gl.GetIntegerv(DGL_PALETTED_TEXTURES, &i);
	if(i == DGL_FALSE)
	{
		ST_Message("\nPaletted textures init failed!\n");
		return false;
	}
	// Textures must be uploaded as 8-bit, now.
	load8bit = true;
	return true;
}

void GL_TexInit()
{
	int		i;

	gammaSupport = (gl.Gamma(DGL_TRUE, 0) != DGL_UNSUPPORTED);
	if(M_CheckParm("-nogamma")) gammaSupport = false;

	// Allocate memory for the flat texture numbers.
	flattexnames = (DGLuint *)Z_Malloc(sizeof(DGLuint)*numflats, PU_OPENGL, 0);	
	memset(flattexnames, 0, sizeof(DGLuint)*numflats);

	texnames = (DGLuint *)Z_Malloc(sizeof(DGLuint)*numtextures, PU_OPENGL, 0);
	memset(texnames, 0, sizeof(DGLuint)*numtextures);

	texmasked = (char *)Z_Malloc(numtextures, PU_OPENGL, 0);
	memset(texmasked, 0, numtextures);

	// Sprites.
	spritenames = (DGLuint *)Z_Malloc(sizeof(DGLuint)*numspritelumps, PU_OPENGL, 0);
	memset(spritenames, 0, sizeof(DGLuint)*numspritelumps);	
//	spriteheights = Z_Malloc(sizeof(short)*numspritelumps, PU_OPENGL, 0);
//	memset(spriteheights, 0, sizeof(short)*numspritelumps);
	spritecolors = (rgbcol_t *)Z_Malloc(sizeof(rgbcol_t)*numspritelumps, PU_OPENGL, 0);
	memset(spritecolors, 0, sizeof(rgbcol_t)*numspritelumps);
	
	transsprites = 0;
	numtranssprites = 0;

	// Standard lump textures (raw images and other gfx).
	// Numlumps changes when data is loaded at runtime.
	mynumlumps = numlumps;
	// First parts.
	lumptexnames = (DGLuint *)Z_Malloc(sizeof(DGLuint)*numlumps, PU_OPENGL, 0);
	memset(lumptexnames, 0, sizeof(DGLuint)*numlumps);
	// Second parts.
	lumptexnames2 = (DGLuint *)Z_Malloc(sizeof(DGLuint)*numlumps, PU_OPENGL, 0);
	memset(lumptexnames2, 0, sizeof(DGLuint)*numlumps);
	// Size data.
	lumptexsizes = (texsize_t *)Z_Malloc(sizeof(texsize_t)*numlumps, PU_OPENGL, 0);
	memset(lumptexsizes, 0, sizeof(texsize_t)*numlumps);

	// Raw screen lump book keeping.
	rawlumps = 0;
	numrawlumps = 0;

	// The dynamic light map.
	dltexname = 0;
	memset(flaretexnames, 0, sizeof(flaretexnames));

	// The palette lump, for color information (really??!!?!?!).
	pallump = W_GetNumForName("PLAYPAL");

	GL_InitPalettedTexture();

	// DGL needs the palette information regardless of whether the
	// paletted textures are enabled or not.
	LoadPalette();
	if(gammaSupport) GL_UpdateGamma();

	// Load the pal18to8 table from the lump PAL18TO8. We need it
	// when resizing textures.
	if((i=W_CheckNumForName("PAL18TO8")) == -1)
	{
		LookupPal18to8((byte *)W_CacheLumpNum(pallump, PU_CACHE));
	}
	else
	{
		memcpy(pal18to8, W_CacheLumpNum(i, PU_CACHE), sizeof(pal18to8));
	}
	dltexname = GL_PrepareLightTexture();
	// DGL's list renderer must know the light texture.
	gl.SetInteger(DGL_LIGHT_TEXTURE, dltexname);
}

void GL_TexDestroy()
{
	GL_TexReset();
	GL_ResetLumpTexData();
	Z_FreeTags(PU_OPENGL, PU_OPENGL);
}

void GL_TexReset(void)
{
	int					i;

	if(flattexnames)
	{
		gl.DeleteTextures(numflats, flattexnames);
		memset(flattexnames, 0, sizeof(DGLuint)*numflats);
	}

	if(texnames)
	{
		gl.DeleteTextures(numtextures, texnames);
		memset(texnames, 0, sizeof(DGLuint)*numtextures);
	}
	if(texmasked)	
		memset(texmasked, 0, numtextures);

	if(spritenames)
	{
		gl.DeleteTextures(numspritelumps, spritenames);
		memset(spritenames, 0, sizeof(DGLuint)*numspritelumps);
		//memset(spriteheights, 0, sizeof(short)*numspritelumps);
		memset(spritecolors, 0, sizeof(rgbcol_t)*numspritelumps);
	}

	// Delete the translated sprite textures.
	for(i=0; i<numtranssprites; i++)
	{
		gl.DeleteTextures(1, &transsprites[i].tex);
	}
	free(transsprites);
	transsprites = 0;
	numtranssprites = 0;

	free(skytop_colors);
	skytop_colors = 0;
	num_skytop_colors = 0;

	gl.DeleteTextures(1, &dltexname);
	dltexname = 0;
	gl.SetInteger(DGL_LIGHT_TEXTURE, dltexname = GL_PrepareLightTexture());
	
	if(flaretexnames)
	{
		gl.DeleteTextures(NUM_FLARES, flaretexnames);
		memset(flaretexnames, 0, sizeof(flaretexnames));
	}

	// Skins.
	for(i=1; i<MAX_MODELS; i++)
		if(modellist[i])
		{
			model_t *mdl = modellist[i];
			gl.DeleteTextures(mdl->header.numSkins, mdl->skintex);
			memset(mdl->skintex, 0, sizeof(unsigned int) * mdl->header.numSkins);
		}
}

void GL_ResetLumpTexData()
{
	gl.DeleteTextures(mynumlumps, lumptexnames);
	gl.DeleteTextures(mynumlumps, lumptexnames2);
	memset(lumptexnames, 0, sizeof(DGLuint)*mynumlumps);
	memset(lumptexnames2, 0, sizeof(DGLuint)*mynumlumps);
	memset(lumptexsizes, 0, sizeof(texsize_t)*mynumlumps);

	// Free the raw lumps book keeping table.
	free(rawlumps);
	rawlumps = 0;
	numrawlumps = 0;
}

void GL_ClearTextureMemory(void)
{
	GL_TexReset();
	GL_ResetLumpTexData();
}

void GL_UpdateGamma(void)
{
	if(gammaSupport)
	{
		// The driver knows how to update the gamma directly.
		gl.Gamma(DGL_TRUE, gammatable[usegamma]);
	}
	else
	{
		LoadPalette();
		GL_ClearTextureMemory();
	}
}

// Binds the texture if necessary.
void GL_BindTexture(DGLuint texname)
{
	if(curtex != texname) 
	{
		gl.Bind(texname);
		curtex = texname;
	}
}

/*static void PalToRGB(byte *palidx, byte *rgb)
{
	int i;
	
	if(gammaSupport)
	{
		rgb[0] = palidx[0];
		rgb[1] = palidx[1];
		rgb[2] = palidx[2];
	}
	else for(i=0; i<3; i++) 
	{
		*(rgb+i) = gammatable[usegamma][*(palidx+i)];
	}
}*/

void PalIdxToRGB(byte *pal, int idx, byte *rgb)
{
	int c;
	int gammalevel = gammaSupport? 0 : usegamma;

	for(c=0; c<3; c++) // Red, green and blue.
	{
		rgb[c] = gammatable[gammalevel][pal[idx*3 + c]];
	}
}

// in/outformat:
// 1 = palette indices
// 2 = palette indices followed by alpha values
// 3 = RGB 
// 4 = RGBA
static void ConvertBuffer(int width, int height, int informat, int outformat, 
						  byte *in, byte *out, boolean gamma)
{
	byte	*palette = (byte *)W_CacheLumpName("playpal", PU_CACHE);
	int		inSize = informat==2? 1 : informat;
	int		outSize = outformat==2? 1 : outformat;
	int		i, numPixels = width * height, a;
	
	if(informat == outformat) 
	{
		// No conversion necessary.
		memcpy(out, in, numPixels * informat);
		return;
	}
	// Conversion from pal8(a) to RGB(A).
	if(informat <= 2 && outformat >= 3)
	{
		for(i=0; i<numPixels; i++, in += inSize, out += outSize) 
		{
			// Copy the RGB values in every case.
			if(gamma)
			{
				for(a=0; a<3; a++) out[a] = gammatable[usegamma][*(palette + 3*(*in) + a)];
			}
			else
				memcpy(out, palette + 3*(*in), 3);
			// Will the alpha channel be necessary?
			a = 0;
			if(informat == 2) a = in[numPixels*inSize];
			if(outformat == 4) out[3] = a;
		}
	}
	// Conversion from RGB(A) to pal8(a), using pal18to8.
	else if(informat >= 3 && outformat <= 2)
	{
		for(i=0; i<numPixels; i++, in += inSize, out += outSize)
		{
			// Convert the color value.
			*out = pal18to8[RGB18(in[0]>>2, in[1]>>2, in[2]>>2)];
			// Alpha channel?
			a = 0;
			if(informat == 4) a = in[3];
			if(outformat == 2) out[numPixels*outSize] = a;
		}
	}
	else if(informat == 3 && outformat == 4)
	{
		for(i=0; i<numPixels; i++, in += inSize, out += outSize)
		{
			memcpy(out, in, 3);
			out[3] = 0;
		}
	}
}

// Len is in measured in out units. Comps is the number of components per pixel,
// or rather the number of bytes per pixel (3 or 4). The strides must be 
// byte-aligned anyway, though; not in pixels.
static void scaleLine(byte *in, int inStride, byte *out, int outStride,
					  int outLen, int inLen, int comps)
{
	int		i, c;
	float	inToOutScale = outLen / (float) inLen;

	if(inToOutScale > 1)	
	{
		// Magnification is done using linear interpolation.
		fixed_t inPosDelta = (FRACUNIT*(inLen-1))/(outLen-1), inPos = inPosDelta;
		byte *col1, *col2;
		int weight, invWeight;
		// The first pixel.
		memcpy(out, in, comps);
		out += outStride;
		// Step at each out pixel between the first and last ones.
		for(i=1; i<outLen-1; i++, out += outStride, inPos += inPosDelta)
		{
			col1 = in + (inPos >> FRACBITS) * inStride;
			col2 = col1 + inStride;
			weight = inPos & 0xffff;
			invWeight = 0x10000 - weight;
			out[0] = (col1[0]*invWeight + col2[0]*weight) >> 16;
			out[1] = (col1[1]*invWeight + col2[1]*weight) >> 16;
			out[2] = (col1[2]*invWeight + col2[2]*weight) >> 16;
			if(comps == 4)
				out[3] = (col1[3]*invWeight + col2[3]*weight) >> 16;
		}
		// The last pixel.
		memcpy(out, in + (inLen-1)*inStride, comps);
	}
	else if(inToOutScale < 1)
	{
		// Minification needs to calculate the average of each of 
		// the pixels contained by the out pixel.
		unsigned int cumul[4] = {0, 0, 0, 0}, count = 0;
		int outpos = 0;
		for(i=0; i<inLen; i++, in += inStride)
		{
			if((int) (i*inToOutScale) != outpos)
			{
				outpos = (int) i*inToOutScale;
				for(c=0; c<comps; c++) 
				{
					out[c] = cumul[c] / count;
					cumul[c] = 0;
				}
				count = 0;
				out += outStride;
			}
			for(c=0; c<comps; c++) cumul[c] += in[c];
			count++;
		}
		// Fill in the last pixel, too.
		for(c=0; c<comps; c++) out[c] = cumul[c] / count;
	}
	else 
	{
		// No need for scaling.
		for(i=0; i<outLen; i++, out += outStride, in += inStride)
			memcpy(out, in, comps);
	}
}

static void ScaleBuffer32(byte *in, int inWidth, int inHeight,
						  byte *out, int outWidth, int outHeight, int comps)
{
	int		i;
	byte	*temp = (byte *)Z_Malloc(outWidth * inHeight * comps, PU_STATIC, 0);

	// First scale horizontally, to outWidth, into the temporary buffer.
	for(i=0; i<inHeight; i++)
	{
		scaleLine(in + inWidth*comps*i, comps, temp + outWidth*comps*i, comps, outWidth, 
			inWidth, comps);
	}
	// Then scale vertically, to outHeight, into the out buffer.
	for(i=0; i<outWidth; i++)
	{
		scaleLine(temp + comps*i, outWidth*comps, out + comps*i, outWidth*comps, outHeight,
			inHeight, comps);
	}
	Z_Free(temp);
}			

// Works within the given data, reducing the size of the picture to half 
// its original. Width and height must be powers of two.
static void DownMipmap32(byte *in, int width, int height, int comps)
{
	byte	*out;
	int		x, y, c, outW = width>>1, outH = height>>1;

	if(width == 1 && height == 1)
		I_Error("DownMipmap32 can't be called for a 1x1 image.\n");

	if(!outW || !outH) // Limited, 1x2|2x1 -> 1x1 reduction?
	{
		int outDim = width > 1? width : height;
		out = in;
		for(x=0; x<outDim; x++, in += comps*2)
			for(c=0; c<comps; c++, out++) 
				*out = (in[c] + in[comps+c]) >> 1;
	}
	else // Unconstrained, 2x2 -> 1x1 reduction?
	{
		out = in;
		for(y=0; y<outH; y++, in += width*comps)
			for(x=0; x<outW; x++, in += comps*2)
				for(c=0; c<comps; c++, out++) 
					*out = (in[c] + in[comps + c] + in[comps*width + c] + in[comps*(width+1) + c]) >> 2;
	}
}	

// Returns the name of the texture. 
// The texture parameters will NOT be set here.
// 'data' contains indices to the playpal. If 'alphachannel' is true,
// 'data' also contains the alpha values (after the indices).
static DGLuint UploadTexture(byte *data, int width, int height, boolean alphaChannel, 
						  boolean generateMipmaps, boolean RGBData)
{
	int		format = alphaChannel? DGL_COLOR_INDEX_8_PLUS_A8 : DGL_COLOR_INDEX_8;
	int		i, levelWidth, levelHeight;	// width and height at the current level
	byte	*buffer, *rgbaOriginal, *idxBuffer;
	DGLuint	texName;

	if(RGBData) format = alphaChannel? DGL_RGBA : DGL_RGB;
	
	// Determine the most favorable size for the texture.
	if(texQuality == TEXQ_BEST)	// The best quality.
	{
		// At the best texture quality level, all textures are
		// sized *upwards*, so no details are lost. This takes
		// more memory, but naturally looks better.
		levelWidth = CeilPow2(width);
		levelHeight = CeilPow2(height);
	}
	else if(texQuality == 0)
	{
		// At the lowest quality, all textures are sized down
		// to the nearest power of 2.
		levelWidth = FloorPow2(width);
		levelHeight = FloorPow2(height);
	}
	else 
	{
		// At the other quality levels, a weighted rounding
		// is used.
		levelWidth = WeightPow2(width, 1 - texQuality/(float)TEXQ_BEST);
		levelHeight = WeightPow2(height, 1 - texQuality/(float)TEXQ_BEST);
	}

	// Hardware limitations may force us to modify the preferred
	// texture size.
	if(levelWidth > maxTexSize) levelWidth = maxTexSize;
	if(levelHeight > maxTexSize) levelHeight = maxTexSize;
	if(ratioLimit)
	{
		if(levelWidth > levelHeight) // Wide texture.
		{
			if(levelHeight < levelWidth/ratioLimit)
				levelHeight = levelWidth / ratioLimit;
		}
		else // Tall texture.
		{
			if(levelWidth < levelHeight/ratioLimit)
				levelWidth = levelHeight / ratioLimit;
		}
	}
	
	// Get the RGB(A) version of the original texture.
	rgbaOriginal = (byte *)Z_Malloc(width * height * (alphaChannel? 4 : 3), PU_STATIC, 0);
	if(RGBData)
	{
		memcpy(rgbaOriginal, data, width * height * (alphaChannel? 4 : 3));
	}
	else
	{
		ConvertBuffer(width, height, alphaChannel? 2 : 1, alphaChannel? 4 : 3, 
			data, rgbaOriginal,	!load8bit && !gammaSupport);
	}
	
	// Prepare the RGB(A) buffer for the texture.
	if(width != levelWidth || height != levelHeight)
	{
		buffer = (byte *)Z_Malloc(levelWidth * levelHeight * (alphaChannel?4:3), PU_STATIC, 0);
		ScaleBuffer32(rgbaOriginal, width, height, buffer, levelWidth, levelHeight, 
			alphaChannel? 4 : 3);
		Z_Free(rgbaOriginal);
	}
	else
	{
		buffer = rgbaOriginal;
	}

	if(load8bit)
	{
		// Prepare the palette indices buffer, to be handed over to DGL. 
		idxBuffer = (byte *)Z_Malloc(levelWidth * levelHeight * (alphaChannel? 2 : 1), PU_STATIC, 0);
	}

	// Generate a new texture name and bind it.
	texName = gl.NewTexture();
	for(i=0; levelWidth || levelHeight; i++)
	{
		if(!levelWidth) levelWidth = 1;
		if(!levelHeight) levelHeight = 1;

		if(load8bit)
		{
			// Convert to palette indices.
			ConvertBuffer(levelWidth, levelHeight, alphaChannel? 4 : 3,
				alphaChannel? 2 : 1, buffer, idxBuffer, false);
			// Upload it.
			if(gl.TexImage(format, levelWidth, levelHeight, i, idxBuffer) != DGL_OK)
			{
				I_Error("LoadTexture: TexImage failed (%i x %i) as 8-bit, alpha:%i\n",
					levelWidth, levelHeight, alphaChannel);
			}			
		}
		else
		{
			if(gl.TexImage(alphaChannel? DGL_RGBA : DGL_RGB, levelWidth, levelHeight, i, buffer) != DGL_OK)
			{
				I_Error("LoadTexture: TexImage failed (%i x %i), alpha:%i\n",
					levelWidth, levelHeight, alphaChannel);
			}
		}

		// If no mipmaps need to generated, quit now.
		if(!generateMipmaps) break;

		if(levelWidth > 1 || levelHeight > 1)
			DownMipmap32(buffer, levelWidth, levelHeight, alphaChannel? 4 : 3);

		// Move on.
		levelWidth >>= 1;
		levelHeight >>= 1;
	}
	//printf( "Done with %i.\n", texName);

	Z_Free(buffer);
	if(load8bit) Z_Free(idxBuffer);

	return texName;
}

unsigned int GL_BindTexFlat(int lump)
{
	DGLuint name;
	byte *flatptr;
	
	if(lumpinfo[lump].size < 4096)
	{
		GL_BindTexture(0);	
		return 0;
	}
	// Get a pointer to the texture data.
	flatptr = (byte *)W_CacheLumpNum(lump, PU_STATIC);
	
	// Load the texture.
	name = UploadTexture(flatptr, 64, 64, false, !test3dfx, false);
	
	// Set the parameters.
	gl.TexParameter(DGL_MIN_FILTER, glmode[mipmapping]);
	gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
	
	Z_ChangeTag(flatptr, PU_CACHE);
	// The name of the texture is returned.
	return name;
}

// Returns the OpenGL name of the texture.
unsigned int GL_PrepareFlat(int idx)
{
	if(idx < 0 || idx > numflats-1) idx = 0;

	idx = flattranslation[idx];
	if(!flattexnames[idx])
	{
		// The flat isn't yet bound with OpenGL.
		flattexnames[idx] = GL_BindTexFlat(firstflat+idx);
	}
	texw = texh = 64;
	texmask = 0;
	return flattexnames[idx];
}

void GL_SetFlat(int idx)
{
	gl.Bind(curtex = GL_PrepareFlat(idx));
}

// If maskZero is true, the buffer must have room for the alpha values.
// Returns true if the buffer really has alpha information.
static int DrawRealPatch(byte *buffer, byte *palette, int texwidth,
						 int texheight, patch_t *patch, boolean maskZero,
						 unsigned char *transtable)
{
	int			count;
	int			col;
	column_t	*column;
	byte		*destTop, *destAlphaTop = NULL;
	byte		*dest1, *dest2;
	byte		*source;
	int			w, i, bufsize = texwidth*texheight;

	col = 0;
	destTop = buffer;
	destAlphaTop = buffer + texwidth*texheight;
	w = SHORT(patch->width);
	for(; col < w; col++, destTop++, destAlphaTop++)
	{
		column = (column_t *)((byte *)patch+LONG(patch->columnofs[col]));
		// Step through the posts in a column
		while(column->topdelta != 0xff)
		{
			source = (byte *)column+3;
			
			dest1 = destTop + column->topdelta*texwidth;
			dest2 = destAlphaTop + column->topdelta*texwidth;

			count = column->length;
			while(count--)
			{
				unsigned char palidx = *source++;
				// Do we need to make a translation?
				if(transtable) palidx = transtable[palidx];

				// Out of the buffer? This happens if the
				// patch is larger than the texture.
				if(dest1-buffer >= bufsize) break;

#ifdef RANGECHECK
				if(dest1-buffer > texwidth*texheight)
					I_Error("DrawRealPatch: Out of bounds. col:%i patch:%ix%i buf:%ix%i dest1:%p buffer:%p diff:%i texwidth*texheight:%i\n",
					col, w, patch->height, texwidth, texheight, dest1, buffer, dest1-buffer, texwidth*texheight);
				if(dest2-buffer > 2*texwidth*texheight)
					I_Error("DrawRealPatch: Alpha out of bounds.\n");
#endif

				if(!maskZero || palidx)
					*dest1 = palidx;
				dest1 += texwidth; // One row down.
				
				if(maskZero)
					*dest2 = palidx? 0xff : 0;
				else
					*dest2 = 0xff;
				dest2 += texwidth; // One row down.
			}
			column = (column_t *)((byte *)column+column->length+4);
		}
	}
	// Scan through the RGBA buffer and check for sub-0xff alpha.
	source = buffer + texwidth*texheight;
	for(i=0; i<texwidth*texheight; i++)
		if(source[i] < 0xff) return true; // Has alpha data.
	return false; // Doesn't have alpha data.
}

// Translate colors in the specified patch.
void TranslatePatch(patch_t *patch, byte *transTable)
{
	int			count;
	int			col = 0;
	column_t	*column;
	byte		*source;
	int			w = SHORT(patch->width);

	for(; col < w; col++)
	{
		column = (column_t *)((byte *)patch+LONG(patch->columnofs[col]));
		// Step through the posts in a column
		while(column->topdelta != 0xff)
		{
			source = (byte*) column+3;
			count = column->length;
			while(count--)
			{
				*source = transTable[*source];
				source++;
			}
			column = (column_t *)((byte *)column+column->length+4);
		}
	}
}

// Returns the OpenGL texture name.
unsigned int GL_PrepareTexture(int idx)
{
	if(idx == 0)//|| idx > numtextures-1)
	{
		// No texture?
		texw = 1;
		texh = 1;
		texmask = 0;
		return 0;
	}
	idx = texturetranslation[idx];
	if(!texnames[idx])
	{
		// The texture must be given to OpenGL.
		int			i, k;
		boolean		alphaChannel = false;
		texture_t	*tex = textures[idx];
		byte		*palette = (byte *)W_CacheLumpNum(pallump, PU_CACHE);
		byte		*buffer = (byte *)Z_Malloc(2 * tex->width * tex->height, PU_STATIC, 0);
		byte		*colptr;
	
		if(tex->patchcount > 1)
		{
			// The texture will have no alpha information.
			for(i=0; i<tex->width; i++)
			{
				byte *pixel = buffer + i;
				colptr = R_GetColumn(idx,i);
				for(k=0; k<tex->height; k++, pixel+=tex->width)
				{
					*pixel = colptr[k];
				}
			}
		}
		else 
		{
			// This texture has only only one patch. It might be masked.
			memset(buffer, 0, 2*tex->width*tex->height);
			alphaChannel = DrawRealPatch(buffer, palette, tex->width, tex->height,
				(patch_t *)W_CacheLumpNum(tex->patches[0].patch, PU_CACHE), false, 0);
		}
		texnames[idx] = UploadTexture(buffer, tex->width, tex->height, alphaChannel, true, false);
		gl.TexParameter(DGL_MIN_FILTER, glmode[mipmapping]);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);

		texmasked[idx] = alphaChannel;

		Z_Free(buffer);
	}
	texw = textures[idx]->width;
	texh = textures[idx]->height;
	texmask = texmasked[idx];
	return texnames[idx];
}

void GL_SetTexture(int idx)
{
	gl.Bind(GL_PrepareTexture(idx));
}

int LineAverageRGB(byte *imgdata, int width, int height, int line, byte *rgb, byte *palette)
{
	byte	*start = imgdata + width*line;
	byte	*alphaStart = start + width*height;
	int		i, c, count = 0;
	int		integerRGB[3] = {0,0,0};
	byte	col[3];

	for(i=0; i<width; i++)
	{
		// Not transparent?
		if(alphaStart[i] > 0)
		{
			count++;
			// Ignore the gamma level.
			memcpy(col, palette + 3*start[i], 3);
			for(c=0; c<3; c++) integerRGB[c] += col[c];
		}
	}
	// All transparent? Sorry...
	if(!count) return 0;

	// We're going to make it!
	for(c=0; c<3; c++) rgb[c] = integerRGB[c]/count;
	return 1;	// Successful.
}

// The imgdata must have alpha info, too.
void ImageAverageRGB(byte *imgdata, int width, int height, byte *rgb, byte *palette)
{
	int	i, c, integerRGB[3] = {0,0,0}, count = 0;

	for(i=0; i<height; i++)
	{
		if(LineAverageRGB(imgdata, width, height, i, rgb, palette))
		{
			count++;
			for(c=0; c<3; c++) integerRGB[c] += rgb[c];
		}
	}
	if(count)	// If there were pixels...
		for(c=0; c<3; c++) 
			rgb[c] = integerRGB[c]/count;
}

// Return a skycol_t for texidx. 
// More housekeeping...
skycol_t *GL_GetSkyColor(int texidx)
{
	int			i;
	skycol_t	*skycol;

	if(texidx < 0) return NULL;

	// Try to find a skytop color for this.
	for(i=0; i<num_skytop_colors; i++)
		if(skytop_colors[i].texidx == texidx)
			return skytop_colors + i;

	// There was no skycol for the specified texidx!
	skytop_colors = (skycol_t *)realloc(skytop_colors, sizeof(skycol_t) * ++num_skytop_colors);
	// -JL- Paranoia
	if (!skytop_colors)
		I_Error("GL_GetSkyColor: realloc failed");
	skycol = skytop_colors + num_skytop_colors-1;
	memset(skycol, 0, sizeof(*skycol));
	skycol->texidx = texidx;
	return skycol;
}

// Fills the empty pixels with reasonable color indices.
// This gets rid of the black outlines.
// Not a very efficient algorithm...
static void ColorOutlines(byte *buffer, int width, int height)
{
	int		numpels = width*height;
	byte	*ptr;
	int		i, k, a, b;

	for(k=0; k<height; k++)
		for(i=0; i<width; i++)
			// Solid pixels spread around...
			if(buffer[numpels + i + k*width])
			{
				for(b=-1; b<=1; b++)
					for(a=-1; a<=1; a++)
					{
						// First check that the pixel is OK.
						if(!a && !b || i+a < 0 || k+b < 0 || i+a >= width || k+b >= height) 
							continue;

						ptr = buffer + i+a + (k+b)*width;
						if(!ptr[numpels]) // An alpha pixel?
							*ptr = buffer[i + k*width];
					}
			}
}

// Sky textures are ALWAYS 256 pixels wide!!
unsigned int GL_PrepareSky(int idx, boolean zeroMask)
{
	if(idx > numtextures-1) return 0;

	if(idx != texturetranslation[idx])
		I_Error("Skytex: %d, translated: %d\n", idx, texturetranslation[idx]);
	
	idx = texturetranslation[idx];
	
	if(!texnames[idx])
	{
		skycol_t	*skycol = GL_GetSkyColor(idx);
		int			i, k;
		byte		*palette = (byte *)W_CacheLumpNum(pallump, PU_CACHE);
		texture_t	*tex = textures[idx];
		int			height = tex->height;
		int			numpels;
		byte		*imgdata, *colptr;

		if(tex->patchcount > 1)
		{
			numpels = tex->width * tex->height;
			imgdata = (byte *)Z_Malloc(2 * numpels, PU_STATIC, 0);
			memset(imgdata, 0, 2 * numpels);
			for(i=0; i<tex->width; i++)
			{
				colptr = R_GetColumn(idx, i);
				for(k=0; k<tex->height; k++)
				{
					if(!zeroMask)
						imgdata[k*tex->width + i] = colptr[k];
					else if(colptr[k])
					{
						byte *imgpos = imgdata+(k*tex->width + i);
						*imgpos = colptr[k];
						*(imgpos+numpels) = 0xff;	// Not transparent, this pixel.
					}
				}
			}
		}
		else 
		{
			patch_t *patch = (patch_t *)W_CacheLumpNum(tex->patches[0].patch, PU_CACHE);
			int bufHeight = patch->height > tex->height? patch->height : tex->height;
			if(bufHeight > height)
			{
				// Heretic sky textures are reported to be 128 tall, even if the
				// data is 200. We'll adjust the real height of the texture up to
				// 200 pixels (remember Caldera?).
				height = bufHeight;
				if(height > 200) height = 200;
			}
			// Allocate a large enough buffer. At least Caldera has a too big sky (245 high).
			numpels = tex->width * bufHeight;
			imgdata = (byte *)Z_Malloc(2 * numpels, PU_STATIC, 0);
			memset(imgdata, 0, 2 * numpels);
			DrawRealPatch(imgdata, palette, tex->width, bufHeight, patch, zeroMask, 0);
		}
		if(zeroMask && filloutlines) ColorOutlines(imgdata, tex->width, height);

		// Calculate the topline RGB for sky top fadeouts.
		LineAverageRGB(imgdata, tex->width, height, 0, skycol->rgb, palette);

		texnames[idx] = UploadTexture(imgdata, tex->width, height, zeroMask, true, false);
		gl.TexParameter(DGL_MIN_FILTER, glmode[mipmapping]);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);

		// Free the buffer.
		Z_Free(imgdata);

		// Do we have a masked texture?
		if(zeroMask)
			texmasked[idx] = 1;
		else
			texmasked[idx] = 0;
	}
	texw = textures[idx]->width;
	texh = textures[idx]->height;
	texmask = texmasked[idx];
	return texnames[idx];
}

void GL_GetSkyTopColor(int texidx, byte *rgb)
{
	skycol_t *skycol = GL_GetSkyColor(texidx);

	if(!skycol) return;
	memcpy(rgb, skycol->rgb, 3);
}

transspr_t *GL_NewTranslatedSprite(int pnum, unsigned char *table)
{
	transspr_t *news;
	
	transsprites = (transspr_t *)realloc(transsprites, sizeof(transspr_t)* ++numtranssprites);
	// -JL- Paranoia
	if (!transsprites)
		I_Error("GL_NewTranslatedSprite: realloc failed");
	news = transsprites + numtranssprites-1;
	news->patch = pnum;
	news->tex = 0;
	news->table = table;
	return news;
}

transspr_t *GL_GetTranslatedSprite(int pnum, unsigned char *table)
{
	int		i;

	for(i=0; i<numtranssprites; i++)
		if(transsprites[i].patch == pnum && transsprites[i].table == table)
			return transsprites + i;
	return 0;
}

// Data must have alpha info.
void averageColorIdx(rgbcol_t *sprcol, byte *data, int w, int h, byte *palette)
{
	int				i;
	unsigned int	r, g, b, count;
	byte			*alphaStart = data + w*h, rgb[3];

	// First clear them.
	r = g = b = count = 0;
	for(i=0; i<w*h; i++)
	{
		if(alphaStart[i])
		{
			count++;
			memcpy(rgb, palette + 3*data[i], 3);
			r += rgb[0];
			g += rgb[1];
			b += rgb[2];
		}
	}
	sprcol->rgb[0] = r/count;
	sprcol->rgb[1] = g/count;
	sprcol->rgb[2] = b/count;
}

void averageColorRGBA(rgbcol_t *sprcol, byte *rgbaData, int w, int h)
{
	int				i;
	unsigned int	r, g, b, count;

	// First clear them.
	r = g = b = count = 0;
	for(i=0; i<w*h; i++)
	{
		byte *pixel = rgbaData + 4*i;
		if(pixel[3]) // Skip pixels with no visible color (alpha==0).
		{
			count++;
			r += pixel[0];
			g += pixel[1];
			b += pixel[2];
		}
	}
	sprcol->rgb[0] = r/count;
	sprcol->rgb[1] = g/count;
	sprcol->rgb[2] = b/count;
}

unsigned int GL_PrepareTranslatedSprite(int pnum, unsigned char *table)
{
	transspr_t	*tspr = GL_GetTranslatedSprite(pnum, table);

	if(!tspr)
	{
		// There's no name for this patch, load it in.
		patch_t		*patch = (patch_t *)W_CacheLumpNum(firstspritelump+pnum, PU_CACHE);
		int			bufsize = 2 * patch->width * patch->height;
		byte		*palette = (byte *)W_CacheLumpNum(pallump, PU_CACHE);
		byte		*buffer = (byte *)Z_Malloc(bufsize, PU_STATIC, 0);
		
		memset(buffer, 0, bufsize);
		DrawRealPatch(buffer, (byte *)W_CacheLumpNum(pallump, PU_CACHE), 
			patch->width, patch->height, patch, false, table);
		if(filloutlines) ColorOutlines(buffer, patch->width, patch->height);
		// The average color is used if the sprite emits light.
		averageColorIdx(spritecolors+pnum, buffer, patch->width, patch->height, palette);

		tspr = GL_NewTranslatedSprite(pnum, table);
		tspr->tex = UploadTexture(buffer, patch->width, patch->height, true, true, false);
		gl.TexParameter(DGL_MIN_FILTER, glmode[mipmapping]);
		gl.TexParameter(DGL_MAG_FILTER, filterSprites? DGL_LINEAR : DGL_NEAREST);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

		// Also here the height is updated.
		//spriteheight[pnum] = patch->height;

		Z_Free(buffer);
	}
	return tspr->tex;
}

unsigned int GL_PrepareSprite(int pnum)
{
	if(!spritenames[pnum])
	{
		// There's no name for this patch, load it in.
		byte	*palette = (byte *)W_CacheLumpNum(pallump, PU_CACHE);
		patch_t *patch = (patch_t *)W_CacheLumpNum(firstspritelump+pnum, PU_CACHE);
		int		bufsize = 2 * patch->width * patch->height;
		byte	*buffer = (byte *)Z_Malloc(bufsize, PU_STATIC, 0);

		memset(buffer, 0, bufsize);
		DrawRealPatch(buffer, (byte *)W_CacheLumpNum(pallump, PU_CACHE), 
			patch->width, patch->height, patch, false, 0);
		if(filloutlines) ColorOutlines(buffer, patch->width, patch->height);
		// The average color is used if the sprite emits light.
		averageColorIdx(spritecolors+pnum, buffer, patch->width, patch->height, palette);

		spritenames[pnum] = UploadTexture(buffer, patch->width, patch->height, true, true, false);
		gl.TexParameter(DGL_MIN_FILTER, glmode[mipmapping]);
		gl.TexParameter(DGL_MAG_FILTER, filterSprites? DGL_LINEAR : DGL_NEAREST);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

		//spriteheights[pnum] = patch->height;

		Z_Free(buffer);
	}
	return spritenames[pnum];
}

void GL_GetSpriteColor(int pnum, unsigned char *rgb)
{
	if(pnum > numspritelumps-1) return;
	memcpy(rgb, spritecolors+pnum, 3);
}
 
void GL_SetSprite(int pnum)
{
	GL_BindTexture(GL_PrepareSprite(pnum));
}

void GL_SetTranslatedSprite(int pnum, unsigned char *trans)
{
	GL_BindTexture(GL_PrepareTranslatedSprite(pnum, trans));
}

void GL_NewRawLump(int lump)
{
	rawlumps = (int *)realloc(rawlumps, sizeof(int) * ++numrawlumps);
	// -JL- Paranoia
	if (!rawlumps)
		I_Error("GL_NewRawLump: realloc failed");
	rawlumps[numrawlumps-1] = lump;
}

DGLuint GL_GetOtherPart(int lump)
{
	return lumptexnames2[lump];
}



// Part is either 1 or 2. Part 0 means only the left side is loaded.
// No splittex is created in that case. Once a raw image is loaded
// as part 0 it must be deleted before the other part is loaded at the
// next loading.
//
// Part can also contain the width and height of the texture.
void GL_SetRawImage(int lump, int part)
{
	if(part < 0 || part > 2 || lump > numlumps-1) return;	// Check the part.

	if(!lumptexnames[lump])
	{
		// Load the raw image data.
		// It's most efficient to create two textures for it (256 + 64 = 320).
		int		i, k, idx;
		byte	*raw = (byte *)W_CacheLumpNum(lump, PU_CACHE), *dat1, *dat2, *palette;
		int		height = lumpinfo[lump].size / 320;
		int		assumedWidth = 320;

		if(height < 200 && part == 2) return; // What is this?!
		if(height < 200) assumedWidth = 256;

		dat1 = (byte *)Z_Malloc(256 * 256, PU_STATIC, 0);	
		dat2 = (byte *)Z_Malloc(64 * 256, PU_STATIC, 0);
		memset(dat1, 0, 256 * 256);
		memset(dat2, 0, 64 * 256);
		palette = (byte *)W_CacheLumpNum(pallump, PU_CACHE);

		for(k=0; k<height; k++)
			for(i=0; i<256; i++)
			{
				idx = k*assumedWidth + i;
				// Part one.
				dat1[k*256 + i] = raw[idx];
				// We can setup part two at the same time.
				if(i<64 && part) dat2[k*64 + i] = raw[idx + 256];
			}

		// Upload part one.
		lumptexnames[lump] = UploadTexture(dat1, 256, assumedWidth<320? height : 256, false, false, false);
		gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
		gl.TexParameter(DGL_MAG_FILTER, linearRaw? DGL_LINEAR : DGL_NEAREST);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);
		
		if(part)
		{
			// And the other part.
			lumptexnames2[lump] = UploadTexture(dat2, 64, 256, false, false, false);
			gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
			gl.TexParameter(DGL_MAG_FILTER, linearRaw? DGL_LINEAR : DGL_NEAREST);
			gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
			gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

			// Add it to the list.
			GL_NewRawLump(lump);
		}

		lumptexsizes[lump].w = 256;
		lumptexsizes[lump].w2 = 64;
		lumptexsizes[lump].h = height;

		Z_Free(dat1);
		Z_Free(dat2);
	}
	// Bind the part that was asked for.
	if(part <= 1) gl.Bind(lumptexnames[lump]);
	if(part == 2) gl.Bind(lumptexnames2[lump]);
	// We don't track the current texture with raw images.
	curtex = 0;
}

struct vpic_t
{
	char magic[4];
	short width;
	short height;
	byte bpp;
	byte reserved[7];
};

void GL_SetVPic(int lump, void *data)
{
	if(lump > numlumps-1) return;
	if(!lumptexnames[lump])
	{
		vpic_t *pic = (vpic_t *)data;
		if (pic->bpp != 15)
			I_Error("Only 15 bit vpics supported");

		//	Unpack data
		int		numpels = pic->width * pic->height;
		byte	*buffer = (byte *)Z_Malloc(4 * numpels, PU_STATIC, 0);
		unsigned short *pData = (unsigned short *)(pic + 1);
		byte *pDst = buffer;
		for (int i = 0; i < numpels; i++, pData++, pDst += 4)
		{
			if (*pData & 0x8000)
			{
				pDst[0] = 0;
				pDst[1] = 0;
				pDst[2] = 0;
				pDst[3] = 0;
			}
			else
			{
				pDst[0] = (*pData >> 7) & 0xf8;
				pDst[1] = (*pData >> 2) & 0xf8;
				pDst[2] = (*pData << 3) & 0xf8;
				pDst[3] = 255;
			}
		}

		// Generate a texture.
		lumptexnames[lump] = UploadTexture(buffer, pic->width, pic->height, 
			true, false, true);
		gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

		//	Fill in info
		lumptexsizes[lump].w = pic->width;
		lumptexsizes[lump].h = pic->height;
		lumptexsizes[lump].w2 = 0;
		lumptexsizes[lump].offx = 0;
		lumptexsizes[lump].offy = 0;

		Z_Free(buffer);
	}
	else
	{
		GL_BindTexture(lumptexnames[lump]);
	}
	curtex = lumptexnames[lump];
}

// No mipmaps are generated for regular patches.
void GL_SetPatch(int lump)	
{
	if(lump > numlumps-1) return;
	if(!lumptexnames[lump])
	{
		// Load the patch.
		patch_t	*patch = (patch_t *)W_CacheLumpNum(lump, PU_CACHE);

		// -JL- HACK! HACK! HACK! Check for VPIC
		if (!memcmp(patch, "VPIC", 4))
		{
			GL_SetVPic(lump, patch);
			return;
		}

		int		numpels = patch->width * patch->height;
		byte	*buffer = (byte *)Z_Malloc(2 * numpels, PU_STATIC, 0);
		int		alphaChannel;
		
		memset(buffer, 0, 2*numpels);
		alphaChannel = DrawRealPatch(buffer, (byte *)W_CacheLumpNum(pallump, PU_CACHE),
			patch->width, patch->height, patch, false, 0);
		if(filloutlines) ColorOutlines(buffer, patch->width, patch->height);

		// See if we have to split the patch into two parts.
		// This is done to conserve the quality of wide textures
		// (like the status bar) on video cards that have a pitifully
		// small maximum texture size. ;-)
		if(patch->width > maxTexSize) 
		{
			// The width of the first part is maxTexSize.
			int part2width = patch->width - maxTexSize;
			byte *tempbuff = (byte *)Z_Malloc(2 * maxTexSize * patch->height, PU_STATIC, 0);
			
			// We'll use a temporary buffer for doing to splitting.
			// First, part one.
			pixBlt(buffer, patch->width, patch->height, tempbuff, maxTexSize, patch->height, 
				alphaChannel, 0, 0, 0, 0, maxTexSize, patch->height);
			lumptexnames[lump] = UploadTexture(tempbuff, maxTexSize, patch->height, 
				alphaChannel, false, false);

			gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
			gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
			gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
			gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

			// Then part two.
			pixBlt(buffer, patch->width, patch->height, tempbuff, part2width, patch->height,
				alphaChannel, maxTexSize, 0, 0, 0, part2width, patch->height);
			lumptexnames2[lump] = UploadTexture(tempbuff, part2width, patch->height, 
				alphaChannel, false, false);

			gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
			gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
			gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
			gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

			GL_BindTexture(lumptexnames[lump]);

			lumptexsizes[lump].w = maxTexSize;
			lumptexsizes[lump].w2 = patch->width - maxTexSize;

			Z_Free(tempbuff);
		}
		else // We can use the normal one-part method.
		{
			// Generate a texture.
			lumptexnames[lump] = UploadTexture(buffer, patch->width, patch->height, 
				alphaChannel, false, false);
			gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
			gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
			gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
			gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

			lumptexsizes[lump].w = patch->width;
			lumptexsizes[lump].w2 = 0;
		}
		// The rest of the size information.
		lumptexsizes[lump].h = patch->height;
		lumptexsizes[lump].offx = -patch->leftoffset;
		lumptexsizes[lump].offy = -patch->topoffset;

		Z_Free(buffer);
	}
	else
	{
		GL_BindTexture(lumptexnames[lump]);
	}
	curtex = lumptexnames[lump];
}

void GL_SetNoTexture()
{
	gl.Bind(0);
	curtex = 0;
}

// The dynamic light map is a 64x64 grayscale 8-bit image.
DGLuint GL_PrepareLightTexture()
{
	if(!dltexname)
	{
		// We need to generate the texture, I see.
		byte *image = (byte *)W_CacheLumpName("DLIGHT", PU_CACHE);
		if(!image) 
			I_Error("GL_SetLightTexture: no dlight texture.\n");
		dltexname = gl.NewTexture();
		// No mipmapping or resizing is needed, upload directly.
		gl.TexImage(DGL_LUMINANCE, 64, 64, 0, image);
		gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);
	}
	// Set the info.
	texw = texh = 64;
	texmask = 0;
	return dltexname;
}

DGLuint GL_PrepareFlareTexture(int flare)
{
	int		w, h;

	// There are three flare textures.
	if(flare < 0 || flare >= NUM_FLARES) return 0;

	// What size? Hardcoded dimensions... ack.
	w = h = flare==2? 128 : 64;

	if(!flaretexnames[flare])
	{
		byte *image = (byte *)W_CacheLumpName(flare==0? "FLARE" : flare==1? "BRFLARE" : "BIGFLARE", PU_CACHE);
		if(!image)
			I_Error("GL_PrepareFlareTexture: flare texture %i not found!\n", flare);

		flaretexnames[flare] = gl.NewTexture();
		gl.TexImage(DGL_LUMINANCE, w, h, 0, image);
		gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);
	}
	texmask = 0;
	texw = w;
	texh = h;
	return flaretexnames[flare];
}

int GL_GetLumpTexWidth(int lump)
{
	return lumptexsizes[lump].w;
}

int GL_GetLumpTexHeight(int lump)
{
	return lumptexsizes[lump].h;
}

// Updates the textures, flats and sprites (gameTex) or the user 
// interface textures (patches and raw screens).
void GL_SetTextureParams(int minMode, int magMode, int gameTex, int uiTex)
{
	int	i;
	
	if(gameTex)
	{
		// Textures.
		for(i=0; i<numtextures; i++)
			if(texnames[i])	// Is the texture loaded?
			{
				gl.Bind(texnames[i]);
				gl.TexParameter(DGL_MIN_FILTER, minMode);
				gl.TexParameter(DGL_MAG_FILTER, magMode);
			}
		// Flats.
		for(i=0; i<numflats; i++)
			if(flattexnames[i]) // Is the texture loaded?
			{
				gl.Bind(flattexnames[i]);
				gl.TexParameter(DGL_MIN_FILTER, minMode);
				gl.TexParameter(DGL_MAG_FILTER, magMode);
			}
		// Sprites.
		for(i=0; i<numspritelumps; i++)
			if(spritenames[i])
			{
				gl.Bind(spritenames[i]);
				gl.TexParameter(DGL_MIN_FILTER, minMode);
				gl.TexParameter(DGL_MAG_FILTER, magMode);
			}
		// Translated sprites.
		for(i=0; i<numtranssprites; i++)
		{	
			gl.Bind(transsprites[i].tex);
			gl.TexParameter(DGL_MIN_FILTER, minMode);
			gl.TexParameter(DGL_MAG_FILTER, magMode);
		}
	}
	if(uiTex)
	{
		for(i=0; i<numlumps; i++)
		{
			if(lumptexnames[i])
			{
				gl.Bind(lumptexnames[i]);
				gl.TexParameter(DGL_MIN_FILTER, minMode);
				gl.TexParameter(DGL_MAG_FILTER, magMode);
			}
			if(lumptexnames2[i])
			{
				gl.Bind(lumptexnames[i]);
				gl.TexParameter(DGL_MIN_FILTER, minMode);
				gl.TexParameter(DGL_MAG_FILTER, magMode);
			}
		}
	}
}

void GL_UpdateTexParams(int mipmode)
{
	mipmapping = mipmode;
	GL_SetTextureParams(glmode[mipmode], DGL_LINEAR, true, false);
}

void GL_LowRes()
{
	// Set everything as low as they go.
	GL_SetTextureParams(DGL_NEAREST, DGL_NEAREST, true, true);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
}

// Updates the raw screen smoothing (linear magnification).
// This is the main reason for having the rawlumps table.
// Not much of a reason, eh?
void GL_UpdateRawScreenParams(int smoothing)
{
	int		i;
	int		glmode = (smoothing)? DGL_LINEAR : DGL_NEAREST;

	linearRaw = smoothing;
	for(i=0; i<numrawlumps; i++)
	{
		// First part 1.
		gl.Bind(lumptexnames[rawlumps[i]]);
		gl.TexParameter(DGL_MAG_FILTER, glmode);
		// Then part 2.
		gl.Bind(lumptexnames2[rawlumps[i]]);
		gl.TexParameter(DGL_MAG_FILTER, glmode);
	}
}

void GL_TextureFilterMode(int target, int parm)
{
	if(target == DD_TEXTURES) GL_UpdateTexParams(parm);
	if(target == DD_RAWSCREENS) GL_UpdateRawScreenParams(parm);
}

// Deletes a texture. Only for textures (not for sprites, flats, etc.).
void GL_DeleteTexture(int texidx)
{
	if(texidx < 0) return;

	if(texnames[texidx])
	{
		gl.DeleteTextures(1, texnames+texidx);
		texnames[texidx] = 0;
	}
}

unsigned int GL_GetTextureName(int texidx)
{
	return texnames[texidx];
}

unsigned int GL_PrepareSkin(model_t *mdl, int skin)
{
	if(skin < 0 || skin > mdl->header.numSkins) skin = 0;
	// If the texture has already been loaded, we don't need to
	// do anything.
	if(!mdl->skintex[skin])
	{
		int		size;
		byte	*image;

		// Allocate the image buffer (RGB).
		size = mdl->header.skinWidth * mdl->header.skinHeight * 3;		
		image = (byte *)Z_Malloc(size, PU_STATIC, 0);

		// Load the texture.
		R_LoadSkinRGB(mdl, skin, image);

		mdl->skintex[skin] = UploadTexture(image, 
			mdl->header.skinWidth, mdl->header.skinHeight,
			false, true, true);

		gl.TexParameter(DGL_MIN_FILTER, glmode[mipmapping]);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);

		// We don't need the image any more.
		Z_Free(image);
	}
	return mdl->skintex[skin];
}


//--------------------------------------------------------------------------

#ifdef _DEBUG
int CCmdTranslateFont(int argc, char **argv)
{
	char	name[32];
	int		i, lump, size;
	patch_t	*patch;
	byte	redToWhite[256];

	if(argc < 3) return false;

	// Prepare the red-to-white table.
	for(i=0; i<256; i++)
	{
		if(i == 176) redToWhite[i] = 168; // Full red -> white.
		else if(i == 45) redToWhite[i] = 106;
		else if(i == 46) redToWhite[i] = 107;
		else if(i == 47) redToWhite[i] = 108; 
		else if (i >= 177 && i <= 191)
		{
			redToWhite[i] = 80 + (i-177)*2;
		}
		else redToWhite[i] = i; // No translation for this.
	}

	// Translate everything.
	for(i=0; i<256; i++)
	{
		sprintf(name, "%s%.3d", argv[1], i);
		if((lump = W_CheckNumForName(name)) != -1)
		{
			ST_Message( "%s...\n", name);
			size = W_LumpLength(lump);
			patch = (patch_t*) Z_Malloc(size, PU_STATIC, 0);
			memcpy(patch, W_CacheLumpNum(lump, PU_CACHE), size);
			TranslatePatch(patch, redToWhite);
			sprintf(name, "%s%.3d.lmp", argv[2], i);
			M_WriteFile(name, patch, size);
			Z_Free(patch);
		}
	}
	return true;
}
#endif