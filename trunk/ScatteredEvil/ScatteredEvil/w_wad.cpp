
//**************************************************************************
//**
//** w_wad.c : Heretic 2 : Raven Software, Corp.
//**
//** This version supports runtime (un)loading, replacement of
//** flats and sprites and GWA files.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "h2def.h"
#include "i_win32.h"

// MACROS ------------------------------------------------------------------

#ifdef NeXT
// NeXT doesn't need a binary flag in open call
#define O_BINARY 0
#define strcmpi strcasecmp
#endif


enum // Grouping Tags
{
	GT_NONE,
	GT_FLATS,
	GT_SPRITES,
	NUM_GT
};

// TYPES -------------------------------------------------------------------

typedef struct
{
	char identification[4];
	int numlumps;
	int infotableofs;
} wadinfo_t;

typedef struct
{
	int filepos;
	int size;
	char name[8];
} filelump_t;

typedef struct
{
	char	*start, *end;	// Start and end markers.
} grouping_t; 

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int MarkerForGroup(char *name, boolean begin);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

lumpinfo_t *lumpinfo = 0;
int numlumps = 0;
void **lumpcache = 0;
int numcache = 0;
//int cachesize, initialnumlumps;

// The file records.
int numrecords = 0;
filerecord_t *records = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean loadingForStartup = false;

static grouping_t groups[] =
{
	"", "",
	"F_START", "F_END",		// Flats
	"S_START", "S_END"		// Sprites
};

static lumpinfo_t *PrimaryLumpInfo;
static int PrimaryNumLumps;
static void **PrimaryLumpCache;
static lumpinfo_t *AuxiliaryLumpInfo;
static int AuxiliaryNumLumps;
static void **AuxiliaryLumpCache;
static int AuxiliaryHandle = 0;
boolean AuxiliaryOpened = false;

// CODE --------------------------------------------------------------------

#ifdef NeXT
//==========================================================================
//
// strupr
//
//==========================================================================

void strupr(char *s)
{
    while(*s)
	*s++ = toupper(*s);
}

//==========================================================================
//
// filelength
//
//==========================================================================

int filelength(int handle)
{
    struct stat fileinfo;

    if(fstat(handle, &fileinfo) == -1)
	{
		I_Error("Error fstating");
	}
    return fileinfo.st_size;
}
#endif


static void convertSlashes(char *modifiableBuffer)
{
	unsigned int i;
	for(i=0; i<strlen(modifiableBuffer); i++)
		if(modifiableBuffer[i] == '\\')
			modifiableBuffer[i] = '/';
}

//==========================================================================
//
// File Record Handling
// (all functions named W_Record*)
//
//==========================================================================

// Allocate a new file record.
filerecord_t *W_RecordNew()
{
	filerecord_t	*ptr;
		
	records = (filerecord_t *)realloc(records, sizeof(filerecord_t) * (++numrecords));
	// -JL- Paranoia
	if (!records)
		I_Error("W_RecordNew: realloc failed");
	ptr = records + numrecords-1;
	memset(ptr, 0, sizeof(*ptr));
	return ptr;
}

int W_RecordGetIdx(char *filename)
{
	int		i;
	char	buffer[RECORD_FILENAMELEN];

	// We have to make sure the slashes are correct.
	strcpy(buffer, filename);
	convertSlashes(buffer);

	for(i=0; i<numrecords; i++)
		if(!stricmp(records[i].filename, buffer))
			return i;
	return -1;
}

// Destroy the specified record. Returns true if successful.
boolean W_RecordDestroy(int idx)
{
	if(idx < 0 || idx > numrecords-1) return false;	// Huh?
	
	// First unallocate the memory of the record.				
//	free(records[idx].indices);

	// Collapse the record array.
	if(idx != numrecords-1)
		memmove(records+idx, records+idx+1, sizeof(filerecord_t) * (numrecords-idx-1));

	// Reallocate the records memory.
	numrecords--;	// One less record.
	records = (filerecord_t *)realloc(records, sizeof(filerecord_t) * numrecords);
	// -JL- Paranoia
	if (!records)
		I_Error("W_RecordDestroy: realloc failed");
	
	return true;
}

