
//**************************************************************************
//**
//** r_data.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include "h2def.h"
#include "i_win32.h"
#include "gl_def.h" // For precaching.

extern thinker_t thinkercap;


int		firstflat, lastflat, numflats;
int		firstpatch, lastpatch, numpatches;
int		firstspritelump, lastspritelump, numspritelumps;

int			numtextures;
texture_t	**textures;
int			*texturewidthmask;
fixed_t		*textureheight;		// needed for texture pegging
int			*texturecompositesize;
short		**texturecolumnlump;
unsigned short		**texturecolumnofs;
byte		**texturecomposite;

int			*flatflags;  //crap

int			*flattranslation;		// for global animation
int			*texturetranslation;	// for global animation

fixed_t		*spritewidth;		// needed for pre rendering
int			*spriteheight;		
//fixed_t		*spriteheight;		// For OpenGL rendering. -jk
fixed_t		*spriteoffset;
fixed_t		*spritetopoffset;

// Glowing textures are always rendered fullbright.
int			r_texglow = true;

//lighttable_t	*colormaps;



int R_SetFlatTranslation(int flat, int translateTo)
{
	int old = flattranslation[flat];
	flattranslation[flat] = translateTo;
	return old;
}

int R_SetTextureTranslation(int tex, int translateTo)
{
	int old = texturetranslation[tex];
	texturetranslation[tex] = translateTo;
	return old;
}



/*
==============================================================================

						MAPTEXTURE_T CACHING

when a texture is first needed, it counts the number of composite columns
required in the texture and allocates space for a column directory and any
new columns.  The directory will simply point inside other patches if there
is only one patch in a given column, but any columns with multiple patches
will have new column_ts generated.

==============================================================================
*/

/*
===================
=
= R_DrawColumnInCache
=
= Clip and draw a column from a patch into a cached post
=
===================
*/

void R_DrawColumnInCache (column_t *patch, byte *cache, int originy, int cacheheight)
{
	int		count, position;
	byte	*source, *dest;
	
	dest = (byte *)cache + 3;
	
	while (patch->topdelta != 0xff)
	{
		source = (byte *)patch + 3;
		count = patch->length;
		position = originy + patch->topdelta;
		if (position < 0)
		{
			count += position;
			position = 0;
		}
		if (position + count > cacheheight)
			count = cacheheight - position;
		if (count > 0)
			memcpy (cache + position, source, count);
		
		patch = (column_t *)(  (byte *)patch + patch->length + 4);
	}
}


/*
===================
=
= R_GenerateComposite
=
===================
*/

void R_GenerateComposite (int texnum)
{
	byte		*block;
	texture_t	*texture;
	texpatch_t	*patch;	
	patch_t		*realpatch;
	int			x, x1, x2;
	int			i;
	column_t	*patchcol;
	short		*collump;
	unsigned short *colofs;
	
	texture = textures[texnum];
	block = (byte *)Z_Malloc (texturecompositesize[texnum], PU_REFRESHTEX, 
		&texturecomposite[texnum]);	
	collump = texturecolumnlump[texnum];
	colofs = texturecolumnofs[texnum];
		
//
// composite the columns together
//
	patch = texture->patches;
		
	for (i=0 , patch = texture->patches; i<texture->patchcount ; i++, patch++)
	{
		realpatch = (patch_t *)W_CacheLumpNum (patch->patch, PU_CACHE);
		x1 = patch->originx;
		x2 = x1 + SHORT(realpatch->width);

		if (x1<0)
			x = 0;
		else
			x = x1;
		if (x2 > texture->width)
			x2 = texture->width;

		for ( ; x<x2 ; x++)
		{
			if (collump[x] >= 0)
				continue;		// column does not have multiple patches
			patchcol = (column_t *)((byte *)realpatch + 
				LONG(realpatch->columnofs[x-x1]));
			R_DrawColumnInCache (patchcol, block + colofs[x], patch->originy,
			texture->height);
		}
						
	}

// now that the texture has been built, it is purgable
	Z_ChangeTag (block, PU_CACHE);
}


/*
===================
=
= R_GenerateLookup
=
===================
*/

