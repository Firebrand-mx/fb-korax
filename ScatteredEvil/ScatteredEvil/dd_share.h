// Common stuff to both the main engine and games.

#ifndef __DOOMSDAY_SHARED_H__
#define __DOOMSDAY_SHARED_H__

#include <stdlib.h>

//------------------------------------------------------------------------
//
// General Definitions and Macros
//
//------------------------------------------------------------------------

#define strcasecmp stricmp
#define strncasecmp strnicmp

typedef unsigned int uint;

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
#ifdef __cplusplus
typedef int ddboolean;
#else
typedef enum {false, true} ddboolean;
#endif
typedef unsigned char byte;
#define boolean ddboolean
#endif

#ifdef __BIG_ENDIAN__
short ShortSwap(short);
long LongSwap(long);
#define SHORT(x)	ShortSwap(x)
#define LONG(x)		LongSwap(x)
#else
#define SHORT(x)	(x)
#define LONG(x)		(x)
#endif

#define DDMAXCHAR	((char)0x7f)
#define DDMAXSHORT	((short)0x7fff)
#define DDMAXINT	((int)0x7fffffff)	/* max pos 32-bit int */
#define DDMAXLONG	((long)0x7fffffff)

#define DDMINCHAR	((char)0x80)
#define DDMINSHORT	((short)0x8000)
#define DDMININT 	((int)0x80000000)	/* max negative 32-bit integer */
#define DDMINLONG	((long)0x80000000)

enum 
{
	// TexFilterMode targets
	DD_TEXTURES = 0,
	DD_RAWSCREENS,

	// Filter/mipmap modes
	DD_NEAREST = 0,
	DD_LINEAR,
	DD_NEAREST_MIPMAP_NEAREST,
	DD_LINEAR_MIPMAP_NEAREST,
	DD_NEAREST_MIPMAP_LINEAR,
	DD_LINEAR_MIPMAP_LINEAR,

	// Music devices
	DD_MUSIC_NONE = 0,
	DD_MUSIC_MIDI,
	DD_MUSIC_CDROM,

	// CD-ROM control
	DD_INIT = 0,
	DD_AVAILABLE,
	DD_PLAY,
	DD_PLAY_LOOP,
	DD_STOP,
	DD_RESUME,
	DD_SET_VOLUME,
	DD_GET_VOLUME,
	DD_GET_FIRST_TRACK,
	DD_GET_LAST_TRACK,
	DD_GET_TRACK_LENGTH,
	DD_GET_CURRENT_TRACK,
	DD_GET_TIME_LEFT,
	DD_STATUS,
	DD_NOT_READY,
	DD_STOPPED,		
	DD_PLAYING,
	DD_PAUSED,	

	// General constants
	DD_SKY = -1,
	DD_DISABLE,
	DD_ENABLE,
	DD_MASK,
	DD_YES,
	DD_NO,
	DD_TEXTURE,
	DD_OFFSET,
	DD_HEIGHT,
	DD_COLUMNS,
	DD_ROWS,
	DD_COLOR_LIMIT,
	DD_PRE,
	DD_POST,
	DD_VERSION_SHORT,
	DD_VERSION_LONG,
	DD_HORIZON,
};


//------------------------------------------------------------------------
//
// Fixed-Point Math
//
//------------------------------------------------------------------------

typedef int fixed_t;
typedef unsigned angle_t;

#define	FRACBITS			16
#define	FRACUNIT			(1<<FRACBITS)

#define	FINEANGLES			8192
#define	FINEMASK			(FINEANGLES-1)
#define	ANGLETOFINESHIFT	19	// 0x100000000 to 0x2000

#define ANGLE_45			0x20000000
#define ANGLE_90			0x40000000
#define ANGLE_180			0x80000000
#define ANGLE_MAX			0xffffffff
#define ANGLE_1				(ANGLE_45/45)
#define ANGLE_60			(ANGLE_180/3)

#define	ANG45				0x20000000
#define	ANG90				0x40000000
#define	ANG180				0x80000000
#define	ANG270				0xc0000000

#define FIX2FLT(x)		( (x) / (float) FRACUNIT )
#define Q_FIX2FLT(x)	( (float)((x)>>FRACBITS) )

#pragma warning(push)
#pragma warning(disable : 4035)