// Look for the named lump, starting from the specified index.
int W_ScanForName(char *lumpname, int startfrom)
{
	char		name8[9];
	int			v1, v2;
	int			i;
	lumpinfo_t	*lump_p;

	if(startfrom < 0 || startfrom > numlumps-1) return -1;

	memset(name8, 0, sizeof(name8));
	strncpy(name8, lumpname, 8);
	v1 = *(int*) name8;
	v2 = *(int*) (name8+4);
	// Start from the beginning.
	for(i=startfrom, lump_p=lumpinfo+startfrom; i<numlumps; i++, lump_p++)
	{
		if(v1 == *(int*)lump_p->name &&
			v2 == *(int*)(lump_p->name+4))
			return i;
	}
	return -1;
}

//==========================================================================
//
// Lumpinfo inserting, filling, removal and other operations.
//
//==========================================================================

void W_FillLumpInfo(int liIndex, filelump_t *flump, filerecord_t *rec)
{
	lumpinfo_t	*lump = lumpinfo + liIndex;

	lump->handle = rec->handle;
	lump->position = LONG(flump->filepos);
	lump->size = LONG(flump->size);
	strncpy(lump->name, flump->name, 8);
}

// Moves 'count' lumpinfos, starting from 'from'. Updates the lumpcache. 
// Offset can only be positive.
//
// Lumpinfo and lumpcache are assumed to have enough memory for the operation!
//
void W_MoveLumps(int from, int count, int offset)
{
	int		i;

	// Check that our information is valid.
	if(!offset || count <= 0 || from < 0 || from > numlumps-1) return;

	// First update the lumpcache.
	memmove(lumpcache+from+offset, lumpcache+from, sizeof(*lumpcache) * count);
	for(i=from+offset; i<from+count+offset; i++)
		if(lumpcache[i])
			Z_ChangeUser(lumpcache[i], lumpcache+i); // Update the user.
	// Clear the 'revealed' memory.
	if(offset > 0) // Revealed from the beginning.
		memset(lumpcache+from, 0, offset*sizeof(*lumpcache));
	else // Revealed from the end.
		memset(lumpcache+from+count+offset, 0, -offset*sizeof(*lumpcache));

	// Lumpinfo.
	memmove(lumpinfo+from+offset, lumpinfo+from, sizeof(lumpinfo_t) * count);
}

// Moves the rest of the lumps forward.
void W_InsertAndFillLumpRange(int toIndex, filelump_t *lumps, int num,
							  filerecord_t *rec)
{
	int		i;

	// Move lumps if needed.
	if(toIndex < numlumps) W_MoveLumps(toIndex, numlumps-toIndex, num);

	// Now we can just fill in the lumps.
	for(i=0; i<num; i++) W_FillLumpInfo(toIndex+i, lumps+i, rec);

	// Update the number of lumps.
	numlumps += num;
}

void W_RemoveLumpsWithHandle(int handle)
{
	int		i, k, first, len;

	for(i=0, first=-1; i<numlumps; i++)
	{
		if(first < 0 && lumpinfo[i].handle == handle)
		{
			// Start a region.
			first = i;
			continue;
		}
		// Does a region end?
		if(first >= 0)
			if(lumpinfo[i].handle != handle || i==numlumps-1 ||
				MarkerForGroup(lumpinfo[i].name, true) ||
				MarkerForGroup(lumpinfo[i].name, false))
			{
				if(lumpinfo[i].handle == handle && i==numlumps-1) 
					i++; // Also free the last one.
				// The length of the region.
				len = i-first;
				// Free the memory allocated for the region.
				for(k=first; k<i; k++) 
				{
				//	ST_Message("removing lump: %s (%d)\n", lumpinfo[k].name, lumpinfo[k].handle);
					if(lumpcache[k]) 
					{
						
						// If the block has a user, it must be explicitly freed.
						/*if((unsigned int)Z_GetUser(lumpcache[k]) > 0x100) 
							Z_Free(lumpcache[k]);
						else if(Z_GetTag(lumpcache[k]) < PU_LEVEL)
							Z_ChangeTag(lumpcache[k], PU_LEVEL);*/
						//Z_ChangeTag(lumpcache[k], PU_CACHE);
						//Z_ChangeUser(lumpcache[k], NULL);
						if(Z_GetTag(lumpcache[k]) < PU_LEVEL)
							Z_ChangeTag(lumpcache[k], PU_LEVEL);
						// Mark the memory pointer in use, but unowned.
						Z_ChangeUser(lumpcache[k], (void*) 0x2);
					}
				}
				// Move the data in the lump storage.
				W_MoveLumps(i, numlumps-i, -len);
				numlumps -= len;
				i -= len;
				// Make it possible to begin a new region.
				first = -1;
			}
	}
}

