#ifndef BITMAP_H
#define BITMAP_H

#include "gfx.h"
#include "stdio.h"
#include "fat.h"

#include "../../libs/include/stdlib.h"

typedef struct tagBITMAPFILEHEADER {

    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;

}__attribute__((packed)) bmp_fileheader_t;

typedef struct tagBITMAPINFOHEADER {

    unsigned int biSize;
    long biWidth;
    long biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;

} bmp_infoheader_t;

typedef struct bitmap {

    unsigned int width;
    unsigned int height;

    struct FATContent* file;
    uint32_t header_offset;

    unsigned int total_size;
    uint32_t bpp;

    void* data_pointer;
    
} bitmap_t;

typedef struct palette {

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

} palette_t;

bitmap_t* BMP_create(char* filename);

void BMP_display_at(bitmap_t* bmp, int screen_x, int screen_y);
void BMP_unload(bitmap_t* bitmap);

#endif