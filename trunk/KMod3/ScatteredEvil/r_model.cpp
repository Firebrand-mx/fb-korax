
//**************************************************************************
//**
//** R_MODEL.C
//**
//** Sprite/frame replacement manager.
//** MD2 loading/handling.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <ctype.h>
#include "h2def.h"
#include "dd_md2.h"
#include "i_win32.h"
#include "console.h"
#include "gl_def.h"
//#include "tga.h"

// MACROS ------------------------------------------------------------------

#define	DEFAULT_MDEF_NAME	"ModelDef.txt"

// TYPES -------------------------------------------------------------------

#pragma pack(1)
typedef struct
{
    char			manufacturer;
    char			version;
    char			encoding;
    char			bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char			reserved;
    char			color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char			filler[58];
    unsigned char	data;			// unbounded
} pcx_t;
#pragma pack()

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

model_t		dummy = { true, "Dummy! Dummy!" };

model_t		*modellist[MAX_MODELS] = { &dummy };
boolean		useModels = true;	

modeldef_t	*models;
int			nummodels;

float avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

char *copyword(char *dest, char *ptr)
{
	boolean quoted = false;

	while(isspace(*ptr) && *ptr) ptr++;
	if(*ptr == '"') 
	{
		quoted = true;
		ptr++;
	}
	while(((!quoted && !isspace(*ptr)) || (quoted && *ptr != '"')) && *ptr) 
		*dest++ = *ptr++;
	*dest = 0;	// End the word.
	return ptr;
}

// Returns the index.
int R_GetModelFor(char *filename)
{
	int		i;

	for(i=0; i<MAX_MODELS; i++)
		if(modellist[i] && !stricmp(modellist[i]->fileName, filename))
			return i;
	// Then we must initialize a new one.
	for(i=0; i<MAX_MODELS; i++)
		if(!modellist[i])
		{
			modellist[i] = (model_t*) Z_Malloc(sizeof(model_t), PU_STATIC, 0);
			memset(modellist[i], 0, sizeof(model_t));
			return i;
		}
	// Out of models.
	return -1;
}

static void *AllocAndLoad(FILE *file, int offset, int len)
{
	void *ptr = Z_Malloc(len, PU_STATIC, 0);

	fseek(file, offset, SEEK_SET);
	fread(ptr, len, 1, file);
	return ptr;
}

