#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include "utils.h"

PngImage readPng(char *filename){
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
    *
    * Taken from https://gist.github.com/niw/5963798 on 2021-07-23
    */
    FILE *fp = fopen(filename, "rb");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    PngImage pngImg;

    pngImg.width      = png_get_image_width(png, info);
    pngImg.height     = png_get_image_height(png, info);
    pngImg.color_type = png_get_color_type(png, info);
    pngImg.bit_depth  = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if(pngImg.bit_depth == 16)
        png_set_strip_16(png);

    if(pngImg.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(pngImg.color_type == PNG_COLOR_TYPE_GRAY && pngImg.bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if(pngImg.color_type == PNG_COLOR_TYPE_RGB ||
        pngImg.color_type == PNG_COLOR_TYPE_GRAY ||
        pngImg.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(pngImg.color_type == PNG_COLOR_TYPE_GRAY ||
        pngImg.color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    pngImg.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * pngImg.height);
    for(int y = 0; y < pngImg.height; y++) {
        pngImg.row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
    }

    png_read_image(png, pngImg.row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);
    return pngImg;
}

void freePng(PngImage *pngImg){
    for(int i = 0; i < pngImg->height; i++){
        free(pngImg->row_pointers[i]);
    }
    free(pngImg->row_pointers);
}

PgmImage pngToPgm(PngImage *pngImg){
    //creating and allocating pgm image
    PgmImage pgmImg;
    pgmImg.width = pngImg->width;
    pgmImg.height = pngImg->height;

    mallocPgm(&pgmImg);

    for(int i=0; i<pngImg->height; i++){
        png_bytep row = pngImg->row_pointers[i];
        for(int j = 0; j < pngImg->width; j++) {
            png_bytep px = &(row[j * 4]);
            // Do something for each pixel here...
            //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", i, j, px[0], px[1], px[2], px[3]);

            //average of RBG values gives grayscale pixel
            pgmImg.pixel_matrix[i][j] = (px[0] + px[1] + px[2]) / 3;
        }
    }
    return pgmImg;
}

void mallocPgm(PgmImage *pgmImg){
    // pgmImg->pixel_matrix = malloc(pgmImg->height*sizeof(int *));
    // for (int i=0; i<pgmImg->height; i++){
    //     pgmImg->pixel_matrix[i] = (int *)malloc(pgmImg->width * sizeof(int));
    // }
    int *data = (int *)malloc(pgmImg->height*pgmImg->width*sizeof(int));
    pgmImg->pixel_matrix = (int **)malloc(pgmImg->height*sizeof(int*));
    for(int i=0; i<pgmImg->height; i++){
        pgmImg->pixel_matrix[i] = &(data[pgmImg->width*i]);
    }
}

PgmImage readPgm(char *filename){
    FILE *fp;
    PgmImage pgmImg;
	char p2, comment[200];
    int grays;
	
	fp = fopen(filename, "r");

    fscanf(fp, "%s", &p2);                     //discarding read line
    fscanf(fp, "%s", comment);                 //discarding read line
    fgets(comment, 200, fp);                   //discarding read line
    fscanf(fp, "%d %d", &pgmImg.width, &pgmImg.height);
    fscanf(fp, "%d", &grays);                  //discarding read line

    mallocPgm(&pgmImg);
	
	for(int i=0; i<pgmImg.height; i++){
		for(int j=0; j<pgmImg.width; j++){
			fscanf(fp, "%d", &pgmImg.pixel_matrix[i][j]);
		}
	}
	
	fclose(fp);
    return pgmImg;
}

void writePgm(PgmImage *pgmImg, char *filename){
    FILE *fp = fopen(filename, "w");
 
    if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }

    int grays = 255;

    fprintf(fp, "P2\n");
    fprintf(fp, "# %s\n", filename);
    fprintf(fp, "%d %d\n", pgmImg->width, pgmImg->height);
    fprintf(fp, "%d\n", grays);

    for(int i=0; i < pgmImg->height; i++){
        for(int j=0; j < pgmImg->width; j++){
            fprintf(fp, "%d ", pgmImg->pixel_matrix[i][j]);
        }
        fprintf(fp, "\n");
    }
}

void freePgm(PgmImage *pgmImg){
    free(pgmImg->pixel_matrix[0]);
    free(pgmImg->pixel_matrix);
}

void binarizePgm(PgmImage *pgmImg){
    for(int i=0; i < pgmImg->height; i++){
        for(int j=0; j < pgmImg->width; j++){
            if(pgmImg->pixel_matrix[i][j] <= 128){
                pgmImg->pixel_matrix[i][j] = 0;
            }
            else {
                pgmImg->pixel_matrix[i][j] = 1;
            }
        }
    }
}

int visitStar(PgmImage * img, int i, int j){
    //checking if pixel is unvisited star
    if(img->pixel_matrix[i][j] == 1){
        img->pixel_matrix[i][j] = 2;

        //4neighborhood north
        //if has unvisited neighbor then visit it and its unvisited neighbors recursively
        if(i-1 >= 0 && img->pixel_matrix[i-1][j] == 1){
            //visiting neighbor to mark it as visited, ignoring count from it
            //counting is done only in the first visited pixel of the star
            visitStar(img, i-1, j);
        }
        //4neighborhood south
        //if has unvisited neighbor then visit it and its unvisited neighbors recursively
        if(i+1 < img->height && img->pixel_matrix[i+1][j] == 1){
            //visiting neighbor to mark it as visited, ignoring count from it
            //counting is done only in the first visited pixel of the star
            visitStar(img, i+1, j);
        }
        //4neighborhood west
        //if has unvisited neighbor then visit it and its unvisited neighbors recursively
        if(j-1 >= 0 && img->pixel_matrix[i][j-1] == 1){
            //visiting neighbor to mark it as visited, ignoring count from it
            //counting is done only in the first visited pixel of the star
            visitStar(img, i, j-1);
        }
        //4neighborhood east
        //if has unvisited neighbor then visit it and its unvisited neighbors recursively
        if(j+1 < img->width && img->pixel_matrix[i][j+1] == 1){
            //visiting neighbor to mark it as visited, ignoring count from it
            //counting is done only in the first visited pixel of the star
            visitStar(img, i, j+1);
        }
        //current pixel is the first visited pixel of this star, count +1
        return 1;
    }
    //if pixel is either already visited (==2) or not part of a star (==0), don't count
    return 0;
}

int countStars(PgmImage * img){
    int count = 0;
    for(int i = 0; i < img->height; i++){
        for(int j = 0; j < img->width; j++){
            count = count + visitStar(img, i, j);
        }
    }
    return count;
}