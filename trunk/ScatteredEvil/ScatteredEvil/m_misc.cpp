
//**************************************************************************
//**
//** m_misc.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
#include <conio.h>

#include <ctype.h>
#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

/*
===============
=
= M_Random
=
= Returns a 0-255 number
=
===============
*/


// This is the new flat distribution table
unsigned char rndtable[256] = {
	201,  1,243, 19, 18, 42,183,203,101,123,154,137, 34,118, 10,216,
	135,246,  0,107,133,229, 35,113,177,211,110, 17,139, 84,251,235,
	182,166,161,230,143, 91, 24, 81, 22, 94,  7, 51,232,104,122,248,
	175,138,127,171,222,213, 44, 16,  9, 33, 88,102,170,150,136,114,
	 62,  3,142,237,  6,252,249, 56, 74, 30, 13, 21,180,199, 32,132,
	187,234, 78,210, 46,131,197,  8,206,244, 73,  4,236,178,195, 70,
	121, 97,167,217,103, 40,247,186,105, 39, 95,163, 99,149,253, 29,
	119, 83,254, 26,202, 65,130,155, 60, 64,184,106,221, 93,164,196,
	112,108,179,141, 54,109, 11,126, 75,165,191,227, 87,225,156, 15,
	 98,162,116, 79,169,140,190,205,168,194, 41,250, 27, 20, 14,241,
	 50,214, 72,192,220,233, 67,148, 96,185,176,181,215,207,172, 85,
	 89, 90,209,128,124,  2, 55,173, 66,152, 47,129, 59, 43,159,240,
	239, 12,189,212,144, 28,200, 77,219,198,134,228, 45, 92,125,151,
	  5, 53,255, 52, 68,245,160,158, 61, 86, 58, 82,117, 37,242,145,
	 69,188,115, 76, 63,100, 49,111,153, 80, 38, 57,174,224, 71,231,
	 23, 25, 48,218,120,147,208, 36,226,223,193,238,157,204,146, 31
};


int backup_prndindex;
int rndindex = 0;
int prndindex = 0;

int M_Random (void)
{
	rndindex = (rndindex+1)&0xff;
	return rndtable[rndindex];
}

void M_ClearRandom (void)
{
	rndindex = prndindex = 0;
}

void P_SaveRandom (void)
{
	backup_prndindex = prndindex;
}

void P_RestoreRandom (void)
{
	prndindex = backup_prndindex;
}

/*
void M_ClearBox (fixed_t *box)
{
	box[BOXTOP] = box[BOXRIGHT] = H2MININT;
	box[BOXBOTTOM] = box[BOXLEFT] = H2MAXINT;
}

void M_AddToBox (fixed_t *box, fixed_t x, fixed_t y)
{
	if (x<box[BOXLEFT])
		box[BOXLEFT] = x;
	else if (x>box[BOXRIGHT])
		box[BOXRIGHT] = x;
	if (y<box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	else if (y>box[BOXTOP])
		box[BOXTOP] = y;
}
*/
/*
==================
=
= M_WriteFile
=
==================
*/