// Reallocates lumpinfo and lumpcache. 
void W_ResizeLumpStorage(int numitems)
{
	lumpinfo = (lumpinfo_t *)realloc(lumpinfo, sizeof(lumpinfo_t) * numitems);
	// -JL- Paranoia
	if (!lumpinfo)
		I_Error("W_ResizeLumpStorage: realloc failed");

	// Updating the cache is a bit more difficult. We need to make sure
	// the user pointers in the memory zone remain valid.
	if(numcache != numitems)
	{
		void **newcache;	// This is the new cache.	
		int	i, newCacheBytes = numitems * sizeof(*newcache);// The new size of the cache (bytes).
		int numToMod;
		newcache = (void **)malloc(newCacheBytes);
		// -JL- Paranoia
		if (!newcache)
			I_Error("W_ResizeLumpStorage: malloc failed");
		memset(newcache, 0, newCacheBytes);		// Clear the new cache.
		// Copy the old cache.
		if(numcache < numitems)
			numToMod = numcache;
		else 
			numToMod = numitems;
		memcpy(newcache, lumpcache, numToMod*sizeof(*lumpcache));
		// Update the user information in the memory zone.
		for(i=0; i<numToMod; i++)
			if(newcache[i])
				Z_ChangeUser(newcache[i], newcache+i);
/*
		// Check that the rest of the cache has been freed (the part that gets
		// removed, if the storage is getting smaller).
		for(i=numToMod; i<numcache; i++)
			if(lumpcache[i])
				I_Error("W_ResizeLumpStorage: Cached lump getting lost.\n");
*/
		// Get rid of the old cache.
		free(lumpcache);
		lumpcache = newcache;
		numcache = numitems;
	}
}

// Returns one of the grouping tags.
int MarkerForGroup(char *name, boolean begin)
{
	int		i;

	for(i=1; i<NUM_GT; i++)
		if(!strnicmp(name, begin? groups[i].start : groups[i].end, 8) || 
			!strnicmp(name+1, begin? groups[i].start : groups[i].end, 7))
			return i;

	// No matches...
	return GT_NONE;
}

//==========================================================================
//
// W_InsertLumps
//
// Inserts the lumps in the fileinfo/record to their correct places in
// the lumpinfo. Also maintains lumpinfo/records that all data is valid.
//
// The current placing of the lumps is that flats and sprites are added
// to previously existing flat and sprite groups. All other lumps are
// just appended to the end of the list.
//
//==========================================================================

