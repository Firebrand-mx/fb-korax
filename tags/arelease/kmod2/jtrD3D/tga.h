// Some routines to handle TGA files.
#ifndef __MY_TARGA_H_
#define __MY_TARGA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef	unsigned char		uChar;		// 1 byte
typedef	unsigned short int	uShort;		// 2 bytes

// Screen origins.
#define	TGA_SCREEN_ORIGIN_LOWER	0	// Lower left-hand corner.
#define	TGA_SCREEN_ORIGIN_UPPER	1	// Upper left-hand corner.

// Data interleaving.
#define	TGA_INTERLEAVE_NONE		0	// Non-interleaved.
#define	TGA_INTERLEAVE_TWOWAY	1	// Two-way (even/odd) interleaving.
#define	TGA_INTERLEAVE_FOURWAY	2	// Four-way interleaving.

#pragma pack(1)	// Confirm that the structures are formed correctly.
typedef struct 		// Targa image descriptor byte; a bit field
{
	uChar	attributeBits	: 4;	// Attribute bits associated with each pixel.
    uChar	reserved		: 1;	// A reserved bit; must be 0.
    uChar	screenOrigin	: 1;	// Location of screen origin; must be 0.
    uChar	dataInterleave	: 2;	// TGA_INTERLEAVE_*
} TARGA_IMAGE_DESCRIPTOR;

typedef struct
{
	uChar	idFieldSize;			// Identification field size in bytes.
	uChar	colorMapType;			// Type of the color map.
    uChar	imageType;				// Image type code.
    // Color map specification.
	uShort  colorMapOrigin;			// Index of first color map entry.
    uShort	colorMapLength;			// Number of color map entries.
	uChar	colorMapEntrySize;		// Number of bits in a color map entry (16/24/32).
    // Image specification.
    uShort	xOrigin;				// X coordinate of lower left corner.
    uShort	yOrigin;				// Y coordinate of lower left corner.
    uShort	imageWidth;				// Width of the image in pixels.
    uShort	imageHeight;			// Height of the image in pixels.
    uChar	imagePixelSize;			// Number of bits in a pixel (16/24/32).
	TARGA_IMAGE_DESCRIPTOR imageDescriptor;	// A bit field.
} TARGA_HEADER;
#pragma pack()		// Back to the default value.

// Save the rgb565 buffer as Targa 16.
int saveTGA24_rgb565(char *filename,int w,int h,uShort *buffer);

// Save the rgb888 buffer as Targa 24.
int saveTGA24_rgb888(char *filename, int w, int h, uChar *buffer);

int saveTGA24_rgba8888(char *filename, int w, int h, uChar *buffer);

// Save the rgb888 buffer as Targa 16.
int saveTGA16_rgb888(char *filename, int w, int h, uChar *buffer);

#ifdef __cplusplus
}
#endif

#endif