// Finds the existing model or loads in a new one.
int R_LoadModel(char *filename)
{
	int		i, k, c, index;
	model_t	*mdl;
	FILE	*file;
	byte	*frames;
	
	// First try to open the file. It might not even exist.
	if((file = fopen(filename, "rb")) == NULL)
	{
		ST_Message("R_LoadModel: can't find '%s'.\n", filename);
		return 0;
	}
	// Allocate a new model_t.
	index = R_GetModelFor(filename);
	if(index < 0) 
	{
		fclose(file);
		return 0;	// Bugger.
	}
	mdl = modellist[index];
	if(mdl->loaded) 
	{
		fclose(file);
		return index;	// Already loaded.
	}
	// Now we can load in the data.
	fread(&mdl->header, sizeof(mdl->header), 1, file);
	if(mdl->header.magic != MD2_MAGIC)
	{
		// Bad magic! Cancel the loading.
		Z_Free(mdl);
		modellist[index] = 0;
		fclose(file);
		return 0;
	}
	
	// Allocate and load the various arrays.
	mdl->skins = (md2_skin_t*) AllocAndLoad(file, mdl->header.offsetSkins, 
		sizeof(md2_skin_t) * mdl->header.numSkins);

	mdl->texCoords = (md2_textureCoordinate_t*) AllocAndLoad(file, 
		mdl->header.offsetTexCoords, 
		sizeof(md2_textureCoordinate_t) * mdl->header.numTexCoords);

	mdl->triangles = (md2_triangle_t*) AllocAndLoad(file, 
		mdl->header.offsetTriangles,
		sizeof(md2_triangle_t) * mdl->header.numTriangles);

	// The frames need to be unpacked.
	frames = (byte*) AllocAndLoad(file, 
		mdl->header.offsetFrames,
		mdl->header.frameSize * mdl->header.numFrames);
	mdl->frames = (md2_frame_t *)Z_Malloc(sizeof(md2_frame_t) * mdl->header.numFrames, PU_STATIC, 0);
	for(i=0; i<mdl->header.numFrames; i++)
	{
		md2_packedFrame_t *pfr = (md2_packedFrame_t*) 
			(frames + mdl->header.frameSize * i);
		md2_triangleVertex_t *pVtx;
		md2_modelVertex_t *vtx;
		memcpy(mdl->frames[i].name, pfr->name, sizeof(pfr->name));
		mdl->frames[i].vertices = (md2_modelVertex_t *)Z_Malloc(sizeof(md2_modelVertex_t) * 
			mdl->header.numVertices, PU_STATIC, 0);
		// Translate each vertex.
		for(k=0, vtx=mdl->frames[i].vertices, pVtx=pfr->vertices;
			k<mdl->header.numVertices; k++, vtx++, pVtx++)
		{
			vtx->lightNormalIndex = pVtx->lightNormalIndex;
			for(c=0; c<3; c++)
			{
				vtx->vertex[c] = pVtx->vertex[c] * pfr->scale[c] + 
					pfr->translate[c];
			}
		}
	}
	Z_Free(frames);

	mdl->glCommands = (int*) AllocAndLoad(file,
		mdl->header.offsetGlCommands,
		sizeof(int) * mdl->header.numGlCommands);

	ST_Message( "Model loaded: %s\n", filename);
	/*ST_Message( "  skins:%i frames:%i tris:%i tcs:%i glCs:%i\n",
		mdl->header.numSkins, mdl->header.numFrames, mdl->header.numTriangles,
		mdl->header.numTexCoords, mdl->header.numGlCommands);*/
/*	{
		int i;
		for(i=0; i<mdl->header.numFrames; i++)
			ST_Message( "%i: %s\n", i, ((md2_frame_t*) (mdl->frames+i*mdl->header.frameSize))->name);
	}*/

	// Allocate memory for the skin texture names.
	mdl->skintex = (unsigned int*) Z_Malloc(sizeof(unsigned int) 
		* mdl->header.numSkins,	PU_STATIC, 0);
	memset(mdl->skintex, 0, sizeof(unsigned int) * mdl->header.numSkins);

	// We're done.
	mdl->loaded = true;
	strcpy(mdl->fileName, filename);
	fclose(file);
	return index;
}

int R_ModelFrameNumForName(int modelnum, char *fname)
{
	int			i;
	model_t		*mdl;

	if(!modelnum) return 0;
	mdl = modellist[modelnum];
	for(i=0; i<mdl->header.numFrames; i++)
	{
		if(!stricmp(mdl->frames[i].name, fname)) return i;
	}
	return 0;
}

// Is there a model for this sprite/frame combination?
modelframe_t *R_CheckModelFor(int sprite, int frame)
{
	int mdIdx = sprites[sprite].modeldef;
	modeldef_t *modef;

	//if(!sprites[sprite].modelframes) return NULL;
	//if(!sprites[sprite].modelframes[frame].sub[0].model) return NULL;

	if(mdIdx < 0) return NULL;
	modef = models + mdIdx;
	if(frame >= modef->numframes) return NULL;
	
	//return &sprites[sprite].modelframes[frame];
	
	if(!modef->modelframes[frame].sub[0].model) return NULL;
	return modef->modelframes + frame;
}

md2_frame_t *R_GetMD2Frame(int model, int frame)
{
	return modellist[model]->frames + frame;
}

float R_GetModelHRange(int model, int frame, float *top, float *bottom)
{
	md2_frame_t	*mframe = R_GetMD2Frame(model, frame);
	float		z;
	int			i;

	if(!mframe) I_Error("R_GetModelHRange: bad model/frame.\n");

	// Find the top and bottom heights.
	for(i=0; i<modellist[model]->header.numVertices; i++)
	{
		z = mframe->vertices[i].vertex[VZ];
		if(!i) 
			*top = *bottom = z;
		else if(z > *top)
			*top = z;
		else if(z < *bottom)
			*bottom = z;
	}
	return *top - *bottom;
}

void R_SetModelScales(modelframe_t *mf)
{
	int	i, k;
	modeldef_t *modef;

	for(i=0; i<numsprites; i++)
	{
		if(sprites[i].modeldef < 0) continue;
		modef = models + sprites[i].modeldef;
		for(k=0; k<modef->numframes; k++)
			if(modef->modelframes[k].sub[0].model == mf->sub[0].model)
			{
				modef->modelframes[k].autoscale = mf->autoscale;
			}
	}
}