__inline fixed_t FixedMul (fixed_t a, fixed_t b)
{
	__asm {
		// The parameters in eax and ebx.
		mov		eax, a
		mov		ebx, b
		// The multiplying.
		imul	ebx
		shrd	eax, edx, 16
		// eax should hold the return value.
	}
}

__inline fixed_t FixedDiv2 (fixed_t a, fixed_t b)
{
	__asm {
		// The parameters.
		mov		eax, a
		mov		ebx, b
		// The operation.
		cdq
		shld	edx, eax, 16
		sal		eax, 16
		idiv	ebx
		// And the value returns in eax.
	}
}

__inline fixed_t FixedDiv (fixed_t a, fixed_t b)
{
	if((abs(a)>>14) >= abs(b))
	{
		return((a^b)<0 ? DDMININT : DDMAXINT);
	}
	return(FixedDiv2(a, b));
}

#pragma warning(pop)


//------------------------------------------------------------------------
//
// Key Codes
//
//------------------------------------------------------------------------

//
// most key data are simple ascii (uppercased)
//
#define	DDKEY_RIGHTARROW		0xae
#define	DDKEY_LEFTARROW			0xac
#define	DDKEY_UPARROW			0xad
#define	DDKEY_DOWNARROW			0xaf
#define	DDKEY_ESCAPE			27
#define	DDKEY_ENTER				13
#define DDKEY_TAB				9
#define	DDKEY_F1				(0x80+0x3b)
#define	DDKEY_F2				(0x80+0x3c)
#define	DDKEY_F3				(0x80+0x3d)
#define	DDKEY_F4				(0x80+0x3e)
#define	DDKEY_F5				(0x80+0x3f)
#define	DDKEY_F6				(0x80+0x40)
#define	DDKEY_F7				(0x80+0x41)
#define	DDKEY_F8				(0x80+0x42)
#define	DDKEY_F9				(0x80+0x43)
#define	DDKEY_F10				(0x80+0x44)
#define	DDKEY_F11				(0x80+0x57)
#define	DDKEY_F12				(0x80+0x58)

#define DDKEY_NUMLOCK			0xf0
#define DDKEY_SCROLL			0xf1    /* Scroll Lock */
#define DDKEY_NUMPAD7			0xf2
#define DDKEY_NUMPAD8			0xf3
#define DDKEY_NUMPAD9			0xf4
#define DDKEY_SUBTRACT			0xf5    /* - on numeric keypad */
#define DDKEY_NUMPAD4			0xf6
#define DDKEY_NUMPAD5			0xf7
#define DDKEY_NUMPAD6			0xf8
#define DDKEY_ADD				0xf9    /* + on numeric keypad */
#define DDKEY_NUMPAD1			0xfa
#define DDKEY_NUMPAD2			0xfb
#define DDKEY_NUMPAD3			0xfc
#define DDKEY_NUMPAD0			0xfd
#define DDKEY_DECIMAL			0xfe

#define	DDKEY_BACKSPACE			127
#define	DDKEY_PAUSE				0xff

#define DDKEY_EQUALS			0x3d
#define DDKEY_MINUS				0x2d

#define	DDKEY_RSHIFT			(0x80+0x36)
#define	DDKEY_RCTRL				(0x80+0x1d)
#define	DDKEY_RALT				(0x80+0x38)

#define	DDKEY_LALT				DDKEY_RALT

#define DDKEY_FIVE				0x35
#define DDKEY_SIX				0x36
#define DDKEY_SEVEN				0x37
#define DDKEY_EIGHT				0x38
#define DDKEY_NINE				0x39
#define DDKEY_ZERO				0x30
#define DDKEY_BACKSLASH			0x5C

#define DDKEY_LSHIFT			0xfe

#define DDKEY_INS				(0x80+0x52)
#define DDKEY_DEL				(0x80+0x53)
#define DDKEY_PGUP				(0x80+0x49)
#define DDKEY_PGDN				(0x80+0x51)
#define DDKEY_HOME				(0x80+0x47)
#define DDKEY_END				(0x80+0x4f)


//------------------------------------------------------------------------
//
// Events
//
//------------------------------------------------------------------------

typedef enum
{
	ev_keydown,
	ev_keyup,
	ev_keyrepeat,
	ev_mouse,
	ev_mousebdown,
	ev_mousebup,
	ev_joystick,
	ev_joybdown,
	ev_joybup,
	ev_povdown,
	ev_povup
} evtype_t;