void W_InsertLumps(filelump_t *fileinfo, filerecord_t *rec)
{
	int			i, to, num;
	filelump_t	*flump = fileinfo;
	int			inside = GT_NONE;	// Not inside any group.
	int			groupFirst = 0;		// First lump in the current group.
	int			maxNumLumps = numlumps + rec->numlumps; // This must be enough.

	// Allocate more memory for the lumpinfo.
	W_ResizeLumpStorage(maxNumLumps);

	for(i=0; i<rec->numlumps; i++, flump++)
	{
		if(inside == GT_NONE)
		{
			// We are currently not inside any group.
			if((inside=MarkerForGroup(flump->name, true)) != GT_NONE)
			{
				// We have entered a group! Go to the next lump.
				groupFirst = i+1;
				continue; 
			}
			// This lump is very ordinary. Just append it to the
			// lumpinfo.
			//rec->indices[i] = numlumps;
			W_FillLumpInfo(numlumps++, flump, rec);
		}
		else 
		{
			if(MarkerForGroup(flump->name, false) == inside) // Our group ends?
			{
				// This is how many lumps we'll add.
				num = i-groupFirst;

				// Find the existing group.
				to = W_ScanForName(groups[inside].end, 0);
				if(to < 0)
				{
					// There is no existing group. Include the start
					// and end markers in the range of lumps to add.
					groupFirst--;
					num += 2;
					to = numlumps;
				}
				W_InsertAndFillLumpRange(to, fileinfo+groupFirst, num, rec);

				// We exit this group.
				inside = GT_NONE;
			}
		}
	}

	// It may be that all lumps weren't added. Make sure we don't have
	// excess memory allocated (=synchronize the storage size with the 
	// real numlumps).
	W_ResizeLumpStorage(numlumps);

	// Update the record with the number of lumps that were loaded.
	rec->numlumps -= maxNumLumps - numlumps;
}


//==========================================================================
//
// W_AddFile
//
// Files with a .wad extension are wadlink files with multiple lumps,
// other files are single lumps with the base filename for the lump name.
//
// Returns true if the operation is successful.
//
//==========================================================================

boolean W_AddFile(char *filename)
{
	wadinfo_t		header;
	int				handle, length;
	filelump_t		*fileinfo, singleinfo;
	filelump_t		*freeFileInfo;
	filerecord_t	*rec;

	if((handle = open(filename, O_RDONLY|O_BINARY)) == -1)
	{ // Didn't find file
		ST_Message( "W_AddFile: '%s' not found.\n", filename);
		return false;
	}
	// Get a new file record.
	rec = W_RecordNew();
	strcpy(rec->filename, filename);
	convertSlashes(rec->filename);
	rec->handle = handle;

	// If we're not loading for startup, flag the record to be a Runtime one.
	if(!loadingForStartup) rec->flags = FRF_RUNTIME;

	if(stricmp(filename+strlen(filename)-3, "wad") 
		&& stricmp(filename+strlen(filename)-3, "gwa"))
	{ // Single lump file
		fileinfo = &singleinfo;
		freeFileInfo = NULL;
		singleinfo.filepos = 0;
		singleinfo.size = LONG(filelength(handle));
		M_ExtractFileBase(filename, singleinfo.name);
		rec->numlumps = 1;
	}
	else
	{ // WAD file
		read(handle, &header, sizeof(header));
		if(strncmp(header.identification, "IWAD", 4))
		{
			if(strncmp(header.identification, "PWAD", 4))
			{ // Bad file id
				I_Error("Wad file %s doesn't have IWAD or PWAD id\n",
					filename);
			}
		}
		header.numlumps = LONG(header.numlumps);
		header.infotableofs = LONG(header.infotableofs);
		length = header.numlumps*sizeof(filelump_t);
		if(!(fileinfo = (filelump_t *)malloc(length)))
		{
			I_Error("W_AddFile:  fileinfo malloc failed\n");
		}
		freeFileInfo = fileinfo;
		lseek(handle, header.infotableofs, SEEK_SET);
		read(handle, fileinfo, length);
		rec->numlumps = header.numlumps;
	}

	// Insert the lumps to lumpinfo, into their rightful places.
	W_InsertLumps(fileinfo, rec);

	if(freeFileInfo) free(freeFileInfo);

	PrimaryLumpInfo = lumpinfo;
	PrimaryLumpCache = lumpcache;
	PrimaryNumLumps = numlumps;

	// glBSP: Also load a possible GWA.
	if(!stricmp(filename + strlen(filename)-3, "wad"))
	{
		char buff[256];
		strcpy(buff, filename);
		strcpy(buff + strlen(buff)-3, "gwa");
		W_AddFile(buff);
	}	

	return true;
}

//==========================================================================
//
// W_RemoveFile
//
//==========================================================================