void R_GenerateLookup (int texnum)
{
	texture_t	*texture;
	byte		*patchcount;		// [texture->width]
	texpatch_t	*patch;	
	patch_t		*realpatch;
	int			x, x1, x2;
	int			i;
	short		*collump;
	unsigned short	*colofs;
	
	texture = textures[texnum];

	texturecomposite[texnum] = 0;	// composited not created yet
	texturecompositesize[texnum] = 0;
	collump = texturecolumnlump[texnum];
	colofs = texturecolumnofs[texnum];
	
//
// count the number of columns that are covered by more than one patch
// fill in the lump / offset, so columns with only a single patch are
// all done
//
	patchcount = (byte *) Z_Malloc(texture->width, PU_STATIC, 0);
	memset (patchcount, 0, texture->width);
	patch = texture->patches;
		
	for (i=0 , patch = texture->patches; i<texture->patchcount ; i++, patch++)
	{
		realpatch = (patch_t *)W_CacheLumpNum (patch->patch, PU_CACHE);
		x1 = patch->originx;
		x2 = x1 + SHORT(realpatch->width);
		if (x1 < 0)
			x = 0;
		else
			x = x1;
		if (x2 > texture->width)
			x2 = texture->width;
		for ( ; x<x2 ; x++)
		{
			patchcount[x]++;
			collump[x] = patch->patch;
			colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
		}
	}
	
	for (x=0 ; x<texture->width ; x++)
	{
		if (!patchcount[x])
		{
			ST_Message ("R_GenerateLookup: column without a patch (%s)\n", texture->name);
			free(patchcount);
			return;
		}
//			I_Error ("R_GenerateLookup: column without a patch");
		if (patchcount[x] > 1)
		{
			collump[x] = -1;	// use the cached block
			colofs[x] = texturecompositesize[texnum];
			if (texturecompositesize[texnum] > 0x10000-texture->height)
				I_Error ("R_GenerateLookup: texture %i is >64k",texnum);
			texturecompositesize[texnum] += texture->height;
		}
	}	
	Z_Free(patchcount);
}


/*
================
=
= R_GetColumn
=
================
*/

byte *R_GetColumn (int tex, int col)
{
	int	lump, ofs;
	
	col &= texturewidthmask[tex];
	lump = texturecolumnlump[tex][col];
	ofs = texturecolumnofs[tex][col];
	if (lump > 0)
		return (byte *)W_CacheLumpNum(lump,PU_CACHE)+ofs;
	if (!texturecomposite[tex])
		R_GenerateComposite (tex);
	return texturecomposite[tex] + ofs;
}


/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

void R_InitTextures (void)
{
	maptexture_t	*mtexture;
	texture_t		*texture;
	mappatch_t	*mpatch;
	texpatch_t	*patch;
	int			i,j;
	int			*maptex, *maptex2, *maptex1;
	char		name[9], *names, *name_p;
	int			*patchlookup;
	int			totalwidth;
	int			nummappatches;
	int			offset, maxoff, maxoff2;
	int			numtextures1, numtextures2;
	int			*directory;

//
// load the patch names from pnames.lmp
//
	name[8] = 0;
	names = (char *)W_CacheLumpName ("PNAMES", PU_REFRESHTEX);
	nummappatches = LONG ( *((int *)names) );
	name_p = names+4;
	patchlookup = (int *)Z_Malloc(nummappatches * sizeof(*patchlookup), PU_STATIC, 0);
	for (i=0 ; i<nummappatches ; i++)
	{
		strncpy (name,name_p+i*8, 8);
		patchlookup[i] = W_CheckNumForName (name);
	}
	Z_Free (names);

//
// load the map texture definitions from textures.lmp
//
	maptex = maptex1 = (int *)W_CacheLumpName ("TEXTURE1", PU_REFRESHTEX);
	numtextures1 = LONG(*maptex);
	maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
	directory = maptex+1;

	if (W_CheckNumForName ("TEXTURE2") != -1)
	{
		maptex2 = (int *)W_CacheLumpName ("TEXTURE2", PU_REFRESHTEX);
		numtextures2 = LONG(*maptex2);
		maxoff2 = W_LumpLength (W_GetNumForName ("TEXTURE2"));
	}
	else
	{
		maptex2 = NULL;
		numtextures2 = 0;
		maxoff2 = 0;
	}
	numtextures = numtextures1 + numtextures2;

	textures = (texture_t **)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);
	texturecolumnlump = (short **)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);
	texturecolumnofs = (unsigned short **)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);
	texturecomposite = (byte **)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);
	texturecompositesize = (int *)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);
	texturewidthmask = (int *)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);
	textureheight = (int *)Z_Malloc (numtextures*4, PU_REFRESHTEX, 0);

	totalwidth = 0;

	for (i=0 ; i<numtextures ; i++, directory++)
	{
		if (i == numtextures1)
		{	// start looking in second texture file
			maptex = maptex2;
			maxoff = maxoff2;
			directory = maptex+1;
		}

		offset = LONG(*directory);
		if (offset > maxoff)
			I_Error ("R_InitTextures: bad texture directory");
		mtexture = (maptexture_t *) ( (byte *)maptex + offset);
		texture = textures[i] = (texture_t *)Z_Malloc (sizeof(texture_t) 
			+ sizeof(texpatch_t)*(SHORT(mtexture->patchcount)-1), PU_REFRESHTEX,
			0);
		texture->width = SHORT(mtexture->width);
		texture->height = SHORT(mtexture->height);
		texture->flags = mtexture->masked? TXF_MASKED : 0;

		texture->patchcount = SHORT(mtexture->patchcount);
		memcpy (texture->name, mtexture->name, sizeof(texture->name));
		mpatch = &mtexture->patches[0];
		patch = &texture->patches[0];
		for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
		{
			patch->originx = SHORT(mpatch->originx);
			patch->originy = SHORT(mpatch->originy);
			patch->patch = patchlookup[SHORT(mpatch->patch)];
			if (patch->patch == -1)
				I_Error (
				"R_InitTextures: Missing patch in texture %s",texture->name);
		}		
		texturecolumnlump[i] = (short *)Z_Malloc (texture->width*2, PU_REFRESHTEX,0);
		texturecolumnofs[i] = (unsigned short *)Z_Malloc (texture->width*2, PU_REFRESHTEX,0);
		j = 1;
		while (j*2 <= texture->width)
			j<<=1;
		texturewidthmask[i] = j-1;
		textureheight[i] = texture->height<<FRACBITS;
		
		totalwidth += texture->width;
	}

	Z_Free (maptex1);
	if (maptex2)
		Z_Free (maptex2);

