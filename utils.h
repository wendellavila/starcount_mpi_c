#ifndef UTILS_H
#define UTILS_H

#define FREE2DARRAY(arr, n) {\
    for(int i = 0; i < n; i++) {\
        free(arr[i]);\
    }\
    free(arr);\
}

typedef struct {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
} PngImage;

typedef struct {
    int width, height, grays, **pixel_matrix;
} PgmImage;

PngImage readPng(char *filename);

PgmImage pngToPgm(PngImage *pngImg);

PgmImage readPgm(char *filename);

void writePgm(PgmImage *pgmImg, char *filename);

char *sliceImageFilename(char *filename, int trailingInt);

void binarizePgm(PgmImage *pgmImg);

//void slicePgm(PgmImage * img, int n);

int visitStar(PgmImage * img, int i, int j);

int countStars(PgmImage * img);

#endif