boolean W_RemoveFile(char *filename)
{
	int				idx = W_RecordGetIdx(filename), oldnum = numlumps;
	filerecord_t	*rec;

	if(idx == -1) return false;	// No such file loaded.
	rec = records + idx;

	// We must remove all the data of this file from the lump storage
	// (lumpinfo + lumpcache).
	W_RemoveLumpsWithHandle(rec->handle); 

	// Resize the lump storage to match numlumps.
	W_ResizeLumpStorage(numlumps);

	// Close the file, we don't need it any more.
	close(rec->handle);

	// Destroy the file record.
	W_RecordDestroy(idx);

	PrimaryLumpInfo = lumpinfo;
	PrimaryLumpCache = lumpcache;
	PrimaryNumLumps = numlumps;

	// Success!
	return true;
}

// Remove all records flagged Runtime.
void W_Reset()
{
	int		i;
	
	for(i=0; i<numrecords; i++)
		if(records[i].flags & FRF_RUNTIME)
			W_RemoveFile(records[i].filename);
}


//==========================================================================
//
// W_InitMultipleFiles
//
// Pass a null terminated list of files to use.  All files are optional,
// but at least one file must be found.  Lump names can appear multiple
// times.  The name searcher looks backwards, so a later file can
// override an earlier one.
//
//==========================================================================

void W_InitMultipleFiles(char **filenames)
{
	// Open all the files, load headers, and count lumps
	numlumps = 0;
	lumpinfo = (lumpinfo_t *)malloc(1); // Will be realloced as lumps are added

	// This'll force the loader NOT the flag new records Runtime.
	loadingForStartup = true;

	for(; *filenames; filenames++)
	{
		W_AddFile(*filenames);
	}

	// Startup loading ends.
	loadingForStartup = false;

	if(!numlumps)
	{
		I_Error("W_InitMultipleFiles: no files found");
	}
}




//==========================================================================
//
// W_UpdateCache
//
// Reallocate the lump cache so that it has the right amount of memory.
// 
//==========================================================================

void W_UpdateCache()
{
/*	unsigned int i, numcopy;
	int size;
	void **newcache;

	size = numlumps * sizeof(*lumpcache);
	newcache = malloc(size);
	memset(newcache, 0, size);
	// The cache is a list of pointers to the memory zone. We must
	// update the zone user pointers.
	numcopy = cachesize/sizeof(*lumpcache); // Copy this many.
	// If the new cache is smaller, don't copy too much.
	if((unsigned)numlumps < numcopy) 
	{
//		for(i=numlumps; i<numcopy; i++)
//			if(lumpcache[i])
//			{
//				
//				// The blocks aren't used by anyone any more.
//				Z_Free(lumpcache[i]);
//			}
//		numcopy = numlumps;
		I_Error("W_UpdateCache: Trying to copy more lumps than there are.\n");
	}
	for(i=0; i<numcopy; i++)
		if(lumpcache[i])
			Z_ChangeUser(lumpcache[i], newcache+i);
	// Copy the old cache contents.
	memcpy(newcache, lumpcache, numcopy*sizeof(*lumpcache));

	// Get rid of the old cache.
	free(lumpcache);
	lumpcache = newcache;
	cachesize = size;

	PrimaryLumpInfo = lumpinfo;
	PrimaryLumpCache = lumpcache;
	PrimaryNumLumps = numlumps;*/
}

//==========================================================================
//
// W_InitFile
//
// Initialize the primary from a single file.
//
//==========================================================================

void W_InitFile(char *filename)
{
	char *names[2];

	names[0] = filename;
	names[1] = NULL;
	W_InitMultipleFiles(names);
}

//==========================================================================
//
// W_OpenAuxiliary
//
//==========================================================================

