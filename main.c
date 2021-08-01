#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <mpi.h>
#include <string.h>
#include "utils.h"

int main(int argc, char *argv[]){

    int rank, size;
    int totalStarCount = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    if(rank == 0){
        // FILE *fp = fopen ("filepaths.txt", "r" );
        // if(fp != NULL){
        //     char filename [128];
        //     while(fgets(filename, sizeof filename, fp) != NULL){
                
        //     }

        // }
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

            mallocPgm(&sliceImg);

            for(int k=0; k < sliceImg.height; k++){
                for(int l=0; l < sliceImg.width; l++){
                    sliceImg.pixel_matrix[k][l] = pgmImg.pixel_matrix[k][(i * sliceWidth) + l];
                }
            }

            printf("%d: sending slice %d to slave %d\n", rank, i, i);
            MPI_Send(&sliceImg.width, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&sliceImg.height, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&(sliceImg.pixel_matrix[0][0]), sliceImg.width*sliceImg.height, MPI_INT, i, 1, MPI_COMM_WORLD);;
            freePgm(&sliceImg);
        }
        freePng(&pngImg);
        freePgm(&pgmImg);

        printf("%d: waiting until all slaves return a count\n", rank);
        MPI_Reduce(MPI_IN_PLACE, &totalStarCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        printf("%d: total number of stars = %d\n", rank, totalStarCount);
    }
    else {
        printf("\t%d: waiting to receive image slice from master\n", rank);

        PgmImage sliceImg;
        MPI_Recv(&sliceImg.width, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&sliceImg.height, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        mallocPgm(&sliceImg);
        MPI_Recv(&(sliceImg.pixel_matrix[0][0]), sliceImg.width*sliceImg.height, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        printf("\t%d: received image slice from master\n", rank);

        printf("\t%d: binarizing pgm\n", rank);
        binarizePgm(&sliceImg);
        printf("\t%d: binarization successful\n", rank);

        printf("\t%d: counting stars in slice %d\n", rank, rank);
        int starCount = countStars(&sliceImg);
        printf("\t%d: counting finished: stars in slice = %d\n", rank, starCount);

        printf("\t%d: sending local count to master\n", rank);
        MPI_Reduce(&starCount, &totalStarCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        freePgm(&sliceImg);
    }
    MPI_Finalize();

    return 0;
}