// Memory zone routines.

#ifndef __DOOMSDAY_ZONE_H__
#define __DOOMSDAY_ZONE_H__

// tags < 100 are not overwritten until freed
#define	PU_STATIC		1			// static entire execution time
#define	PU_SOUND		2			// static while playing
#define	PU_MUSIC		3			// static while playing
#define	PU_DAVE			4			// anything else Dave wants static

#define PU_OPENGL		10			// OpenGL allocated stuff. -jk
#define PU_REFRESHTEX	11			// Textures/refresh.
#define PU_REFRESHCM	12			// Colormap.
#define PU_REFRESHTRANS	13
#define PU_REFRESHSPR	14
#define PU_SPRITE		20

#define	PU_LEVEL		50			// static until level exited
#define	PU_LEVSPEC		51			// a special thinker in a level
// tags >= 100 are purgable whenever needed
#define	PU_PURGELEVEL	100
#define	PU_CACHE		101


void	Z_Init (void);
void 	*Z_Malloc (int size, int tag, void *ptr);
void 	Z_Free (void *ptr);
void 	Z_FreeTags (int lowtag, int hightag);
//void 	Z_DumpHeap (int lowtag, int hightag);
//void	Z_FileDumpHeap (FILE *f);
void	Z_CheckHeap (void);
void	Z_ChangeTag2 (void *ptr, int tag);
void	Z_ChangeUser(void *ptr, void *newuser); //-jk
//int 	Z_FreeMemory (void);
void	*Z_GetUser(void *ptr);
int		Z_GetTag(void *ptr);


typedef struct memblock_s
{
	int 				size;			// including the header and possibly tiny fragments
	void				**user; 		// NULL if a free block
	int 				tag;			// purgelevel
	int 				id; 			// should be ZONEID
	struct memblock_s	*next, *prev;
} memblock_t;

typedef struct
{
	int			size;		// total bytes malloced, including header
	memblock_t	blocklist;	// start / end cap for linked list
	memblock_t	*rover;
} memzone_t;


#define Z_ChangeTag(p,t) \
{ \
if (( (memblock_t *)( (byte *)(p) - sizeof(memblock_t)))->id!=0x1d4a11) \
	I_Error("Z_CT at "__FILE__":%i",__LINE__); \
Z_ChangeTag2(p,t); \
};


#endif