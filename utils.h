#ifndef UTILS_H
#define UTILS_H

typedef struct {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
} PngImage;

typedef struct {
    int width, height, **pixel_matrix;
} PgmImage;

PngImage readPng(char *filename);

void freePng(PngImage *pngImg);

void mallocPgm(PgmImage *pgmImg);

PgmImage pngToPgm(PngImage *pngImg);

PgmImage readPgm(char *filename);

void writePgm(PgmImage *pgmImg, char *filename);

void freePgm(PgmImage *pgmImg);

void binarizePgm(PgmImage *pgmImg);

//void slicePgm(PgmImage * img, int n);

int visitStar(PgmImage * img, int i, int j);

int countStars(PgmImage * img);

#endif