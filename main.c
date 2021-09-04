#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <png.h>
#include <mpi.h>
#include <string.h>
#include "utils.h"

/* Projeto de Desempenho Paralelo - Programa contador de estrelas utilizando OpenMPI

Autor: Wendell João Castro de Ávila 2017.1.08.013
RA:    2017.1.08.013
Data:  3/9/2021

Execução:
mpirun -n numProcessadores starCount

*/

int main(int argc, char *argv[]){
    int rank, size;
    int totalCount = 0;

    MPI_Init(&argc, &argv);

    double starttime = MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    //master
    if(rank == 0){
        FILE *fp = fopen ("filepaths.txt", "r" );
        int hasNextImage;
        if(fp != NULL){
            int numFiles;
            fscanf(fp, "%d", &numFiles);
            
            for(int w = 0; w < numFiles; w++){
                char filename[100];
                fscanf(fp, "%s", filename);
                hasNextImage = 1;

                printf("%d: reading %s\n", rank, filename);
                PngImage pngImg = readPng(filename);
                printf("%d: reading finished\n", rank);
                printf("%d: converting %s to pgm\n", rank, filename);
                PgmImage pgmImg = pngToPgm(&pngImg);
                freePng(&pngImg);
                printf("%d: conversion finished\n", rank);

                int numSlices = size-1;
                //picking number of slices from argv if available
                //otherwise stick with standard of 1 slice per slave
                if(argc == 2){
                    char* endarg;
                    errno = 0;
                    long arg = strtol(argv[1], &endarg, 10);
                    if(*endarg == '\0' && errno == 0 && arg>0){
                        numSlices = arg;
                    }
                    else if(arg<=0){
                        numSlices = 1;
                    }
                }
                
                int sliceHeight = pgmImg.height / numSlices;
                int nextSlave = 1;
                //slicing image in ranknum-1 horizontal slices
                printf("%d: slicing full image in %d slices\n", rank, numSlices);
                for(int i=0; i < numSlices; i++){
                    MPI_Send(&hasNextImage, 1, MPI_INT, nextSlave, 0, MPI_COMM_WORLD);
                    printf("%d: sending slice %d to slave %d\n", rank, i+1, nextSlave);
                    MPI_Send(&pgmImg.width, 1, MPI_INT, nextSlave, 1, MPI_COMM_WORLD);
                    MPI_Send(&sliceHeight, 1, MPI_INT, nextSlave, 1, MPI_COMM_WORLD);
                    MPI_Send(&(pgmImg.pixel_matrix[i * sliceHeight][0]), pgmImg.width*sliceHeight, MPI_INT, nextSlave, 1, MPI_COMM_WORLD);
                    if((nextSlave+1) >= size){
                        nextSlave = 1;
                    }
                    else {
                        nextSlave++;
                    }
                }
                freePgm(&pgmImg);
            }//end for numFiles
            printf("%d: all files loaded and sent to slaves\n");
        }//end file check
        hasNextImage = 0;
        for(int i=1; i < size; i++){
            MPI_Send(&hasNextImage, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        fclose(fp);
        printf("%d: waiting until all slaves return their local count\n", rank);
        MPI_Reduce(MPI_IN_PLACE, &totalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        printf("\n%d: total number of stars: [%d]\n", rank, totalCount);
        double endtime = MPI_Wtime();
        printf("%d: elapsed time: %f seconds\n\n", rank, endtime-starttime);
    }//end master
    else { //slaves
        int localCount = 0;
        for(;;){ //while true
            int hasNextImage;
            MPI_Recv(&hasNextImage, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if(hasNextImage != 0){
                printf("    %d: waiting to receive image slice from master\n", rank);

                PgmImage sliceImg;
                MPI_Recv(&sliceImg.width, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&sliceImg.height, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                mallocPgm(&sliceImg);
                MPI_Recv(&(sliceImg.pixel_matrix[0][0]), sliceImg.width*sliceImg.height, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                printf("    %d: received image slice from master\n", rank);
                printf("    %d: binarizing slice\n", rank);
                binarizePgm(&sliceImg);
                printf("    %d: binarization finished\n", rank);
                printf("    %d: counting stars in slice\n", rank);
                int sliceCount = countStars(&sliceImg);
                freePgm(&sliceImg);
                printf("    %d: counting in slice finished: %d stars found\n", rank, sliceCount);
                localCount = localCount + sliceCount;
            }
            else {
                printf("    %d: no more images to be processed. slave %d has counted %d stars in total.\n", rank, rank, localCount);
                printf("    %d: sending local count to master\n", rank);
                MPI_Reduce(&localCount, &totalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
                break;
            }
        }//end while true
    }//end slaves

    MPI_Finalize();
    return 0;
}