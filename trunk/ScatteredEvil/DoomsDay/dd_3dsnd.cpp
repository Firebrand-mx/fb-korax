// Calculation of the aural properties of sectors.

#include "dd_def.h"
#include "i_win32.h"
#include <ctype.h>

enum // Texture types
{
	TEXTYPE_UNKNOWN,
	TEXTYPE_METAL,
	TEXTYPE_ROCK,
	TEXTYPE_WOOD,
	TEXTYPE_CLOTH
};

typedef struct
{
	unsigned int data[NUM_REVERB_DATA];
} subreverb_t;

// This could hold much more detailed information...
typedef struct
{
	char	name[9];	// Name of the texture.
	int		type;		// Which type?
} textype_t;

static textype_t	*texTypes = 0;
static int			numTexTypes = 0;


// (0,1) = top left; (2,3) = bottom right
// Assumes sectors are always closed.
void DD_SectorBoundingBox(sector_t *sec, float *bbox)
{
	guardSlow(DD_SectorBoundingBox);
	float	x, y;
	int		i;
	line_t	*li;
	
	bbox[BLEFT] = bbox[BRIGHT] = sec->lines[0]->v1->x >> FRACBITS;
	bbox[BTOP] = bbox[BBOTTOM] = sec->lines[0]->v1->y >> FRACBITS;
	for(i=1; i<sec->linecount; i++)
	{
		li = sec->lines[i];
		x = li->v1->x >> FRACBITS;
		y = li->v1->y >> FRACBITS;
		if(x < bbox[BLEFT]) bbox[BLEFT] = x;
		if(x > bbox[BRIGHT]) bbox[BRIGHT] = x;
		if(y < bbox[BTOP]) bbox[BTOP] = y;
		if(y > bbox[BBOTTOM]) bbox[BBOTTOM] = y;
	}
	unguardSlow;
}

