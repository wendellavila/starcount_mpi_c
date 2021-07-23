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
    int width, height, **pixel_matrix;
} PgmImage;

PngImage read_png_file(char *filename);

void write_png_file(PngImage *img, char *filename);

PgmImage pngToPgm(PngImage *pngImg);

void binarizePgm(PgmImage *pgmImg);
#endif