void W_OpenAuxiliary(char *filename)
{
	int i;
	int size;
	wadinfo_t header;
	int handle;
	int length;
	filelump_t *fileinfo;
	filelump_t *sourceLump;
	lumpinfo_t *destLump;

	if(AuxiliaryOpened)
	{
		W_CloseAuxiliary();
	}
	if((handle = open(filename, O_RDONLY|O_BINARY)) == -1)
	{
		I_Error("W_OpenAuxiliary: %s not found.", filename);
		return;
	}
	AuxiliaryHandle = handle;
	read(handle, &header, sizeof(header));
	if(strncmp(header.identification, "IWAD", 4))
	{
		if(strncmp(header.identification, "PWAD", 4))
		{ // Bad file id
			I_Error("Wad file %s doesn't have IWAD or PWAD id\n",
				filename);
		}
	}
	header.numlumps = LONG(header.numlumps);
	header.infotableofs = LONG(header.infotableofs);
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = (filelump_t *)Z_Malloc(length, PU_STATIC, 0);
	lseek(handle, header.infotableofs, SEEK_SET);
	read(handle, fileinfo, length);
	numlumps = header.numlumps;

	// Init the auxiliary lumpinfo array
	lumpinfo = (lumpinfo_t *)Z_Malloc(numlumps*sizeof(lumpinfo_t), PU_STATIC, 0);
	sourceLump = fileinfo;
	destLump = lumpinfo;
	for(i = 0; i < numlumps; i++, destLump++, sourceLump++)
	{
		destLump->handle = handle;
		destLump->position = LONG(sourceLump->filepos);
		destLump->size = LONG(sourceLump->size);
		strncpy(destLump->name, sourceLump->name, 8);
	}
	Z_Free(fileinfo);

	// Allocate the auxiliary lumpcache array
	size = numlumps*sizeof(*lumpcache);
	lumpcache = (void **)Z_Malloc(size, PU_STATIC, 0);
	memset(lumpcache, 0, size);

	AuxiliaryLumpInfo = lumpinfo;
	AuxiliaryLumpCache = lumpcache;
	AuxiliaryNumLumps = numlumps;
	AuxiliaryOpened = true;
}

//==========================================================================
//
// W_CloseAuxiliary
//
//==========================================================================

void W_CloseAuxiliary(void)
{
	int i;

	if(AuxiliaryOpened)
	{
		W_UseAuxiliary();
		for(i = 0; i < numlumps; i++)
		{
			if(lumpcache[i])
			{
				Z_Free(lumpcache[i]);
			}
		}
		Z_Free(AuxiliaryLumpInfo);
		Z_Free(AuxiliaryLumpCache);
		W_CloseAuxiliaryFile();
		AuxiliaryOpened = false;
	}
	W_UsePrimary();
}

//==========================================================================
//
// W_CloseAuxiliaryFile
//
// WARNING: W_CloseAuxiliary() must be called before any further
// auxiliary lump processing.
//
//==========================================================================

void W_CloseAuxiliaryFile(void)
{
	if(AuxiliaryHandle)
	{
		close(AuxiliaryHandle);
		AuxiliaryHandle = 0;
	}
}

//==========================================================================
//
// W_UsePrimary
//
//==========================================================================

void W_UsePrimary(void)
{
	lumpinfo = PrimaryLumpInfo;
	numlumps = PrimaryNumLumps;
	lumpcache = PrimaryLumpCache;
}

//==========================================================================
//
// W_UseAuxiliary
//
//==========================================================================

void W_UseAuxiliary(void)
{
	if(AuxiliaryOpened == false)
	{
		I_Error("W_UseAuxiliary: WAD not opened.");
	}
	lumpinfo = AuxiliaryLumpInfo;
	numlumps = AuxiliaryNumLumps;
	lumpcache = AuxiliaryLumpCache;
}

//==========================================================================
//
// W_NumLumps
//
//==========================================================================

int	W_NumLumps(void)
{
	return numlumps;
}

//==========================================================================
//
// W_CheckNumForName
//
// Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForName(char *name)
{
	char name8[9];
	int v1, v2;
	lumpinfo_t *lump_p;

	// Make the name into two integers for easy compares
	strncpy(name8, name, 8);
	name8[8] = 0; // in case the name was a full 8 chars
	strupr(name8); // case insensitive
	v1 = *(int *)name8;
	v2 = *(int *)&name8[4];

	// Scan backwards so patch lump files take precedence
	lump_p = lumpinfo+numlumps;
	while(lump_p-- != lumpinfo)
	{
		if(*(int *)lump_p->name == v1 && *(int *)&lump_p->name[4] == v2)
		{
			return lump_p-lumpinfo;
		}
	}
	return -1;
}