/*#ifndef O_BINARY
#define O_BINARY 0
#endif

boolean M_WriteFile (char const *name, void *source, int length)
{
	int handle, count;

	handle = open (name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
	if (handle == -1)
		return false;
	count = write (handle, source, length);
	close (handle);

	if (count < length)
		return false;

	return true;
}

//==========================================================================
//
// M_ReadFile
//
// Read a file into a buffer allocated using Z_Malloc().
//
//==========================================================================

int M_ReadFile(char const *name, byte **buffer)
{
	return ReadFile(name, buffer, MALLOC_ZONE);
}

//==========================================================================
//
// M_ReadFileCLib
//
// Read a file into a buffer allocated using malloc().
//
//==========================================================================

int M_ReadFileCLib(char const *name, byte **buffer)
{
	return ReadFile(name, buffer, MALLOC_CLIB);
}

//==========================================================================
//
// ReadFile
//
//==========================================================================

static int ReadFile(char const *name, byte **buffer, int mallocType)
{
	int handle, count, length;
	struct stat fileinfo;
	byte *buf;

	handle = open(name, O_RDONLY|O_BINARY, 0666);
	if(handle == -1)
	{
		gi.Error("Couldn't read file %s", name);
	}
	if(fstat(handle, &fileinfo) == -1)
	{
		gi.Error("Couldn't read file %s", name);
	}
	length = fileinfo.st_size;
	if(mallocType == MALLOC_ZONE)
	{ // Use zone memory allocation
		buf = Z_Malloc(length, PU_STATIC, NULL);
	}
	else
	{ // Use c library memory allocation
		buf = malloc(length);
		if(buf == NULL)
		{
			gi.Error("Couldn't malloc buffer %d for file %s.",
				length, name);
		}
	}
	count = read(handle, buf, length);
	close(handle);
	if(count < length)
	{
		gi.Error("Couldn't read file %s", name);
	}
	*buffer = buf;
	return length;
}

//---------------------------------------------------------------------------
//
// PROC M_FindResponseFile
//
//---------------------------------------------------------------------------

#define MAXARGVS 100

void M_FindResponseFile(void)
{
	int i;

	for(i = 1; i < myargc; i++)
	{
		if(myargv[i][0] == '@')
		{
			FILE *handle;
			int size;
			int k;
			int index;
			int indexinfile;
			char *infile;
			char *file;
			char *moreargs[20];
			char *firstargv;

			// READ THE RESPONSE FILE INTO MEMORY
			handle = fopen(&myargv[i][1], "rb");
			if(!handle)
			{

				printf("\nNo such response file!");
				exit(1);
			}
			gi.Message("Found response file %s!\n",&myargv[i][1]);
			fseek (handle,0,SEEK_END);
			size = ftell(handle);
			fseek (handle,0,SEEK_SET);
			file = malloc (size);
			fread (file,size,1,handle);
			fclose (handle);

			// KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
			for (index = 0,k = i+1; k < myargc; k++)
				moreargs[index++] = myargv[k];
			
			firstargv = myargv[0];
			myargv = malloc(sizeof(char *)*MAXARGVS);
			memset(myargv,0,sizeof(char *)*MAXARGVS);
			myargv[0] = firstargv;
			
			infile = file;
			indexinfile = k = 0;
			indexinfile++;  // SKIP PAST ARGV[0] (KEEP IT)
			do
			{
				myargv[indexinfile++] = infile+k;
				while(k < size &&  

					((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
					k++;
				*(infile+k) = 0;
				while(k < size &&
					((*(infile+k)<= ' ') || (*(infile+k)>'z')))
					k++;
			} while(k < size);
			
			for (k = 0;k < index;k++)
				myargv[indexinfile++] = moreargs[k];
			myargc = indexinfile;
			// DISPLAY ARGS
			if(M_CheckParm("-debug"))
			{
				gi.Message("%d command-line args:\n", myargc);
				for(k = 1; k < myargc; k++)
				{
					gi.Message("%s\n", myargv[k]);
				}
			}
			break;
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------

void M_ForceUppercase(char *text)
{
	char c;

	while((c = *text) != 0)
	{
		if(c >= 'a' && c <= 'z')
		{
			*text++ = c-('a'-'A');
		}
		else
		{
			text++;
		}
	}
}
*/
/*
==============================================================================

							DEFAULTS

==============================================================================
*/

#if 0

int     usemouse;
int     usejoystick;

//extern int controls[NUM_CONTROLS];
//extern int mouseControls[NUM_MOUSECONTROLS];
//extern int joyControls[NUM_JOYCONTROLS];

extern boolean messageson;

extern  int     viewwidth, viewheight;


//int defResX, defResY;

extern char *defaultWads;	// A list of wad names, whitespace in between.
extern int repWait1, repWait2; // The initial and secondary repeater delays (tics).

extern  int screenblocks, consoleFlat, consoleTurn, conCompMode;
extern	int maxDynLights, dlBlend, chooseAndUse;
extern	int haloMode, flareBoldness, flareSize, xhair, xhairColor[3], xhairSize;
extern	int lookdirSpeed;

extern char *chat_macros[10];

typedef struct
{
	char    *name;
	int     *location;
	int     defaultvalue;
	int     scantranslate;      // PC scan code hack
	//int     untranslated;       // lousy hack
} default_t;

//#ifndef __NeXT__
extern int snd_Channels;
//extern int snd_DesiredMusicDevice, snd_DesiredSfxDevice;
/*extern int snd_MusicDevice, // current music card # (index to dmxCodes)
			snd_SfxDevice; // current sfx card # (index to dmxCodes)*/

/*#ifdef USEA3D
extern	int snd_3DAudio;
#endif*/

//tern int     snd_SBport, snd_SBirq, snd_SBdma;       // sound blaster variables
//tern int     snd_Mport;                              // midi variables
//#endif

#endif // 0

/*
==============================================================================

						SCREEN SHOTS

==============================================================================
*/

/*
==================
=
= M_ScreenShot
=
==================
*/

void M_ScreenShot (void)
{
	int		i;
	char	fname[12];

	// Find a file name.
	strcpy(fname,"HEXEN00.TGA");
	for(i=0; i<=99; i++)
	{
		fname[5] = i/10 + '0';
		fname[6] = i%10 + '0';
		if(_access(fname, 0) == -1)
			break;  // file doesn't exist
	}
	if(i==100) 
	{
		gi.Message("M_ScreenShot: Couldn't find a valid filename (too many shots already?)");
		return;
	}
	gi.ScreenShot(fname, 24);
	gi.Message("Wrote %s.\n", fname);
}


