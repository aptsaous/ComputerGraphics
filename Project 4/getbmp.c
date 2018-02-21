#include <stdio.h>
#include <stdlib.h>

#include "getbmp.h"

// Routine to read an uncompressed 24-bit color RGB bmp file into a
// 32-bit color RGBA bitmap file (A value being set to 1).
BitMapFile *getbmp( char *filename )
{
    int offset, headerSize;
    int j, x, y;
    
    // Initialize bitmap files for RGB (input) and RGBA (output).
    BitMapFile *bmpRGB = ( BitMapFile * ) malloc( sizeof( BitMapFile ) );
    BitMapFile *bmpRGBA = ( BitMapFile * ) malloc( sizeof( BitMapFile ) );
    
    FILE *fp;
    
    fp = fopen( filename, "rb" ); // Read input bmp file name.
    
    if ( fp == NULL )
    {
        printf( "Error opening file\n" );
        exit(0);
    }
    
    // Get starting point of image data in bmp file.
    fseek( fp, 10, SEEK_SET );
    fread( ( char * ) &offset, 4, 1, fp );
    
    fread( ( char * ) &headerSize, 4, 1, fp ); // Get header size of bmp file.
    
    // Get image width and height values from bmp file header.
    fseek( fp, 18, SEEK_SET );
    fread( ( char * ) &bmpRGB->sizeX, 4, 1, fp );
    fread( ( char * ) &bmpRGB->sizeY, 4, 1, fp );
    
    // Determine the length of zero-byte padding of the scanlines
    // (each scanline of a bmp file is 4-byte aligned by padding with zeros).
    int padding = ( 3 * bmpRGB->sizeX ) % 4 ? 4 - ( 3 * bmpRGB->sizeX ) % 4 : 0;
    
    // Add the padding to determine size of each scanline.
    int sizeScanline = 3 * bmpRGB->sizeX + padding;
    
    // Allocate storage for image in input bitmap file.
    int sizeStorage = sizeScanline * bmpRGB->sizeY;
    bmpRGB->data = ( unsigned char * ) malloc( sizeof( unsigned char ) * sizeStorage );
    
    // Read bmp file image data into input bitmap file.
    fseek( fp, offset, SEEK_SET );
    fread( ( char * ) bmpRGB->data, sizeStorage, 1, fp );
    
    // Reverse color values from BGR (bmp storage format) to RGB.
    int startScanline, endScanlineImageData, temp;
    for ( y = 0; y < bmpRGB->sizeY; y++ )
    {
        startScanline = y * sizeScanline; // Start position of y'th scanline.
        endScanlineImageData = startScanline + 3 * bmpRGB->sizeX; // Image data excludes padding.
        for ( x = startScanline; x < endScanlineImageData; x += 3 )
        {
            temp = bmpRGB->data[x];
            bmpRGB->data[x] = bmpRGB->data[x+2];
            bmpRGB->data[x+2] = temp;
        }
    }
    
    // Set image width and height values and allocate storage for image in output bitmap file.
    bmpRGBA->sizeX = bmpRGB->sizeX;
    bmpRGBA->sizeY = bmpRGB->sizeY;
    bmpRGBA->data = ( unsigned char * ) malloc( sizeof( unsigned char ) * 4*bmpRGB->sizeX * bmpRGB->sizeY );
    
    // Copy RGB data from input to output bitmap files, set output A to 1.
    for( j = 0; j < 4*bmpRGB->sizeY * bmpRGB->sizeX; j += 4 )
    {
        bmpRGBA->data[j] = bmpRGB->data[(j/4)*3];
        bmpRGBA->data[j+1] = bmpRGB->data[(j/4)*3+1];
        bmpRGBA->data[j+2] = bmpRGB->data[(j/4)*3+2];
        bmpRGBA->data[j+3] = 0xFF;
    }
    
    fclose( fp );
    
    return bmpRGBA;
}
