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

/*FUNC DEC*/
char *strcpy(char *dest, const char *src);
int strcmp (const char* str1, const char* str2);

/* The file to input train data from */
FILE *readFS;

char *imgfilename = NULL;
char *retrieveFile = NULL;

struct Superblock{
    int blockSize;
    int blockCount;
    int fatStart;
    int fatBlocks;
    int rootDirStart;
    int rootDirBlock;
} superblock;

struct RootDirEntry{
    int status;
    int startBlock;
    int numBlocks;
    int fileSize;
    int createYear;
    int createMonth;
    int createDay;
    int createHour;
    int createMin;
    int createSec;
    int modYear;
    int modMonth;
    int modDay;
    int modHour;
    int modMin;
    int modSec;
    char fileName[DIRECTORY_MAX_NAME_LENGTH];
};
struct RootDirEntry entryToFind;

void readSuperblock()
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

struct RootDirEntry *readRootDir(){
    struct RootDirEntry *dirEntries = malloc(sizeof(struct RootDirEntry) * (superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK)); 
    int countEntries = 0;

    //set reader to start of root directory section
    fseek(readFS, superblock.rootDirStart * superblock.blockSize, SEEK_SET);

    for (int i = 0; i < superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK; i++){
        //STATUS
        int status = 0;
        fseek(readFS, DIRECTORY_STATUS_OFFSET, SEEK_CUR);
        fread(&status, DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET, 1, readFS);

        if (status != DIRECTORY_ENTRY_FILE_STATUS && status != DIRECTORY_ENTRY_DIRECTORY_STATUS){
            fseek(readFS, -(DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET), SEEK_CUR); //RESET CURSOR TO BEGINNING OF ENTRY
            fseek(readFS, DIRECTORY_ENTRY_SIZE, SEEK_CUR);
            continue;
        }

        struct RootDirEntry dirEntry; 
        dirEntry.status = status;
        //START BLOCK
        int startBlock = 0;
        fread(&startBlock, DIRECTORY_BLOCK_COUNT_OFFSET - DIRECTORY_START_BLOCK_OFFSET, 1, readFS);
        dirEntry.startBlock = startBlock;

        //BLOCK COUNT
        int numBlocks = 0;
        fread(&numBlocks, DIRECTORY_FILE_SIZE_OFFSET - DIRECTORY_BLOCK_COUNT_OFFSET, 1, readFS);
        dirEntry.numBlocks = htonl(numBlocks);

        //FILE SIZE
        int fileSize = 0;
        fread(&fileSize, DIRECTORY_CREATE_OFFSET - DIRECTORY_FILE_SIZE_OFFSET, 1, readFS);
        dirEntry.fileSize = htonl(fileSize);

        //CREATE DATE
        int createYear = 0;
        fread(&createYear, DATE_YEAR_ALLOCATED_SIZE, 1, readFS);
        dirEntry.createYear = htons(createYear);
        int createMonth = 0;
        fread(&createMonth, DATE_MONTH_ALLOCATED_SIZE, 1, readFS);
        dirEntry.createMonth = createMonth;
        int createDay = 0;
        fread(&createDay, DATE_DAY_ALLOCATED_SIZE, 1, readFS);
        dirEntry.createDay = createDay;
        int createHour = 0;
        fread(&createHour, DATE_HOUR_ALLOCATED_SIZE, 1, readFS);
        dirEntry.createHour = createHour;
        int createMin = 0;
        fread(&createMin, DATE_MIN_ALLOCATED_SIZE, 1, readFS);
        dirEntry.createMin = createMin;
        int createSec = 0;
        fread(&createSec, DATE_SEC_ALLOCATED_SIZE, 1, readFS);
        dirEntry.createSec = createSec;

        //MOD DATE
        int modYear = 0;
        fread(&modYear, DATE_YEAR_ALLOCATED_SIZE, 1, readFS);
        dirEntry.modYear = htons(modYear);
        int modMonth = 0;
        fread(&modMonth, DATE_MONTH_ALLOCATED_SIZE, 1, readFS);
        dirEntry.modMonth = modMonth;
        int modDay = 0;
        fread(&modDay, DATE_DAY_ALLOCATED_SIZE, 1, readFS);
        dirEntry.modDay = modDay;
        int modHour = 0;
        fread(&modHour, DATE_HOUR_ALLOCATED_SIZE, 1, readFS);
        dirEntry.modHour = modHour;
        int modMin = 0;
        fread(&modMin, DATE_MIN_ALLOCATED_SIZE, 1, readFS);
        dirEntry.modMin = modMin;
        int modSec = 0;
        fread(&modSec, DATE_SEC_ALLOCATED_SIZE, 1, readFS);
        dirEntry.modSec = modSec;

        //FILE NAME
        char fileName[DIRECTORY_MAX_NAME_LENGTH];
        fread(&fileName, DIRECTORY_MAX_NAME_LENGTH, 1, readFS);
        strcpy(dirEntry.fileName, fileName);

        dirEntries[countEntries++] = dirEntry;


        fseek(readFS, DIRECTORY_ENTRY_SIZE - (DIRECTORY_FILENAME_OFFSET + DIRECTORY_MAX_NAME_LENGTH), SEEK_CUR);
    }