// If isFactor is true, value is a factor with which to scale.
// Otherwise value is the height to scale to.
void R_ScaleModel(modelframe_t *mf, /*int sprite, int frame, */float destHeight,
				  float offset)
{
//	spritedef_t		*spr = sprites + sprite;
	//modeldef_t		*modef = models + spr->modeldef;
	//modelframe_t	*mf = &modef->modelframes[frame];
	submodelframe_t	*smf = &mf->sub[0];
	md2_frame_t		*mFrame = R_GetMD2Frame(smf->model, smf->frame);
//	int				i, k;//, lump = spr->spriteframes[frame].lump[0];
	int				num = modellist[smf->model]->header.numVertices;
	//float			sprHeight = spriteheight[lump];
	//float			sprOffset = spritetopoffset[lump] >> FRACBITS;
	float			top, bottom, height;
	float			scale;

	// Find the top and bottom heights.
	height = R_GetModelHRange(smf->model, smf->frame, &top, &bottom);
	if(!height) height = 1;
	/*ST_Message( "mdl:%i fr:%i sprh:%f height:%f (%s)\n",
		mf->model[0], mf->frame[0], sprHeight, height,
		mf->model[1]? "has" : "-");*/
/*	if(isFactor)
		scale = value;
	else*/
	
	scale = destHeight /*sprHeight*/ / height;

	if(!mf->autoscale) 
	{
		mf->autoscale = scale;
		R_SetModelScales(mf);
		//ST_Message( "* set scale=%f\n", mf->scale);
	}
	// Scale it appropriately.
	//for(i=0; i<3; i++) mFrame->scale[i] *= scale;
	mf->scale = scale;
	//mf->offset = offset - (mFrame->translate[VZ] + height)*scale;
	mf->offset = -bottom*scale /*-mFrame->translate[VZ]*scale + */ + offset;// + height*scale;

	/*mFrame->translate[VX] *= scale;
	mFrame->translate[VY] *= scale;*/
	//mFrame->translate[VZ] = -height*scale + offset;// + 4;
	//if(mFrame->translate[VZ] < 0) mFrame->translate[VZ] = 0;

	// Scale the sub-models appropriately.
	/*for(i=1; i<MAX_FRAME_MODELS; i++)
	{
		float sTop, sBottom;
		md2_frame_t *sFrame;
		if(!mf->sub[i].model) continue;
		sFrame = R_GetMD2Frame(mf->sub[i].model, mf->sub[i].frame);
		// The scale is the same.
		R_GetModelHRange(mf->sub[i].model, mf->sub[i].frame, &sTop, &sBottom);
		scale = mf->autoscale;
		for(k=0; k<3; k++) sFrame->scale[k] *= scale;
		sFrame->translate[VX] *= scale;
		sFrame->translate[VY] *= scale;
		sFrame->translate[VZ] = mFrame->translate[VZ] + (sBottom-bottom) * scale;
	}*/
}

void R_ScaleModelToSprite(int sprite, int frame)
{
	spritedef_t		*spr = sprites + sprite;
	modeldef_t		*modef = models + spr->modeldef;
	modelframe_t	*mf = &modef->modelframes[frame];
	int				lump = spr->spriteframes[frame].lump[0];
	int				off = (spritetopoffset[lump] >> FRACBITS) - spriteheight[lump];

	if(off < 0) off = 0;
	R_ScaleModel(mf, spriteheight[lump], off);
}

// Create a new modeldef or find the named one.
modeldef_t *R_GetModelDef(char *name, int create_if_missing)
{
	int		i;
	modeldef_t *newlist, *md;

	// First try to find an existing modef with the name.
	for(i=0; i<nummodels; i++)
		if(!stricmp(name, models[i].name))
			return models + i;

	if(!create_if_missing) return NULL;

	// No match. Create a new modef.
	newlist = (modeldef_t *)Z_Malloc(sizeof(modeldef_t) * (nummodels+1), PU_STATIC, NULL);
	if(nummodels)
	{
		// Copy the old list.
		memcpy(newlist, models, sizeof(modeldef_t) * nummodels);
		// Destroy the old list.
		Z_Free(models);
	}
	models = newlist;
	md = models + nummodels;
	nummodels++;
	memset(md, 0, sizeof(*md));
	strcpy(md->name, name);
	return md;
}

