#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tga.h"

// Saves the buffer (which is formatted rgb565) to a Targa 24 image file.
int saveTGA24_rgb565(char *filename,int w,int h,uShort *buffer)
{
	FILE 			*file;
    TARGA_HEADER	header;
    int				i,k;
	uChar			*saveBuf;
	int				saveBufStart = w*h-1;	// From the end.

    if((file=fopen(filename,"wb"))==NULL) return 0;

	saveBuf = malloc(w*h*3);//new uChar[w*h*3];
	
    // Now we have the file. Let's setup the Targa header.
	header.idFieldSize = 0;		// No identification field.
    header.colorMapType = 0;		// No color map.
    header.imageType = 2;		// Targa type 2 (unmapped RGB).
    header.xOrigin = 0;
    header.yOrigin = 0;
    header.imageWidth = w;
    header.imageHeight = h;
	header.imagePixelSize = 24;
	header.imageDescriptor.attributeBits = 0;
    header.imageDescriptor.reserved = 0;		// Must be zero.
    header.imageDescriptor.screenOrigin = TGA_SCREEN_ORIGIN_LOWER;	// Must be.
    header.imageDescriptor.dataInterleave = TGA_INTERLEAVE_NONE;

    // Write the header.
    fwrite(&header,sizeof(header),1,file);

    // Then comes the actual image data. We'll first make a copy of the buffer
    // and convert the pixel data format.
    //uShort *saveBuf = new uShort[w*h];
    
	// Let's start to convert the buffer.
    for(k=0; k<h; k++)
		for(i=0; i<w; i++)	// pixels are at (i,k)
	    {
     		// Our job is to convert:
	        // 	buffer: 	RRRRRGGGGGGBBBBB 	rgb565
          	// Into:
     	    // 	saveBuf:	ARRRRRGGGGGBBBBB	argb1555 (a=0)
			uShort r,g,b;
          	uShort src = buffer[k*w+i];
			int destIndex = (saveBufStart-((k+1)*w-1-i))*3;

     	    r = (src>>11) & 0x1f;		// The last 5 bits.
			g = (src>>5) & 0x3f;		// The middle 6 bits (one bit'll be lost).
     	    b = src & 0x1f;			// The first 5 bits.
          // Let's put the result in the destination.
//               int newcol = (r<<10)+(g<<5)+b;
             // Let's flip the bytes around.
//               newcol = ((newcol>>8)&0xff) + ((newcol&0xff)<<8);
            
			saveBuf[destIndex+2] = b<<3;
            saveBuf[destIndex] = g<<2;
            saveBuf[destIndex+1] = r<<3;
            //saveBuf[saveBufStart-((k+1)*w-1-i)] = newcol;
     }
	 /// Write the converted buffer (bytes may go the wrong way around...!).
     fwrite(saveBuf, w*h*3, 1, file);
     // Clean up.
//     delete [] saveBuf;
	 free(saveBuf);
     fclose(file);
     // A successful saving operation.
     return 1;
}

// Save the rgb888 buffer as Targa 24.
int saveTGA24_rgb888(char *filename, int w, int h, uChar *buffer)
{
	FILE 			*file;
    TARGA_HEADER	header;
    int				i;
	uChar			*savebuf;

    if((file=fopen(filename, "wb")) == NULL) return 0; // Huh?

	savebuf = malloc(w*h*3);

    // Now we have the file. Let's setup the Targa header.
	header.idFieldSize = 0;			// No identification field.
    header.colorMapType = 0;		// No color map.
    header.imageType = 2;			// Targa type 2 (unmapped RGB).
    header.xOrigin = 0;
    header.yOrigin = 0;
    header.imageWidth = w;
    header.imageHeight = h;
	header.imagePixelSize = 24;		// 8 bits per channel.
	header.imageDescriptor.attributeBits = 0;
    header.imageDescriptor.reserved = 0;		// Must be zero.
    header.imageDescriptor.screenOrigin = TGA_SCREEN_ORIGIN_LOWER;	// Must be.
    header.imageDescriptor.dataInterleave = TGA_INTERLEAVE_NONE;

    // Write the header.
    fwrite(&header, sizeof(header), 1, file);

	// The save format is BRG.
	for(i=0; i<w*h; i++)
	{
		uChar *ptr = buffer + i*3, *save = savebuf + i*3;
		save[0] = ptr[2];	
		save[1] = ptr[1];	
		save[2] = ptr[0];	
	}
	fwrite(savebuf, w*h*3, 1, file);

	free(savebuf);
	fclose(file);
	// A success!
	return 1;
}