//
// precalculate whatever possible
//		
	for (i=0 ; i<numtextures ; i++)
	{
		R_GenerateLookup (i);
		if(!(i&31)) ST_Progress();
	}

//
// translation table for global animation
//
	texturetranslation = (int *)Z_Malloc ((numtextures+1)*4, PU_REFRESHTEX, 0);
	for (i=0 ; i<numtextures ; i++)
		texturetranslation[i] = i;

	Z_Free(patchlookup);
}

void R_UpdateTextures (void)
{
	Z_FreeTags(PU_REFRESHTEX, PU_REFRESHTEX);
	R_InitTextures();		
}

int R_TextureFlags(int texture)
{
	if(!r_texglow) return 0;
	texture = texturetranslation[texture];
	if(!texture) return 0;
	return textures[texture]->flags;
}

int R_FlatFlags(int flat)
{
	if(!r_texglow) return 0;
	return flatflags[flattranslation[flat]];
}

/*
================
=
= R_InitFlats
=
=================
*/

void R_InitFlats (void)
{
	int		i;
	
	firstflat = W_GetNumForName ("F_START") + 1;
	lastflat = W_GetNumForName ("F_END") - 1;
	numflats = lastflat - firstflat + 1;

// translation table for global animation
	flattranslation = (int *)Z_Malloc ((numflats+1)*4, PU_STATIC, 0);
	for (i=0 ; i<numflats ; i++)
		flattranslation[i] = i;

	// Flatflags uses TXF_*.
	flatflags = (int *)Z_Malloc((numflats+1)*4, PU_STATIC, 0);
	memset(flatflags, 0, (numflats+1)*4);
}


void R_UpdateFlats (void)
{
	int curfirstflat = W_GetNumForName ("F_START") + 1,
		curlastflat = W_GetNumForName ("F_END") - 1;

	if(curfirstflat != firstflat || curlastflat != lastflat)
	{
		Z_Free(flattranslation);
		Z_Free(flatflags);
		R_InitFlats();
	}
}

/*
================
=
= R_InitSpriteLumps
=
= Finds the width and hoffset of all sprites in the wad, so the sprite doesn't
= need to be cached just for the header during rendering
=================
*/

void R_InitSpriteLumps (void)
{
	int		i;
	patch_t	*patch;

	firstspritelump = W_GetNumForName ("S_START") + 1;
	lastspritelump = W_GetNumForName ("S_END") - 1;
	numspritelumps = lastspritelump - firstspritelump + 1;
	spriteheight = (int *)Z_Malloc (numspritelumps*4, PU_REFRESHSPR, 0);
	spritewidth = (int *)Z_Malloc (numspritelumps*4, PU_REFRESHSPR, 0);
	spriteoffset = (int *)Z_Malloc (numspritelumps*4, PU_REFRESHSPR, 0);
	spritetopoffset = (int *)Z_Malloc (numspritelumps*4, PU_REFRESHSPR, 0);

	for (i=0 ; i< numspritelumps ; i++)
	{
		if (!(i&127)) ST_Progress();
		patch = (patch_t *)W_CacheLumpNum (firstspritelump+i, PU_CACHE);
		spriteheight[i] = SHORT(patch->height);
		spritewidth[i] = SHORT(patch->width)<<FRACBITS;
		spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
		spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
	}
	ST_Message( "\n");
}

void R_UpdateSpriteLumps (void)
{
	int	curfirst = W_GetNumForName ("S_START") + 1, 
		curlast = W_GetNumForName ("S_END") - 1;

	if(curfirst != firstspritelump || curlast != lastspritelump)
	{
		Z_FreeTags(PU_REFRESHSPR, PU_REFRESHSPR);
		R_InitSpriteLumps();
	}
}


