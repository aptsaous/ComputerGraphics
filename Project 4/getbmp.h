#ifndef GETBMP_H
#define GETBMP_H

typedef struct
{
    int sizeX;
    int sizeY;
    unsigned char *data;
} BitMapFile;

BitMapFile *getbmp( char *filename );

#endif