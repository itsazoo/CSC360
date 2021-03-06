/*
 * assign3.c
 * Author: Annie
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "Constants.h"
#include <arpa/inet.h>

#define DIRECTORY_ENTRY_FILE_STATUS 0x03
#define DIRECTORY_ENTRY_DIRECTORY_STATUS 0x05
#define DATE_YEAR_ALLOCATED_SIZE 2
#define DATE_MONTH_ALLOCATED_SIZE 1
#define DATE_DAY_ALLOCATED_SIZE 1
#define DATE_HOUR_ALLOCATED_SIZE 1
#define DATE_MIN_ALLOCATED_SIZE 1
#define DATE_SEC_ALLOCATED_SIZE 1

/* The file to input train data from */
FILE *readFS;

char *imgfilename = NULL;

struct Superblock{
    int blockSize;
    int blockCount;
    int fatStart;
    int fatBlocks;
    int rootDirStart;
    int rootDirBlock;
} superblock;

void	readSuperblock ()
{
    //set reader to beginning of disk
    fseek(readFS, 0, SEEK_SET);
    
    //get Block size
    int blockSize = 0;
    fseek(readFS, BLOCKSIZE_OFFSET, SEEK_SET);
    fread(&blockSize, BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET, 1, readFS);

    superblock.blockSize = htons(blockSize);

    //get block count
    int blockCount = 0;
    fseek(readFS, BLOCKCOUNT_OFFSET, SEEK_SET);
    fread(&blockCount, FATSTART_OFFSET - BLOCKCOUNT_OFFSET, 1, readFS);

    superblock.blockCount = htonl(blockCount);

    //get FAT starts
    int fatStart = 0;
    fseek(readFS, FATSTART_OFFSET, SEEK_SET);
    fread(&fatStart, FATBLOCKS_OFFSET - FATSTART_OFFSET, 1, readFS);

    superblock.fatStart = htonl(fatStart);

    //get FAT blocks
    int fatBlocks = 0;
    fseek(readFS, FATBLOCKS_OFFSET, SEEK_SET);
    fread(&fatBlocks, ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET, 1, readFS);

    superblock.fatBlocks = htonl(fatBlocks);

    //get root directory start
    int rootDirStart = 0;
    fseek(readFS, ROOTDIRSTART_OFFSET, SEEK_SET);
    fread(&rootDirStart, ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET, 1, readFS);

    superblock.rootDirStart = htonl(rootDirStart);

    //get root directory blocks
    int rootDirBlock = 0;
    fseek(readFS, ROOTDIRBLOCKS_OFFSET, SEEK_SET);
    fread(&rootDirBlock, sizeof(int), 1, readFS);

    superblock.rootDirBlock = htonl(rootDirBlock);
}


void readRootDir(){

    //set reader to start of root directory section
    fseek(readFS, superblock.rootDirStart * superblock.blockSize, SEEK_SET);

    for (int i = 0; i < superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK; i++){
        //STATUS
        int status = 0;
        fseek(readFS, DIRECTORY_STATUS_OFFSET, SEEK_CUR);
        fread(&status, DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET, 1, readFS);

        if (status == DIRECTORY_ENTRY_FILE_STATUS)
        {
            printf("F ");
        }
        else if (status == DIRECTORY_ENTRY_DIRECTORY_STATUS)
        {
            printf("D ");
        }
        else
        {
            fseek(readFS, -(DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET), SEEK_CUR); //RESET CURSOR TO BEGINNING OF ENTRY
            fseek(readFS, DIRECTORY_ENTRY_SIZE, SEEK_CUR);
            continue;
        }

        //FILE SIZE
        int fileSize = 0;
        fseek(readFS, DIRECTORY_FILE_SIZE_OFFSET - DIRECTORY_START_BLOCK_OFFSET, SEEK_CUR);
        fread(&fileSize, DIRECTORY_CREATE_OFFSET - DIRECTORY_FILE_SIZE_OFFSET, 1, readFS);

        //MOD DATE
        fseek(readFS, DIRECTORY_MODIFY_OFFSET - DIRECTORY_CREATE_OFFSET, SEEK_CUR);
        int year = 0;
        fread(&year, DATE_YEAR_ALLOCATED_SIZE, 1, readFS);
        int month = 0;
        fread(&month, DATE_MONTH_ALLOCATED_SIZE, 1, readFS);
        int day = 0;
        fread(&day, DATE_DAY_ALLOCATED_SIZE, 1, readFS);
        int hour = 0;
        fread(&hour, DATE_HOUR_ALLOCATED_SIZE, 1, readFS);
        int min = 0;
        fread(&min, DATE_MIN_ALLOCATED_SIZE, 1, readFS);
        int sec = 0;
        fread(&sec, DATE_SEC_ALLOCATED_SIZE, 1, readFS);

        //FILE NAME
        char fileName[DIRECTORY_MAX_NAME_LENGTH];
        fread(&fileName, DIRECTORY_MAX_NAME_LENGTH, 1, readFS);

        printf("%10d ", htonl(fileSize));

        printf("%30s ", fileName);
        printf("%d/%02d/%02d %02d:%02d:%02d\n", htons(year), month, day, hour, min, sec);

        fseek(readFS, DIRECTORY_ENTRY_SIZE - (DIRECTORY_FILENAME_OFFSET + DIRECTORY_MAX_NAME_LENGTH), SEEK_CUR);
    }
}


int main ( int argc, char *argv[] ){

    if ( argc < 2 )
	{
		printf ("Usage: ./diskinfo {image filename}\n");
		printf ("\t\tfilename is the disk image file\n");
		exit(0);
	}

    if( argc >= 2){
        imgfilename = argv[1];

        readFS = fopen(imgfilename, "r");
        if (readFS == NULL)
        {
            fprintf(stderr, "error reading file %s\n", imgfilename);
            exit(0);
        }
    }

    readSuperblock();
    readRootDir();

    fclose(readFS);
    return 0;
}