//==========================================================================
//
// W_GetNumForName
//
// Calls W_CheckNumForName, but bombs out if not found.
//
//==========================================================================

int	W_GetNumForName (char *name)
{
	int	i;

	i = W_CheckNumForName(name);
	if(i != -1)
	{
		return i;
	}
	I_Error("W_GetNumForName: %s not found!", name);
	return -1;
}

//==========================================================================
//
// W_LumpLength
//
// Returns the buffer size needed to load the given lump.
//
//==========================================================================

int W_LumpLength(int lump)
{
	if(lump >= numlumps)
	{
		I_Error("W_LumpLength: %i >= numlumps", lump);
	}
	return lumpinfo[lump].size;
}

//==========================================================================
//
// W_ReadLump
//
// Loads the lump into the given buffer, which must be >= W_LumpLength().
//
//==========================================================================

void W_ReadLump(int lump, void *dest)
{
	int c;
	lumpinfo_t *l;

	if(lump >= numlumps)
	{
		I_Error("W_ReadLump: %i >= numlumps", lump);
	}
	l = lumpinfo+lump;
	//I_BeginRead();
	lseek(l->handle, l->position, SEEK_SET);
	c = read(l->handle, dest, l->size);
	if(c < l->size)
	{
		I_Error("W_ReadLump: only read %i of %i on lump %i",
			c, l->size, lump);
	}
	//I_EndRead();
}

//==========================================================================
//
// W_CacheLumpNum
//
//==========================================================================

void *W_CacheLumpNum(int lump, int tag)
{
	byte *ptr;

	if((unsigned)lump >= (unsigned)numlumps)
	{
		I_Error("W_CacheLumpNum: %i >= numlumps", lump);
	}
	if(!lumpcache[lump])
	{ // Need to read the lump in
		ptr = (byte *)Z_Malloc(W_LumpLength(lump), tag, &lumpcache[lump]);
		W_ReadLump(lump, lumpcache[lump]);
	}
	else
	{
		Z_ChangeTag(lumpcache[lump], tag);
	}
	return lumpcache[lump];
}

//==========================================================================
//
// W_CacheLumpName
//
//==========================================================================

void *W_CacheLumpName(char *name, int tag)
{
	return W_CacheLumpNum(W_GetNumForName(name), tag);
}

//==========================================================================
//
// W_Profile
//
//==========================================================================

// Ripped out for Heretic
/*
int	info[2500][10];
int	profilecount;

void W_Profile (void)
{
	int		i;
	memblock_t	*block;
	void	*ptr;
	char	ch;
	FILE	*f;
	int		j;
	char	name[9];
	
	
	for (i=0 ; i<numlumps ; i++)
	{	
		ptr = lumpcache[i];
		if (!ptr)
		{
			ch = ' ';
			continue;
		}
		else
		{
			block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
			if (block->tag < PU_PURGELEVEL)
				ch = 'S';
			else
				ch = 'P';
		}
		info[i][profilecount] = ch;
	}
	profilecount++;
	
	f = fopen ("waddump.txt","w");
	name[8] = 0;
	for (i=0 ; i<numlumps ; i++)
	{
		memcpy (name,lumpinfo[i].name,8);
		for (j=0 ; j<8 ; j++)
			if (!name[j])
				break;
		for ( ; j<8 ; j++)
			name[j] = ' ';
		fprintf (f,"%s ",name);
		for (j=0 ; j<profilecount ; j++)
			fprintf (f,"    %c",info[i][j]);
		fprintf (f,"\n");
	}
	fclose (f);
}
*/


void W_ChangeCacheTag(int lump, int tag)
{
	if(lumpcache[lump])
		if(((memblock_t *)((byte*)
			(lumpcache[lump])-sizeof(memblock_t)))->id == 0x1d4a11)
		{
			Z_ChangeTag(lumpcache[lump], tag);
		}
}