typedef struct
{
	evtype_t	type;
	int			data1;		// keys/mouse/joystick buttons
	int			data2;		// mouse/joystick x move
	int			data3;		// mouse/joystick y move
} event_t;

// The mouse wheel is considered two extra mouse buttons.
#define DDMB_MWHEELUP		0x10
#define DDMB_MWHEELDOWN		0x20


//------------------------------------------------------------------------
//
// Map Data
//
//------------------------------------------------------------------------

enum // Sector reverb data indices.
{
	SRD_VOLUME,
	SRD_SPACE,
	SRD_DECAY,
	SRD_DAMPING,
	NUM_REVERB_DATA
};


#define DDSEGF_DLIGHT		0x1	

// Dlights are hitting the floor/ceiling of this subsector.
#define DDSUBF_DLIGHT_FLOOR		0x1
#define DDSUBF_DLIGHT_CEILING	0x2


// For PathTraverse.
#define PT_ADDLINES		1
#define	PT_ADDTHINGS	2
#define	PT_EARLYOUT		4

// Mapblocks are used to check movement against lines and things.
#define MAPBLOCKUNITS	128
#define	MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define	MAPBLOCKSHIFT	(FRACBITS+7)
#define	MAPBMASK		(MAPBLOCKSIZE-1)
#define	MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)

/*typedef enum 
{
	ST_HORIZONTAL, ST_VERTICAL, ST_POSITIVE, ST_NEGATIVE
} slopetype_t;*/

// For (un)linking.
#define DDLINK_SECTOR		0x1
#define DDLINK_BLOCKMAP		0x2

struct mobj_t;
struct line_t;


//------------------------------------------------------------------------
//
// Mobjs
//
//------------------------------------------------------------------------

// Frame flags.
#define	FF_FULLBRIGHT	0x8000		// flag in thing->frame
#define FF_FRAMEMASK	0x7fff

// Doomsday mobj flags.
#define DDMF_DONTDRAW		0x00000001
#define DDMF_SHADOW			0x00000002
#define DDMF_ALTSHADOW		0x00000004
#define DDMF_BRIGHTSHADOW	0x00000008
#define DDMF_VIEWALIGN		0x00000010
#define DDMF_FITTOP			0x00000020	// Don't let the sprite go into the ceiling.
#define DDMF_NOFITBOTTOM	0x00000040
#define DDMF_LIGHTSCALE		0x00000180	// Light scale (0: full, 3: 1/4).
#define DDMF_LIGHTOFFSET	0x0000f000	// How to offset light (along Z axis)
#define DDMF_RESERVED		0x00030000	// don't touch these!! (translation class)
#define	DDMF_TRANSLATION	0x1c000000	// use a translation table (>>MF_TRANSHIFT)

#define	DDMF_TRANSSHIFT			26			// table for player colormaps
#define DDMF_CLASSTRSHIFT		16
#define DDMF_LIGHTSCALESHIFT	7
#define DDMF_LIGHTOFFSETSHIFT	12


//------------------------------------------------------------------------
//
// Refresh
//
//------------------------------------------------------------------------

#define TICRATE			35			// number of tics / second
#define TICSPERSEC		35

#define SCREENWIDTH		640
#define SCREENHEIGHT	480

#define I_NOUPDATE	0
#define I_FULLVIEW	1
#define I_STATBAR	2
#define I_MESSAGES	4
#define I_FULLSCRN	8

// Update flags.
#define DDUF_BORDER		0x1		// BorderNeedRefresh = true
#define DDUF_TOP		0x2		// BorderTopRefresh = true
#define DDUF_FULLVIEW	0x10	// Request update for the view.
#define DDUF_STATBAR	0x20	// Request update for the status bar.
#define DDUF_MESSAGES	0x40	// Request update for the messages.
#define DDUF_FULLSCREEN	0x80	// Request update for the whole screen.
#define DDUF_UPDATE		0x10000	// Really calls I_Update.


//------------------------------------------------------------------------
//
// Sound
//
//------------------------------------------------------------------------

// Which fields are valid?
#define DDSOUNDF_VOLUME			0x1
#define DDSOUNDF_PITCH			0x2
#define DDSOUNDF_POS			0x4
#define DDSOUNDF_MOV			0x8

#define DDSOUNDF_LOCAL			0x10000	// The sound is inside the listener's head.
#define DDSOUNDF_VERY_LOUD		0x20000 // The sound has virtually no rolloff.