// Calculate the reverb settings for each sector.
void S_CalcSectorReverbs()
{
	guard(S_CalcSectorReverbs);
	int			i, c, type, k;
	subsector_t	*sub;
	sector_t	*sec;
	seg_t		*seg;
	float		total, metal, rock, wood, cloth;
	subreverb_t *sub_reverb, *rev;

	//gi.Message( "P_CalcSectorReverbs: begin at %i\n", gi.GetTime());

	// Allocate memory for the subsector reverb data.
	sub_reverb = (subreverb_t*) Z_Malloc(sizeof(subreverb_t) * numsubsectors,
		PU_STATIC, 0);
	memset(sub_reverb, 0, sizeof(subreverb_t) * numsubsectors);

	// First determine each subsectors' individual characteristics.
	//ST_Message("%d bytes; sub_reverb: %p\n", sizeof(subreverb_t) * numsubsectors, sub_reverb);
	for(c=0; c<numsubsectors; c++)
	{
		sub = SUBSECTOR_PTR(c);
		rev = &sub_reverb[c];
		// Space is the rough volume of the subsector (bounding box).
		rev->data[SRD_SPACE] = ((sub->sector->ceilingheight - sub->sector->floorheight) >> FRACBITS)
			* (sub->bbox[1].x - sub->bbox[0].x) * (sub->bbox[1].y - sub->bbox[0].y);

		//gi.Message( "sub %i: volume %f Mu\n", c, volume/1e6);
		/*	
		i = (int) (volume/1e4);
		if(i < 5) i = 5;
		if(i > 255) i = 255;
		sub->reverb[SSRD_SPACE] = i;
		*/
		
		// The other reverb properties can be found out by taking a look at the
		// walls surrounding the subsector (floors and ceilings are currently
		// ignored).
		total = metal = rock = wood = cloth = 0;
		for(i=0; i<sub->numlines; i++)
		{
			seg = SEG_PTR(sub->firstline + i);
			if(!seg->linedef || !seg->sidedef->midtexture) continue;
			total += seg->length;
			// The texture of the seg determines its type.
			type = S_TextureTypeForName(R_TextureNameForNum(seg->sidedef->midtexture));
			switch(type)
			{
			case TEXTYPE_METAL:
				metal += seg->length;
				break;

			case TEXTYPE_ROCK:
				rock += seg->length;
				break;

			case TEXTYPE_WOOD:
				wood += seg->length;
				break;

			case TEXTYPE_CLOTH:
				cloth += seg->length;
				break;

			default:
				// The type of the texture is unknown. Assume it's wood.
				wood += seg->length;
			}
		}
		if(!total) continue; // Huh?
		metal /= total;
		rock /= total;
		wood /= total;
		cloth /= total;

		// Volume.
		i = (int) ( metal*255 + rock*200 + wood*80 + cloth*5 );
		if(i < 0) i = 0;
		if(i > 255) i = 255;
		rev->data[SRD_VOLUME] = i;

		// Decay time.
		i = (int) ( metal*255 + rock*160 + wood*50 + cloth*5 );
		if(i < 0) i = 0;
		if(i > 255) i = 255;
		rev->data[SRD_DECAY] = i;

		// High frequency damping.
		i = (int) ( metal*25 + rock*100 + wood*200 + cloth*255 );
		if(i < 0) i = 0;
		if(i > 255) i = 255;
		rev->data[SRD_DAMPING] = i;

		/*ST_Message( "sub %04i: vol:%3i sp:%3i dec:%3i dam:%3i\n", c, rev->data[SRD_VOLUME],
			rev->data[SRD_SPACE], rev->data[SRD_DECAY], rev->data[SRD_DAMPING]);*/
	}
/*	ST_Message("sub_reverb: %p\n", sub_reverb);
	for(c=0; c<numsubsectors; c++)
	{
		rev = &sub_reverb[c];
		ST_Message( "sub %04i: vol:%3i sp:%3i dec:%3i dam:%3i\n", c, rev->data[SRD_VOLUME],
			rev->data[SRD_SPACE], rev->data[SRD_DECAY], rev->data[SRD_DAMPING]);
	}*/
	
	for(c=0; c<numsectors; c++)
	{
		float bbox[4], spaceScatter;
		unsigned int sectorSpace;
		sec = SECTOR_PTR(c);
		DD_SectorBoundingBox(sec, bbox);
		/*ST_Message( "sector %i: (%f,%f) - (%f,%f)\n", c, 
			bbox[BLEFT], bbox[BTOP], bbox[BRIGHT], bbox[BBOTTOM]);*/

		sectorSpace = ((sec->ceilingheight - sec->floorheight) >> FRACBITS)
			* (bbox[BRIGHT] - bbox[BLEFT]) * (bbox[BBOTTOM] - bbox[BTOP]);

		//ST_Message("sector %i: secsp:%i\n", c, sectorSpace);

		bbox[BLEFT] -= 128;
		bbox[BRIGHT] += 128;
		bbox[BTOP] -= 128;
		bbox[BBOTTOM] += 128;

		for(k=0, i=0; i<numsubsectors; i++)
		{
			sub = SUBSECTOR_PTR(i);
			rev = sub_reverb + i;
			// Is this subsector close enough?
			if(sub->midpoint.x > bbox[BLEFT] && sub->midpoint.x < bbox[BRIGHT] 
				&& sub->midpoint.y > bbox[BTOP] && sub->midpoint.y < bbox[BBOTTOM])
			{
				//ST_Message( "- sub %i within, own:%i\n", i, sub->sector == sec);
				k++; 
				sec->reverb[SRD_SPACE] += rev->data[SRD_SPACE];
				sec->reverb[SRD_VOLUME] += rev->data[SRD_VOLUME] / 255.0f * rev->data[SRD_SPACE];
				sec->reverb[SRD_DECAY] += rev->data[SRD_DECAY] / 255.0f * rev->data[SRD_SPACE];
				sec->reverb[SRD_DAMPING] += rev->data[SRD_DAMPING] / 255.0f * rev->data[SRD_SPACE];
			}
		}
		if(sec->reverb[SRD_SPACE])
		{
			spaceScatter = sectorSpace / sec->reverb[SRD_SPACE];
			// These three are weighted by the space.
			sec->reverb[SRD_VOLUME] /= sec->reverb[SRD_SPACE];
			sec->reverb[SRD_DECAY] /= sec->reverb[SRD_SPACE];
			sec->reverb[SRD_DAMPING] /= sec->reverb[SRD_SPACE];
/*			ST_Message("sector %i: sp:%f vol:%f dec:%f dam:%f\n",
				c,
				sec->reverb[SRD_SPACE], 
				sec->reverb[SRD_VOLUME], 
				sec->reverb[SRD_DECAY], 
				sec->reverb[SRD_DAMPING]); */
		}
		else
		{
			spaceScatter = 0;
			sec->reverb[SRD_VOLUME] = .2f;
			sec->reverb[SRD_DECAY] = .4f;
			sec->reverb[SRD_DAMPING] = 1;
		}
		/*ST_Message( "sector %i: secSp:%fM revSp:%fM scatter: %f\n", 
			c, sectorSpace/1e6f, sec->reverb[SRD_SPACE]/1e6f, spaceScatter);*/

		// If the space is scattered, the reverb effect lessens.
		sec->reverb[SRD_SPACE] /= spaceScatter > .8? 10 : spaceScatter > .6? 4 : 1;

		// Scale the reverb space to a reasonable range, so that 0 is very small and
		// .99 is very large. 1.0 is only for open areas.
		sec->reverb[SRD_SPACE] /= 120e6;
		if(sec->reverb[SRD_SPACE] > .99) sec->reverb[SRD_SPACE] = .99f;
		
		if(sec->ceilingpic == skyflatnum) // An open sector?
		{
			// An open sector can still be small. In that case the reverb
			// is diminished a bit.
			if(sec->reverb[SRD_SPACE] > .5) 
				sec->reverb[SRD_VOLUME] = 1; // Full volume.
			else 
				sec->reverb[SRD_VOLUME] = .5f; // Small sector, but still open.
			sec->reverb[SRD_SPACE] = 1;
		}
		else // A closed sector.
		{
			// Large spaces have automatically a bit more audible reverb.
			sec->reverb[SRD_VOLUME] += sec->reverb[SRD_SPACE] / 4;
		}
		if(sec->reverb[SRD_VOLUME] > 1) sec->reverb[SRD_VOLUME] = 1;
/*		sec->reverbDecay /= k/2.0f;
		sec->reverbDamping /= k;*/
	}

	Z_Free(sub_reverb);
		
	//gi.Message( "P_CalcSectorReverbs: end at %i\n", gi.GetTime());
	unguard;
}