    return dirEntries;
}

int findFile(struct RootDirEntry * dirEntries){
    if(retrieveFile == NULL){
        fprintf(stderr, "No file specified to be retrieved.\n");

        fclose(readFS);
        exit(0);
    }

    int MATCH = 0;

    for(int i = 0; i < superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK; i++){
        if(strcmp(dirEntries[i].fileName, retrieveFile) == MATCH){
            entryToFind = dirEntries[i];
            return 0;
        }
    }
    
    return -1;
}

void getFile(){
    //GET BLOCK LINKS
    int blocks[entryToFind.numBlocks];
    int nextBlock = entryToFind.startBlock;
    for(int i = 0; i <= entryToFind.numBlocks; i++){
        blocks[i] = htonl(nextBlock);
        fseek(readFS, superblock.fatStart * superblock.blockSize, SEEK_SET); //go to start of FAT block
        fseek(readFS, htonl(nextBlock) * FAT_ENTRY_SIZE, SEEK_CUR);        //find next block number for file
        fread(&nextBlock, FAT_ENTRY_SIZE, 1, readFS);                      //read in next block number
    }

    FILE *createCopy = fopen(retrieveFile, "w");

    if(createCopy == NULL){
        fprintf(stderr, "Error writing to file %s.\n", retrieveFile);
        fclose(readFS);
        exit(0);
    }
    //PRINT STREAM
    for (int i = 0; i <= entryToFind.numBlocks; i++){
    
        if(blocks[i] == FAT_EOF)
            break;
        
        char content[superblock.blockSize];
        fseek(readFS, blocks[i] * superblock.blockSize, SEEK_SET);
        fread(&content, superblock.blockSize, 1, readFS);
        if(entryToFind.numBlocks > 1){
            if(blocks[i+1] == FAT_EOF){
                fwrite(content, superblock.blockSize - (entryToFind.numBlocks * superblock.blockSize - entryToFind.fileSize), 1, createCopy);
            }else{
                fwrite(content, superblock.blockSize, 1, createCopy);
            }
        }else{
            fwrite(content, entryToFind.fileSize, 1, createCopy);
        }
    }
    fclose(createCopy);
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

    if( argc == 3){
        retrieveFile = argv[2];
    }

    readSuperblock();
    if(findFile(readRootDir()) < 0){
        fprintf(stderr, "File not found.\n");
        fclose(readFS);
        exit(0);
    }

    // printf("--SAVE FILE INFO--\n");
    // printf("File name: %s\n", entryToFind.fileName);
    // printf("Status: %02x\n", entryToFind.status);
    // printf("Start Block: %04x,  %d\n", entryToFind.startBlock, entryToFind.startBlock);
    // printf("Num Blocks: %04x, %d\n", entryToFind.numBlocks, entryToFind.numBlocks);
    // printf("File Size: %04x, %d\n", entryToFind.fileSize, entryToFind.fileSize);
    
    getFile();
    
    fclose(readFS);
    return 0;
}


