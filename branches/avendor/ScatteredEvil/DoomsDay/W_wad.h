#ifndef __JHEXEN_WAD_H__
#define __JHEXEN_WAD_H__

#define RECORD_FILENAMELEN	200

// File record flags.
#define	FRF_RUNTIME		0x1		// Loaded at runtime (for reset).

typedef struct
{
	char	filename[RECORD_FILENAMELEN];	// Full filename (every '\' -> '/').
	int		numlumps;		// Number of lumps.
	int		flags;			
	int		handle;			// File handle.
} filerecord_t;

typedef struct
{
	char		name[8];
	int			handle, position, size;
} lumpinfo_t;

extern lumpinfo_t *lumpinfo;
extern int numlumps;

void W_InitMultipleFiles(char **filenames);
void W_OpenAuxiliary(char *filename);
void W_CloseAuxiliaryFile(void);
void W_CloseAuxiliary(void);
void W_UsePrimary(void);
void W_UseAuxiliary(void);
int W_CheckNumForName(char *name);
int W_GetNumForName(char *name);
int W_LumpLength(int lump);
void W_ReadLump(int lump, void *dest);
void *W_CacheLumpNum(int lump, int tag);
void *W_CacheLumpName(char *name, int tag);
boolean W_AddFile(char *filename);
boolean W_RemoveFile(char *filename);
void W_Reset();
void W_ChangeCacheTag(int lump, int tag);

#endif