int saveTGA24_rgba8888(char *filename, int w, int h, uChar *buffer)
{
	FILE 			*file;
    TARGA_HEADER	header;
    int				i;
	uChar			*savebuf;

    if((file=fopen(filename, "wb")) == NULL) return 0; // Huh?

	savebuf = malloc(w*h*3);

    // Now we have the file. Let's setup the Targa header.
	header.idFieldSize = 0;			// No identification field.
    header.colorMapType = 0;		// No color map.
    header.imageType = 2;			// Targa type 2 (unmapped RGB).
    header.xOrigin = 0;
    header.yOrigin = 0;
    header.imageWidth = w;
    header.imageHeight = h;
	header.imagePixelSize = 24;		// 8 bits per channel.
	header.imageDescriptor.attributeBits = 0;
    header.imageDescriptor.reserved = 0;		// Must be zero.
    header.imageDescriptor.screenOrigin = TGA_SCREEN_ORIGIN_LOWER;	// Must be.
    header.imageDescriptor.dataInterleave = TGA_INTERLEAVE_NONE;

    // Write the header.
    fwrite(&header, sizeof(header), 1, file);

	// The save format is BRG.
	for(i=0; i<w*h; i++)
	{
		uChar *ptr = buffer + i*4, *save = savebuf + i*3;
		save[0] = ptr[2];	
		save[1] = ptr[1];	
		save[2] = ptr[0];	
	}
	fwrite(savebuf, w*h*3, 1, file);

	free(savebuf);
	fclose(file);
	// A success!
	return 1;
}

// Save the rgb888 buffer as Targa 16.
int saveTGA16_rgb888(char *filename, int w, int h, uChar *buffer)
{
	FILE 			*file;
    TARGA_HEADER	header;
    int				i;
	uShort			*savebuf;

    if((file=fopen(filename, "wb")) == NULL) return 0; // Huh?

	savebuf = malloc(w*h*2);

    // Now we have the file. Let's setup the Targa header.
	header.idFieldSize = 0;			// No identification field.
    header.colorMapType = 0;		// No color map.
    header.imageType = 2;			// Targa type 2 (unmapped RGB).
    header.xOrigin = 0;
    header.yOrigin = 0;
    header.imageWidth = w;
    header.imageHeight = h;
	header.imagePixelSize = 16;		// 8 bits per channel.
	header.imageDescriptor.attributeBits = 0;
    header.imageDescriptor.reserved = 0;		// Must be zero.
    header.imageDescriptor.screenOrigin = TGA_SCREEN_ORIGIN_LOWER;	// Must be.
    header.imageDescriptor.dataInterleave = TGA_INTERLEAVE_NONE;

    // Write the header.
    fwrite(&header, sizeof(header), 1, file);

	// The save format is BRG.
	for(i=0; i<w*h; i++)
	{
		uChar *ptr = buffer + i*3;
		// The format is _RRRRRGG GGGBBBBB.
		savebuf[i] = (ptr[2]>>3) + ((ptr[1]&0xf8)<<2) + ((ptr[0]&0xf8)<<7);
	}
	fwrite(savebuf, w*h*2, 1, file);

	free(savebuf);
	fclose(file);
	// A success!
	return 1;
}