/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/

void R_InitData (void)
{
	R_InitTextures();
	R_InitFlats();
	R_InitSpriteLumps();
}

void R_UpdateData (void)
{
	R_UpdateTextures();
	R_UpdateFlats();
	R_UpdateSpriteLumps();
}

//=============================================================================

/*
================
=
= R_FlatNumForName
=
================
*/

int	R_FlatNumForName (char *name)
{
	int		i;
	char	namet[9];

	i = W_CheckNumForName (name);
	if (i == -1)
	{
		namet[8] = 0;
		memcpy (namet, name,8);
		I_Error ("R_FlatNumForName: %s not found",namet);
	}
	return i - firstflat;
}


/*
================
=
= R_CheckTextureNumForName
=
================
*/

int	R_CheckTextureNumForName (char *name)
{
	int		i;
	
	if (name[0] == '-')		// no texture marker
		return 0;
		
	for (i=0 ; i<numtextures ; i++)
		if (!strncasecmp (textures[i]->name, name, 8) )
			return i;
		
	return -1;
}


/*
================
=
= R_TextureNumForName
=
================
*/

int	R_TextureNumForName (char *name)
{
	int		i;
	//char	namet[9];
	
	i = R_CheckTextureNumForName (name);
	if (i==-1)
		I_Error ("R_TextureNumForName: %s not found",name);
	
	return i;
}


char* R_TextureNameForNum(int num)
{
	if(num < 0 || num > numtextures-1) return NULL;
	return textures[num]->name;
}

/*
=================
=
= R_PrecacheLevel
=
= Preloads all relevent graphics for the level
=================
*/

int		flatmemory, texturememory, spritememory;

void R_PrecacheLevel (void)
{
	char			*flatpresent;
	char			*texturepresent;
	char			*spritepresent;
	int				i,j,k, lump;
	texture_t		*texture;
	thinker_t		*th;
	spriteframe_t	*sf;

/*	if (demoplayback)
		return;*/
			
//
// precache flats
//	
	flatpresent = (char *)Z_Malloc(numflats, PU_STATIC, 0);
	memset (flatpresent,0,numflats);	
	for (i=0 ; i<numsectors ; i++)
	{
		flatpresent[sectors[i].floorpic] = 1;
		flatpresent[sectors[i].ceilingpic] = 1;
	}
	
	flatmemory = 0;
	for (i=0 ; i<numflats ; i++)
		if (flatpresent[i])
		{
			lump = firstflat + i;
			flatmemory += lumpinfo[lump].size;
			W_CacheLumpNum(lump, PU_CACHE);
		}
	//printf( "%d bytes of precached flats.\n",flatmemory);
		
//
// precache textures
//
	texturepresent = (char *)Z_Malloc(numtextures, PU_STATIC, 0);
	memset (texturepresent,0, numtextures);
	
	for (i=0 ; i<numsides ; i++)
	{
		texturepresent[sides[i].toptexture] = 1;
		texturepresent[sides[i].midtexture] = 1;
		texturepresent[sides[i].bottomtexture] = 1;
	}
	
	// FIXME: Precache sky textures!
	//texturepresent[Sky1Texture] = 1;
	//texturepresent[Sky2Texture] = 1;

	texturememory = 0;
	for (i=0 ; i<numtextures ; i++)
	{
		if (!texturepresent[i])
			continue;
		texture = textures[i];
		for (j=0 ; j<texture->patchcount ; j++)
		{
			lump = texture->patches[j].patch;
			texturememory += lumpinfo[lump].size;
			W_CacheLumpNum(lump , PU_CACHE);
		}
	}
	//printf( "%d bytes of textures precached.\n",texturememory);
	
//
// precache sprites
//
	spritepresent = (char *)Z_Malloc(numsprites, PU_STATIC, 0);
	memset (spritepresent,0, numsprites);
	
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function == (think_t)P_MobjThinker)
			spritepresent[((mobj_t *)th)->sprite] = 1;
	}
	
	spritememory = 0;
	for (i=0 ; i<numsprites ; i++)
	{
		if (!spritepresent[i])
			continue;
		for (j=0 ; j<sprites[i].numframes ; j++)
		{
			sf = &sprites[i].spriteframes[j];
			for (k=0 ; k<8 ; k++)
			{
				lump = firstspritelump + sf->lump[k];
				spritememory += lumpinfo[lump].size;
				W_CacheLumpNum(lump , PU_CACHE);
			}
		}
	}
	/*ST_Message( "Memory used for flats, textures and sprites: %d bytes\n",
		flatmemory + texturememory + spritememory);*/
	Z_Free(flatpresent);
	Z_Free(texturepresent);
	Z_Free(spritepresent);
}
