
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
#include <math.h>
#include "dd_def.h"
#include "i_win32.h"
#include "console.h"
#include "gl_def.h"
#include "tga.h"

//#include "h2def.h"
//#include "p_local.h"
//#include "soundst.h"


// MACROS ------------------------------------------------------------------

#define MALLOC_CLIB 1
#define MALLOC_ZONE 2

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int ReadFile(char const *name, byte **buffer, int mallocType);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------
//extern char *SavePath;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int myargc;
char **myargv;

char cfgFile[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

int	Argc(void)
{
	return myargc;
}

char *Argv(int i)
{
	if(i<0 || i>myargc-1) I_Error("Argv: Request out of range.\n");
	return myargv[i];
}

char **ArgvPtr(int i)
{
	if(i<0 || i>myargc-1) I_Error("ArgvPtr: Request out of range.\n");
	return &myargv[i];
}

//==========================================================================
//
// M_CheckParm
//
// Checks for the given parameter in the program's command line arguments.
// Returns the argument number (1 to argc-1) or 0 if not present.
//
//==========================================================================

int M_CheckParm(char *check)
{
	int i;

	for(i = 1; i < myargc; i++)
	{
		if(!strcasecmp(check, myargv[i]))
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//
//==========================================================================

boolean M_ParmExists(char *check)
{
	return M_CheckParm(check) != 0 ? true : false;
}

//==========================================================================
//
// M_ExtractFileBase
//
//==========================================================================

void M_ExtractFileBase(char *path, char *dest)
{
	char *src;
	int length;

	src = path+strlen(path)-1;

	// Back up until a \ or the start
	while(src != path && *(src-1) != '\\' && *(src-1) != '/')
	{
		src--;
	}

	// Copy up to eight characters
	memset(dest, 0, 8);
	length = 0;
	while(*src && *src != '.')
	{
		if(++length == 9)
		{
			I_Error("Filename base of %s > 8 chars", path);
		}
		*dest++ = toupper((int)*src++);
	}
}

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
/*unsigned char rndtable[256] = {
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
};*/

/*unsigned char *usedRndtable = NULL;

int backup_rndindex, backup_prndindex;
int rndindex = 0;
int prndindex = 0;

void SetRandomTable(unsigned char *table)
{
	usedRndtable = table;
}

unsigned char P_Random (void)
{
	return usedRndtable[(++prndindex)&0xff];
}

int M_Random (void)
{
	rndindex = (rndindex+1)&0xff;
	return usedRndtable[rndindex];
}

void M_ClearRandom (void)
{
	rndindex = prndindex = 0;
}

void M_SaveRandom (void)
{
	backup_rndindex = rndindex;
	backup_prndindex = prndindex;
}

void M_RestoreRandom (void)
{
	rndindex = backup_rndindex;
	prndindex = backup_prndindex;
}*/


void M_ClearBox (fixed_t *box)
{
	box[BOXTOP] = box[BOXRIGHT] = DDMININT;
	box[BOXBOTTOM] = box[BOXLEFT] = DDMAXINT;
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

/*
==================
=
= M_WriteFile
=
==================
*/

#ifndef O_BINARY
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
		I_Error("Couldn't read file %s\n", name);
	}
	if(fstat(handle, &fileinfo) == -1)
	{
		I_Error("Couldn't read file %s\n", name);
	}
	length = fileinfo.st_size;
	if(mallocType == MALLOC_ZONE)
	{ // Use zone memory allocation
		buf = (byte *)Z_Malloc(length, PU_STATIC, NULL);
	}
	else
	{ // Use c library memory allocation
		buf = (byte *)malloc(length);
		if(buf == NULL)
		{
			I_Error("Couldn't malloc buffer %d for file %s.\n",
				length, name);
		}
	}
	count = read(handle, buf, length);
	close(handle);
	if(count < length)
	{
		I_Error("Couldn't read file %s\n", name);
	}
	*buffer = buf;
	return length;
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

void writeCommented(FILE *file, char *text)
{
	char *buff = (char *)malloc(strlen(text)+1), *line;
	int	i = 0;

	// -JL- Paranoia
	if (!buff)
		I_Error("writeCommented: malloc failed");
	strcpy(buff, text);
	line = strtok(buff, "\n");
	while(line)
	{
		fprintf(file, "# %s\n", line);
		line = strtok(NULL, "\n");
	}
	free(buff);
}

void M_WriteTextEsc(FILE *file, char *text)
{
	int		i;

	for(i=0; text[i]; i++)
	{
		if(text[i] == '"' || text[i] == '\\') fprintf(file, "\\");
		fprintf(file, "%c", text[i]);
	}
}

// Saves all bindings and archived console variables.
// Writes console commands.
void M_SaveDefaults()
{
	extern cvar_t	*cvars;
	extern int		numCVars;
	int				i;
	cvar_t			*var;
	FILE			*file;

	if((file = fopen(cfgFile, "wt")) == NULL)
	{
		ST_Message("M_SaveDefaults: Can't open %s for writing.\n", cfgFile);
		return;
	}

	fprintf(file, "# Korax Engine "DOOMSDAY_VERSION_TEXT"\n");
	fprintf(file, "# This defaults file is generated automatically. Each line is a console\n");
	fprintf(file, "# command. Lines beginning with # are comments. Use autoexec.cfg for\n");
	fprintf(file, "# your own startup commands.\n\n");

	fprintf(file, "#\n# CONSOLE VARIABLES\n#\n\n");

	// We'll write all the console variables that are flagged for archiving.		
	for(i=0; i<numCVars; i++)
	{
		var = cvars + i;
		if(var->type == CVT_NULL || var->flags & CVF_NO_ARCHIVE) continue;
		
		// First print the comment (help text).
		writeCommented(file, var->help);
		fprintf(file, "%s ", var->name);
		if(var->flags & CVF_PROTECTED) fprintf(file, "force ");
		if(var->type == CVT_BYTE) fprintf(file, "%d", *(byte*)var->ptr);
		if(var->type == CVT_INT) fprintf(file, "%d", *(int*)var->ptr);
		if(var->type == CVT_FLOAT) fprintf(file, "%f", *(float*)var->ptr);
		if(var->type == CVT_CHARPTR) 
		{
			fprintf(file, "\"");
			M_WriteTextEsc(file, *(char**) var->ptr);
			fprintf(file, "\"");
		}
		fprintf(file, "\n\n");
	}
	
	fprintf(file, "#\n# BINDINGS\n#\n\n");

	B_WriteToFile(file);	

	fclose(file);
}

static void readline(char *buffer, int len, FILE *file)
{
	int		p;

	fgets(buffer, len, file);
	p = strlen(buffer)-1;
	if(buffer[p] == '\n') buffer[p] = 0;
}

static int iscomment(char *buffer)
{
	int		i = 0;

	while(isspace(buffer[i]) && buffer[i]) i++;
	if(buffer[i] == '#') return true;
	return false;
}

int M_ParseCommands(char *fileName, int setdefault)
{
	FILE	*file;
	char	buff[512];
	int		line = 1;

	// Is this supposed to be the default?
	if(setdefault)
		strcpy(cfgFile, fileName);
	
	// Open the file.
	if((file = fopen(fileName, "rt")) == NULL) return false;
	
	// This file is filled with console commands.
	// Each line is a command.

	readline(buff, 512, file);
	while(!feof(file))
	{
		if(!iscomment(buff))
		{
			// Execute the commands silently.
			if(!CON_Execute(buff, setdefault? true : false))
				ST_Message( "%s(%d): error executing command\n  \"%s\"\n", fileName, line, buff);
		}
		readline(buff, 512, file);
		line++;
	}
	fclose(file);	
	return true;
}



/*
===================
=
= P_AproxDistance
=
= Gives an estimation of distance (not exact)
=
===================
*/

fixed_t M_AproxDistance (fixed_t dx, fixed_t dy)
{
	dx = abs(dx);
	dy = abs(dy);
	if (dx < dy)
		return dx+dy-(dx>>1);
	return dx+dy-(dy>>1);
}


// Writes a targa file of the specified depth.
int M_ScreenShot(char *filename, int bits)
{
	byte *screen;

	if(bits != 16 && bits != 24) return false;

	// Grab that screen!
	screen = GL_GrabScreen();

	if(bits == 16)
		saveTGA16_rgb888(filename, screenWidth, screenHeight, screen);
	else 
		saveTGA24_rgb888(filename, screenWidth, screenHeight, screen);	

	free(screen);	
	return true;
}