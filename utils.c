#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include "utils.h"

/* To build:
* $ gcc -lz -lpng16 readpng.c -o readpng
*/

PngImage read_png_file(char *filename){
    /*
    * A simple libpng example program
    * http://zarb.org/~gc/html/libpng.html
    *
    * Modified by Yoshimasa Niwa to make it much simpler
    * and support all defined color_type.
    *
    * Copyright 2002-2010 Guillaume Cottenceau.
    *
    * This software may be freely redistributed under the terms
    * of the X11 license.
    */
    FILE *fp = fopen(filename, "rb");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    PngImage img;

    img.width      = png_get_image_width(png, info);
    img.height     = png_get_image_height(png, info);
    img.color_type = png_get_color_type(png, info);
    img.bit_depth  = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if(img.bit_depth == 16)
        png_set_strip_16(png);

    if(img.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(img.color_type == PNG_COLOR_TYPE_GRAY && img.bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if(img.color_type == PNG_COLOR_TYPE_RGB ||
        img.color_type == PNG_COLOR_TYPE_GRAY ||
        img.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(img.color_type == PNG_COLOR_TYPE_GRAY ||
        img.color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    //if (row_pointers) abort();

    img.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img.height);
    for(int y = 0; y < img.height; y++) {
        img.row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
    }

    png_read_image(png, img.row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);
    return img;
}

// void write_png_file(PngImage *img, char *filename) {
//     /*
//     * A simple libpng example program
//     * http://zarb.org/~gc/html/libpng.html
//     *
//     * Modified by Yoshimasa Niwa to make it much simpler
//     * and support all defined color_type.
//     *
//     * Copyright 2002-2010 Guillaume Cottenceau.
//     *
//     * This software may be freely redistributed under the terms
//     * of the X11 license.
//     */
//     int y;

//     FILE *fp = fopen(filename, "wb");
//     if(!fp) abort();

//     png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//     if (!png) abort();

//     png_infop info = png_create_info_struct(png);
//     if (!info) abort();

//     if (setjmp(png_jmpbuf(png))) abort();

//     png_init_io(png, fp);

//     // Output is 8bit depth, RGBA format.
//     png_set_IHDR(
//         png,
//         info,
//         img->width, img->height,
//         8,
//         PNG_COLOR_TYPE_RGBA,
//         PNG_INTERLACE_NONE,
//         PNG_COMPRESSION_TYPE_DEFAULT,
//         PNG_FILTER_TYPE_DEFAULT
//     );
//     png_write_info(png, info);

//     // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
//     // Use png_set_filler().
//     //png_set_filler(png, 0, PNG_FILLER_AFTER);

//     if (!img->row_pointers) abort();

//     png_write_image(png, img->row_pointers);
//     png_write_end(png, NULL);

//     fclose(fp);

//     png_destroy_write_struct(&png, &info);
// }

PgmImage pngToPgm(PngImage *pngImg){

    PgmImage pgmImg;
    pgmImg.width = pngImg->width;
    pgmImg.height = pngImg->height;

    pgmImg.pixel_matrix = malloc(pngImg->width*sizeof(int *));
    for (int i=0; i<pngImg->width; i++){
        pgmImg.pixel_matrix[i] = (int *)malloc(pngImg->height * sizeof(int));
    }

    for(int y = 0; y < pngImg->height; y++){
        png_bytep row = pngImg->row_pointers[y];
        for(int x = 0; x < pngImg->width; x++) {
            png_bytep px = &(row[x * 4]);
            // Do something awesome for each pixel here...
            //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
            pgmImg.pixel_matrix[x][y] = (px[0] + px[1] + px[2]) / 3;
        }
    }
    return pgmImg;
}

void binarizePgm(PgmImage *pgmImg){
    for(int x=0; x < pgmImg->width; x++){
        for(int y=0; y < pgmImg->width; y++){
            if(pgmImg->pixel_matrix[x][y] <= 128){
                pgmImg->pixel_matrix[x][y] = 0;
            }
            else {
                pgmImg->pixel_matrix[x][y] = 255;
            }
        }
    }
}

// void divide(PngImage *img, char *filename){
//     //generating output filename
//     int filenameLen = strlen(filename);

//     char *outFilename = filename;
//     outFilename[filenameLen-4] = '\0';
//     char *separator = "-";
//     char *extension = ".png";

//     int trailingInt = 1;
//     char trailingChar[1];
//     sprintf(trailingChar,"%ld", trailingInt);

//     outFilename = strcat(strcat(strcat(outFilename, separator), trailingChar), extension);

//     //generating output
//     write_png_file(img, outFilename);
// }