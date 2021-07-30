#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <mpi.h>
#include "utils.h"

int main(int argc, char *argv[]){

    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == 0){
        char *filename = "img/image.png";

        printf("%d: reading %s\n", rank, filename);
        PngImage pngImg = readPng(filename);
        printf("%d: reading successful\n", rank);
        printf("%d: converting %s to pgm\n", rank, filename);
        PgmImage pgmImg = pngToPgm(&pngImg);
        printf("%d: conversion successful\n", rank);

        int sliceWidth = pgmImg.width / (size-1);

        //slicing image in ranknum-1 vertical slices
        printf("%d: slicing full image in %d slices\n", rank, size-1);
        for(int i=1; i < size; i++){
            //creating separate image
            PgmImage sliceImg;
            sliceImg.width = sliceWidth;
            sliceImg.height = pgmImg.height;
            sliceImg.grays = 255;

            sliceImg.pixel_matrix = malloc(sliceImg.height*sizeof(int *));
            for (int j=0; j < sliceImg.height; j++){
                sliceImg.pixel_matrix[j] = (int *)malloc(sliceImg.width * sizeof(int));
            }

            for(int k=0; k < sliceImg.height; k++){
                for(int l=0; l < sliceImg.width; l++){
                    sliceImg.pixel_matrix[k][l] = pgmImg.pixel_matrix[k][(i * sliceWidth) + l];
                }
            }
            char *sliceImgFilename = sliceImageFilename(filename, i);
            writePgm(&sliceImg, sliceImgFilename);

            printf("%d: slice saved in file. sending file path to slave %d\n", rank, i);
            //SEND FILE PATH TO SLAVE i HERE
            
            free(sliceImgFilename);
            FREE2DARRAY(sliceImg.pixel_matrix, sliceImg.height);
        }
        printf("%d: waiting until all slaves return a count\n", rank);
        //RECEIVE COUNT MESSAGE FROM ALL SLAVES HERE
        //starCount = starCount + countStars(&sliceImg);
        printf("%d: total number of stars = \n", rank);

        FREE2DARRAY(pngImg.row_pointers, pngImg.height);
        FREE2DARRAY(pgmImg.pixel_matrix, pgmImg.height);
    }
    else {
        //RECEIVE FILE PATH FROM MASTER HERE
        printf("%d: waiting to receive a file path from master\n", rank);

        //printf("%d: reading slice from pgm file\n", rank);
        //PgmImage sliceImg = readPgm(sliceImgFilename);

        //printf("%d: binarizing pgm\n", rank);
        //binarizePgm(&pgmImg);

        //printf("%d: counting stars in slice\n", rank);
        //int starCount = countStars(&sliceImg);
        //printf("%d: counting finished: stars in slice = %d\n", rank, starCount);

        //SEND COUNT TO MASTER HERE
        //printf("%d: sending local count to master\n", rank);

        //remove(sliceImgFilename);
        //FREE2DARRAY(sliceImg.pixel_matrix, sliceImg.height);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}