modelframe_t *R_GetModelDefFrame(modeldef_t *md, int frame)
{
	modelframe_t *newlist, *ret;

	if(frame < 0) return NULL;
	if(frame < md->numframes) // In range?
		return md->modelframes + frame;
	if(frame > 1023)
		I_Error("R_GetModelDefFrame: Max number of frames is 1024!\n");
	
	// Allocate some new frames.
	newlist = (modelframe_t *)Z_Malloc(sizeof(modelframe_t) * (frame+1), PU_STATIC, NULL);
	memset(newlist, 0, sizeof(modelframe_t) * (frame+1));
	if(md->numframes)
	{
		memcpy(newlist, md->modelframes, sizeof(modelframe_t) * md->numframes);
		Z_Free(md->modelframes);
	}
	md->modelframes = newlist;
	md->numframes++;
	ret = md->modelframes + frame;
	ret->scale = 1;
	return ret;
}

// Autoscaling is done here.
void R_SetSpriteReplacement(int sprite, char *modelname)
{
	modeldef_t *modef = R_GetModelDef(modelname, false);
	spritedef_t *spr = sprites + sprite;
	int	i;

	// Does the model exist?
	if(!modef) return;
	if(sprite >= numsprites) return;
	
	spr->modeldef = modef - models;

	for(i=0; i<spr->numframes && i<modef->numframes; i++)
	{
		if(modef->modelframes[i].sub[0].model 
			&& modef->modelframes[i].sub[0].flags & MFF_AUTOSCALE)
			R_ScaleModelToSprite(sprite, i);
	}
}