//----------------------------------------------------------------------
// Texture types
//----------------------------------------------------------------------

void readline(char *buffer, int len, FILE *file)
{
	int		p;

	fgets(buffer, len, file);
	p = strlen(buffer)-1;
	if(buffer[p] == '\n') buffer[p] = 0;
}

char *firstchar(char *buffer)
{
	int		i = 0;

	while(isspace(buffer[i]) && buffer[i]) i++;
	return buffer + i;
}

void S_LoadTextureTypes()
{
	guard(S_LoadTextureTypes);
	FILE	*file;
	char	buff[256], *ptr;
	int		curtype = TEXTYPE_UNKNOWN;
	textype_t *tt;

	if((file=fopen("textypes.txt", "rt")) == NULL)
	{
		ST_Message( "S_LoadTextureTypes: textypes.txt not found.\n");
		return;
	}

	for(readline(buff, 255, file); !feof(file); readline(buff, 255, file))
	{
		ptr = firstchar(buff);
		if(*ptr == '#' || *ptr == 0) continue; // A comment.
		if(*ptr == '*')
		{
			ptr = firstchar(ptr+1);
			if(!stricmp(ptr, "metal"))
				curtype = TEXTYPE_METAL;
			else if(!stricmp(ptr, "rock"))
				curtype = TEXTYPE_ROCK;
			else if(!stricmp(ptr, "wood"))
				curtype = TEXTYPE_WOOD;
			else if(!stricmp(ptr, "cloth"))
				curtype = TEXTYPE_CLOTH;
			continue;
		}
		// Allocate a new textype entry.
		texTypes = (textype_t *)realloc(texTypes, sizeof(textype_t) * ++numTexTypes);
		// -JL- Paranoia
		if (!texTypes)
			I_Error("S_LoadTextureTypes: realloc failed");
		tt = texTypes + numTexTypes-1;
		memset(tt, 0, sizeof(*tt));
		strncpy(tt->name, ptr, 8);
		tt->type = curtype;
	}
	ST_Message( "%d texture types loaded.\n", numTexTypes);
	fclose(file);
	unguard;
}

void S_FreeTextureTypes()
{
	guard(S_FreeTextureTypes);
	free(texTypes);
	texTypes = 0;
	numTexTypes = 0;
	unguard;
}

int S_TextureTypeForName(char *name)
{
	guard(S_TextureTypeForName);
	int		i;

	for(i=0; i<numTexTypes; i++)
		if(!strnicmp(name, texTypes[i].name, 8))
			return texTypes[i].type;
	return TEXTYPE_UNKNOWN;
	unguard;
}