typedef struct
{
	int			flags;
	int			volume;			// 0..1000
	int			pitch;			// 1000 is normal.
	fixed_t		pos[3];
	fixed_t		mov[3];
} sound3d_t;

typedef struct
{
	float		volume;		// 0..1
	float		decay;		// Decay factor: 0 (acoustically dead) ... 1 (live)
	float		damping;	// High frequency damping factor: 0..1
	float		space;		// 0 (small space) ... 1 (large space)
} reverb_t;

// Which fields are valid?
#define DDLISTENERF_POS				0x1
#define DDLISTENERF_MOV				0x2
#define DDLISTENERF_YAW				0x4
#define DDLISTENERF_PITCH			0x8
// Reverb control:
#define DDLISTENERF_SET_REVERB		0x10
#define DDLISTENERF_DISABLE_REVERB	0x20

typedef struct
{
	int			flags;
	fixed_t		pos[3];
	fixed_t		mov[3];
	float		yaw, pitch;		// In degrees: (0,0) is to the east.
	reverb_t	reverb;
} listener3d_t;


//------------------------------------------------------------------------
//
// Graphics
//
//------------------------------------------------------------------------

// posts are runs of non masked source pixels
struct post_t
{
	byte		topdelta;		// -1 is the last post in a column
	byte		length;
// length data bytes follows
};

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t	column_t;

// a patch holds one or more columns
// patches are used for sprites and all masked pictures
struct patch_t
{
	short		width;				// bounding box size
	short		height;
	short		leftoffset;			// pixels to the left of origin
	short		topoffset;			// pixels below the origin
	int			columnofs[8];		// only [width] used
									// the [0] is &columnofs[width]
};

// a pic is an unmasked block of pixels
struct pic_t
{
	byte		width,height;
	byte		data;
};

struct spriteinfo_t
{
	int	lump;
	int	flip;
	int offset;
	int topOffset;
};


//------------------------------------------------------------------------
//
// Texture definition
//
//------------------------------------------------------------------------

struct mappatch_t
{
	short	originx;
	short	originy;
	short	patch;
	short	stepdir;
	short	colormap;
};

struct maptexture_t
{
	char		name[8];
	boolean		masked;	
	short		width;
	short		height;
	void		**columndirectory;	// OBSOLETE
	short		patchcount;
	mappatch_t	patches[1];
};


//------------------------------------------------------------------------
//
// Console
//
//------------------------------------------------------------------------

// These correspond the good old text mode VGA colors.
#define CBLF_BLACK		0x00000001
#define CBLF_BLUE		0x00000002
#define CBLF_GREEN		0x00000004
#define CBLF_CYAN		0x00000008	
#define CBLF_RED		0x00000010
#define CBLF_MAGENTA	0x00000020
#define CBLF_YELLOW		0x00000040
#define CBLF_WHITE		0x00000080	
#define CBLF_LIGHT		0x00000100

// Console command.
struct ccmd_t
{
	char		*name;
	int			(*func)(int argc, char **argv);
	char		*help;		// A short help text.
};

// Console variable flags.
#define CVF_NO_ARCHIVE		0x1		// Not written in/read from the defaults file.
#define CVF_PROTECTED		0x2		// Can't be changed unless forced.
#define CVF_NO_MIN			0x4		// Don't use the minimum.
#define CVF_NO_MAX			0x8		// Don't use the maximum.
#define CVF_CAN_FREE		0x10	// The string can be freed.

// Console variable types.
enum cvartype_t
{
	CVT_NULL,
	CVT_BYTE,
	CVT_INT,
	CVT_FLOAT,
	CVT_CHARPTR					// ptr points to a char*, which points to the string.
};

// Console variable.
struct cvar_t
{
	char		*name;
	int			flags;
	cvartype_t	type;
	void		*ptr;			// Pointer to the data.
	float		min, max;		// Minimum and maximum values (for ints and floats).
	char		*help;			// A short help text.
};


//------------------------------------------------------------------------
//
// Networking
//
//------------------------------------------------------------------------

// Network Player Events
enum
{
	DDPE_ARRIVAL,				// A player has arrived.
	DDPE_EXIT,					// A player has exited the game.
	DDPE_CHAT_MESSAGE,			// A player has sent a chat message.
	DDPE_DATA_CHANGE			// The data for this player has been changed.
};	


// Actions.
struct action_t
{
	char		name[9];		// The name of the action.
	boolean		on;				// True if action is active.
};

#endif