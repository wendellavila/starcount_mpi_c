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

    double starttime = MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    //master
    if(rank == 0){
        FILE *imgFile = fopen ("filepaths.txt", "r" );
        int hasNextImage;
        if(imgFile != NULL){
            int numFiles;
            fscanf(imgFile, "%d", &numFiles);
            
            for(int w = 0; w < numFiles; w++){
                char filename[100];
                fscanf(imgFile, "%s", filename);
                hasNextImage = 1;
                for(int i=1; i < size; i++){
                    MPI_Send(&hasNextImage, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }

                printf("%d: reading %s\n", rank, filename);
                PngImage pngImg = readPng(filename);
                printf("%d: reading finished\n", rank);
                printf("%d: converting %s to pgm\n", rank, filename);
                PgmImage pgmImg = pngToPgm(&pngImg);
                printf("%d: conversion finished\n", rank);

                int sliceHeight = pgmImg.height / (size-1);

                //slicing image in ranknum-1 horizontal slices
                printf("%d: slicing full image in %d slices\n", rank, size-1);
                for(int i=1; i < size; i++){
                    printf("%d: sending slice %d to slave %d\n", rank, i, i);
                    MPI_Send(&pgmImg.width, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                    MPI_Send(&sliceHeight, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                    MPI_Send(&(pgmImg.pixel_matrix[(i-1) * sliceHeight][0]), pgmImg.width*sliceHeight, MPI_INT, i, 1, MPI_COMM_WORLD);;
                }
                freePng(&pngImg);
                freePgm(&pgmImg);

                printf("%d: waiting until all slaves return a count\n", rank);
                MPI_Reduce(MPI_IN_PLACE, &totalStarCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
                printf("%d: partial number of stars: %d\n", rank, totalStarCount);
            }//end for numFiles
            printf("%d: all stars in all files counted\n");
        }//end file check
        hasNextImage = 0;
        for(int i=1; i < size; i++){
            MPI_Send(&hasNextImage, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        fclose(imgFile);
        printf("%d: total number of stars: [%d]\n", rank, totalStarCount);
        double endtime = MPI_Wtime();
        printf("%d: elapsed time: %f seconds\n", rank, endtime-starttime);
    }//end master
    else { //slaves
        for(;;){ //while true
            int hasNextImage;
            MPI_Recv(&hasNextImage, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if(hasNextImage != 0){
                printf("\t%d: waiting to receive image slice from master\n", rank);

                PgmImage sliceImg;
                MPI_Recv(&sliceImg.width, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                MPI_Recv(&sliceImg.height, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                mallocPgm(&sliceImg);
                MPI_Recv(&(sliceImg.pixel_matrix[0][0]), sliceImg.width*sliceImg.height, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                printf("\t%d: received image slice from master\n", rank);

                printf("\t%d: binarizing pgm\n", rank);
                binarizePgm(&sliceImg);
                printf("\t%d: binarization finished\n", rank);

                printf("\t%d: counting stars in slice %d\n", rank, rank);
                int starCount = countStars(&sliceImg);
                printf("\t%d: local counting finished: stars in slice: %d\n", rank, starCount);

                printf("\t%d: sending local count to master\n", rank);
                MPI_Reduce(&starCount, &totalStarCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

                freePgm(&sliceImg);
            }
            else {
                printf("\t%d: no more images to be processed\n", rank);
                break;
            }
        }//end while true
    }//end slaves

    MPI_Finalize();
    return 0;
}