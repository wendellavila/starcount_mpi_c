#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include "utils.h"

int main(int argc, char *argv[]){
    if(argc < 2) abort();

    char *filename = argv[1];
    PngImage pngImg = read_png_file(filename);
    int width = pngImg.width;
    int height = pngImg.height;

    PgmImage pgmImg = pngToPgm(&pngImg);
    FREE2DARRAY(pngImg.row_pointers, height);
    
    binarizePgm(&pgmImg);
    FREE2DARRAY(pgmImg.pixel_matrix, width);

    return 0;
}