void R_ReadModelDef(char *filename)
{
	FILE	*file;
	int		line_number;
	char	buff[256], *ptr;	
	modeldef_t *modef = NULL;
	modelframe_t *mf = NULL;
	int		frame_num = 0, skin_num, selector;
	char	md2file[256], temp[256], framename[16];
	int		globalFlags = 0;
	int		defaultFlags = 0;
	float	defaultSize, defaultScale, defaultOffset;

	// Open the file.
	if((file = fopen(filename, "rt")) == NULL)
	{
		ST_Message( "R_ReadModelDef: failed to open %s.\n", filename);
		return;
	}

	ST_Message( "R_ReadModelDef: %s.\n", filename);

	strcpy(md2file, "");

	// Start reading the definitions.
	for(line_number=1, readline(buff, 255, file); !feof(file); 
		readline(buff, 255, file), line_number++)
	{
		ptr = firstchar(buff);
		if(*ptr == '#' || *ptr == 0) continue;	// A comment.
		if(*ptr == '*') // Model name.
		{
#if 0
			spr = NULL;
			ptr = firstchar(ptr+1);
			for(i=0; spritenamelist[i]; i++)
				if(!stricmp(ptr, spritenamelist[i]))
				{
					// This is it!
					spr = sprites + i;
					
					/*ST_Message("%s(%d): sprite found: %s = %i\n", 
						filename, line_number, spritenamelist[i], i);*/

					// Init the model array for this sprite.
					if(!spr->modelframes)
					{
						spr->modelframes = (modelframe_t*) 
							Z_Malloc(sizeof(modelframe_t) * spr->numframes, 
							PU_SPRITE, NULL);
						memset(spr->modelframes, 0, sizeof(modelframe_t) * spr->numframes);
						defaultFlags = 0;
					}
					selector = 0;
					frame_num = 0;
					mf = NULL;
					break;
				}
			if(!spr)
			{
				// Print a warning.
				ST_Message( "%s(%d): invalid sprite name '%s'.\n",
					filename, line_number, ptr);
			}
			continue;
#endif
			ptr = firstchar(ptr+1);

			if(!stricmp(ptr, "Config"))
				modef = NULL;
			else
				modef = R_GetModelDef(ptr, true);

			selector = 0;
			frame_num = 0;
			defaultFlags = 0;
			defaultSize = 0;
			defaultScale = 1;
			defaultOffset = 0;
			mf = 0;
			continue;
		}
		if(*ptr == '(')	// Flags?
		{
			byte fl = 0;
			for(;;)
			{
				ptr = firstchar(ptr+1);
				if(!strnicmp(ptr, "fullbright", 10))
				{
					fl |= MFF_FULLBRIGHT;
					ptr += 10;
				}
				else if(!strnicmp(ptr, "shadow1", 7))
				{
					fl |= MFF_SHADOW1;
					ptr += 7;
				}
				else if(!strnicmp(ptr, "shadow2", 7))
				{
					fl |= MFF_SHADOW2;
					ptr += 7;
				}
				else if(!strnicmp(ptr, "brightshadow", 12))
				{
					fl |= MFF_BRIGHTSHADOW;
					ptr += 12;
				}
				else if(!strnicmp(ptr, "autoscale", 9))
				{
					fl |= MFF_AUTOSCALE;
					ptr += 9;
				}
				else if(!strnicmp(ptr, "movpitch", 8))
				{
					fl |= MFF_MOVEMENT_PITCH;
					ptr += 8;
				}
				else if(!strnicmp(ptr, "spin", 4))
				{
					fl |= MFF_SPIN;
					ptr += 4;
				}
				else if(!strnicmp(ptr, "scale:", 6))
				{
					*(mf? &mf->scale : &defaultScale) = strtod(ptr+6, &ptr);
				}
				else if(!strnicmp(ptr, "height:", 7))
				{
					*(mf? &mf->size : &defaultSize) = strtod(ptr+7, &ptr);
				}
				else if(!strnicmp(ptr, "offset:", 7))
				{
					*(mf? &mf->offset : &defaultOffset) = strtod(ptr+7, &ptr);
				}
				else break;	
			}
			if(!modef) globalFlags = fl;
			else if(!mf) defaultFlags = fl;
			else mf->sub[selector].flags ^= fl;
			continue;
		}
		if(!modef) continue;

		// It is a frame definition line:
		// <sprite frame> <md2 file> <frame name> [<skin number>]
		if(*ptr == '+')
		{
			// An additional model for the same frame?
			selector++;
			if(selector >= MAX_FRAME_MODELS)
			{
				ST_Message( "%s(%d): too many frame models.\n",
					filename, line_number);
				continue;
			}
			ptr = firstchar(ptr+1);
		}
		else
		{
			frame_num = strtol(ptr, &ptr, 10);
			selector = 0;
		}
		ptr = firstchar(ptr);
		ptr = copyword(temp, ptr);
		if(temp[0] != ':') // Repeat last?
			strcpy(md2file, temp);
		ptr = copyword(framename, ptr);
		skin_num = strtol(ptr, NULL, 10);
		
		/*ST_Message( "  %i: [%i] md2:'%s' frame:'%s' skin:%i\n", 
			frame_num, selector, md2file, framename, skin_num);*/
		
		// Check that the frame is OK.
		if(/*frame_num >= spr->numframes || */frame_num < 0)
		{
			ST_Message( "%s(%d): invalid frame number %i.\n",
				filename, line_number, frame_num);
			continue;
		}
		//mf = spr->modelframes + frame_num;
		mf = R_GetModelDefFrame(modef, frame_num);
		mf->sub[selector].model = R_LoadModel(md2file);
		mf->sub[selector].frame = R_ModelFrameNumForName(mf->sub[selector].model, framename);
		mf->sub[selector].skin = skin_num;
		mf->sub[selector].flags = globalFlags ^ defaultFlags;
		
		if(!selector)
		{
			mf->scale = defaultScale;
			mf->size = defaultSize;
			mf->offset = defaultOffset;
		}

		/*ST_Message( "  %i: [%i] md2:%i frame:%i skin:%i\n", 
			frame_num, selector, mf->sub[selector].model, mf->sub[selector].frame, skin_num);*/
	}

	// We're done, close the file.
	fclose(file);
}

