/*
 * assign3.c
 * Author: Annie
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "Constants.h"
#include <arpa/inet.h>
 


/* The file to input train data from */
FILE *readFS;

char *imgfilename = NULL;

struct Superblock
{
    int blockSize;
    int blockCount;
    int fatStart;
    int fatBlocks;
    int rootDirStart;
    int rootDirBlock;
} superblock;

void readSuperblock()
{
    //set reader to beginning of disk
    fseek(readFS, 0, SEEK_SET);

    //SUPER BLOCK INFO
    printf("Super block information: \n");

    //get Block size
    int blockSize = 0;
    fseek(readFS, BLOCKSIZE_OFFSET, SEEK_SET);
    fread(&blockSize, BLOCKCOUNT_OFFSET - BLOCKSIZE_OFFSET, 1, readFS);

    superblock.blockSize = htons(blockSize);
    printf("Block size: %d\n", superblock.blockSize);

    //get block count
    int blockCount = 0;
    fseek(readFS, BLOCKCOUNT_OFFSET, SEEK_SET);
    fread(&blockCount, FATSTART_OFFSET - BLOCKCOUNT_OFFSET, 1, readFS);

    superblock.blockCount = htonl(blockCount);
    printf("Block count: %d\n", superblock.blockCount);

    //get FAT starts
    int fatStart = 0;
    fseek(readFS, FATSTART_OFFSET, SEEK_SET);
    fread(&fatStart, FATBLOCKS_OFFSET - FATSTART_OFFSET, 1, readFS);

    superblock.fatStart = htonl(fatStart);
    printf("FAT starts: %d\n", superblock.fatStart);

    //get FAT blocks
    int fatBlocks = 0;
    fseek(readFS, FATBLOCKS_OFFSET, SEEK_SET);
    fread(&fatBlocks, ROOTDIRSTART_OFFSET - FATBLOCKS_OFFSET, 1, readFS);

    superblock.fatBlocks = htonl(fatBlocks);
    printf("FAT blocks: %d\n", superblock.fatBlocks);

    //get root directory start
    int rootDirStart = 0;
    fseek(readFS, ROOTDIRSTART_OFFSET, SEEK_SET);
    fread(&rootDirStart, ROOTDIRBLOCKS_OFFSET - ROOTDIRSTART_OFFSET, 1, readFS);

    superblock.rootDirStart = htonl(rootDirStart);
    printf("Root directory start: %d\n", superblock.rootDirStart);

    //get root directory blocks
    int rootDirBlock = 0;
    fseek(readFS, ROOTDIRBLOCKS_OFFSET, SEEK_SET);
    fread(&rootDirBlock, sizeof(int), 1, readFS);

    superblock.rootDirBlock = htonl(rootDirBlock);
    printf("Root directory blocks: %d\n", superblock.rootDirBlock);
}

void	readFAT ()
{
    //FAT INFO
    printf("FAT information: \n");

    int fatFreeBlock = 0;
    int fatReservedBlocks = 0;
    int fatAllocatedBlocks = 0;

    //set reader to beginning of FAT section
    fseek(readFS, superblock.fatStart*superblock.blockSize, SEEK_SET);

    for(int i = 0; i < superblock.fatBlocks * FAT_ENTRY_PER_BLOCK; i++){
        int temp = 0;
        fread(&temp, FAT_ENTRY_SIZE, 1, readFS);
        if(htonl(temp) == FAT_FREE){
            fatFreeBlock++;
        }else if(htonl(temp) == FAT_RESERVED){
            fatReservedBlocks++;
        }else{
            fatAllocatedBlocks++;
        }
    }

    printf("Free Blocks: %d\n", fatFreeBlock);
    printf("Reserved Blocks: %d\n", fatReservedBlocks);
    printf("Allocated Blocks: %d\n", fatAllocatedBlocks);


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
    printf("\n");
    readFAT();
    fclose(readFS);

    return 0;
}