// Sprites must be initialized before this. 
// Here we load in the model definition file.
// The -mdef option can be used to specify a particular definition file.
// By default, ModelDef.txt is loaded.
void R_InitModels(void)
{
	int				i, k;
	modelframe_t	*mf;

	// Dedicated servers do nothing with models.
//	if(isDedicated) return;

	models = NULL;
	nummodels = 0;

	i = M_CheckParm("-mdef");
	if(!i)
	{
		R_ReadModelDef(DEFAULT_MDEF_NAME);
	}
	else 
	{
		// Read all the files listed after -mdef.
		for(++i; i<myargc; i++)
		{
			if(myargv[i][0] != '-')
				R_ReadModelDef(myargv[i]);
			else break;
		}
	}
/*	for(i=0; i<nummodels; i++)
	{
		int k, p;
		ST_Message( "Model %i: %s, %i frames\n", i, models[i].name, models[i].numframes);
		for(p=0; p<models[i].numframes; p++)
		{
			modelframe_t *mf = models[i].modelframes + p;
			for(k=0; k<MAX_FRAME_MODELS; k++)
			{
				if(!mf->sub[k].model) continue;
				ST_Message("  [%i]: md=%i fr=%i sk=%i fl=%x\n", 
					k, mf->sub[k].model, mf->sub[k].frame, 
					mf->sub[k].skin, mf->sub[k].flags);
			}
		}
	}*/

/*	for(i=0; i<numsprites; i++)
	{
		if(!sprites[i].modelframes) continue;
		for(k=0; k<sprites[i].numframes; k++)
			if(sprites[i].modelframes[k].sub[0].model)
				R_ScaleModel(i, k);	
	}*/

	// Scale and offset each frame that needs it.
	for(i=0; i<nummodels; i++)
		for(k=0, mf=models[i].modelframes; k<models[i].numframes; k++, mf++)
		{
			if(!mf->sub[0].model) continue;
			if(mf->offset == 0 && mf->size == 0 && mf->scale == 1)
				continue;
			/*ST_Message( "Scaling model %i, frame %i: scale=%f\n", 
				i, k, mf->scale);*/
			if(mf->size) R_ScaleModel(mf, mf->size, mf->offset);
		}
}

static void SkinFileName(char *dest, char *modelfile, char *skinfile)
{
	int		i;

	strcpy(dest, modelfile);
	// Find the last / or \.
	for(i=strlen(dest)-1; i>=0; i--)
		if(dest[i] == '/' || dest[i] == '\\')
		{
			strcpy(dest + i+1, skinfile);
			break;
		}
}

// The buffer must be big enough to hold the RGB version of the image.
// This is a PCX loader, partly borrowed from the Q2 utils source (lbmlib.c).
void R_LoadSkinRGB(model_t *mdl, int skin, byte *outBuffer)
{
	char	namebuff[256];
	FILE	*file;
	byte	*raw, *palette;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*pix;

	// Figure out the name of the skin PCX.
	SkinFileName(namebuff, mdl->fileName, mdl->skins[skin].name);	

	//CON_Printf( "Loading skin '%s'.\n", namebuff);

	// Load the file.
	if((file = fopen(namebuff, "rb")) == NULL)
	{
		ST_Message("R_LoadSkinRGB: can't find %s.\n", namebuff);
		return;
	}
	fseek(file, 0, SEEK_END);	// Seek to end.
	len = ftell(file);			// How long?
	fseek(file, 0, SEEK_SET);
	raw = (byte *)Z_Malloc(len, PU_STATIC, 0);
	fread(raw, 1, len, file);
	fclose(file);

	// Parse the PCX file.
	pcx = (pcx_t *)raw;
	raw = &pcx->data;

/*	pcx->xmin = LittleShort(pcx->xmin);
	pcx->ymin = LittleShort(pcx->ymin);
	pcx->xmax = LittleShort(pcx->xmax);
	pcx->ymax = LittleShort(pcx->ymax);
	pcx->hres = LittleShort(pcx->hres);
	pcx->vres = LittleShort(pcx->vres);
	pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
	pcx->palette_type = LittleShort(pcx->palette_type);*/

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
		I_Error ("R_LoadSkinRGB: Bad pcx file %s\n", namebuff);
	
	//if(((byte*)pcx)[len-769] != 12) I_Error("Bad palette\n");

	//CON_Printf( "%s: %i\n", namebuff, ((byte*)pcx)[len-769]);

	palette = (byte *)Z_Malloc(768, PU_STATIC, 0);
	memcpy(palette, ((byte*) pcx) + len - 768, 768); // Palette is in the end.

/*	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	if (!pic)
		return;*/

	/*out = malloc ( (pcx->ymax+1) * (pcx->xmax+1) );
	if (!out)
		Error ("Skin_Cache: couldn't allocate");*/

	//*pic = out;

	pix = outBuffer;

	for(y=0; y<=pcx->ymax; y++, pix += (pcx->xmax+1)*3)
	{
		for(x=0; x<=pcx->xmax; )
		{
			dataByte = *raw++;
			
			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;
			
			while(runLength-- > 0)
			{
				memcpy(pix + x++ * 3, palette + dataByte*3, 3);
			}
		}
	}

	if(raw - (byte *)pcx > len)
		I_Error ("R_LoadSkinRGB: PCX file %s was malformed.\n", namebuff);

	Z_Free(palette);
	Z_Free(pcx);

//	saveTGA24_rgb888("test.tga", 256, 256, outBuffer);
}