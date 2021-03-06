/*
 * assign3.c
 * Author: Annie
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "Constants.h"
#include <arpa/inet.h>
#include <time.h>
#include <libgen.h>

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
size_t strlen(const char *str);
char *basename(char *path);

/* The file to input train data from */
FILE *readFS;
FILE *writeFS;
FILE *saveFile;

char *imgfilename = NULL;
char *fileToSave = NULL;

int totalFreeSpace;

struct Superblock{
    int blockSize;
    int blockCount;
    int fatStart;
    int fatBlocks;
    int rootDirStart;
    int rootDirBlock;
} superblock;

struct FatBlock{
    int fatFreeBlock;
    int fatReservedBlocks;
    int fatAllocatedBlocks;
} fatBlock;

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
struct RootDirEntry entryToSave;


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


void	readFAT ()
{
    //FAT INFO
    // printf("FAT information: \n");

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
    fatBlock.fatFreeBlock = fatFreeBlock;
    fatBlock.fatReservedBlocks = fatReservedBlocks;
    fatBlock.fatAllocatedBlocks = fatAllocatedBlocks;
    // printf("Free Blocks: %d\n", fatFreeBlock);
    // printf("Reserved Blocks: %d\n", fatReservedBlocks);
    // printf("Allocated Blocks: %d\n", fatAllocatedBlocks);


}

struct RootDirEntry *readRootDir()
{
    struct RootDirEntry *dirEntries = malloc(sizeof(struct RootDirEntry) * (superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK));
    int countEntries = 0;

    //set reader to start of root directory section
    fseek(readFS, superblock.rootDirStart * superblock.blockSize, SEEK_SET);

    for (int i = 0; i < superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK; i++)
    {
        //STATUS
        int status = 0;
        fseek(readFS, DIRECTORY_STATUS_OFFSET, SEEK_CUR);
        fread(&status, DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET, 1, readFS);

        if (status != DIRECTORY_ENTRY_FILE_STATUS && status != DIRECTORY_ENTRY_DIRECTORY_STATUS)
        {
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

void writeToRootDir(){

    //set writer to start of root directory section
    fseek(writeFS, superblock.rootDirStart * superblock.blockSize, SEEK_SET);

    //traverse to an empty directory entry
    for (int i = 0; i < superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK; i++)
    {
        //STATUS
        int status = 0;
        fseek(writeFS, DIRECTORY_STATUS_OFFSET, SEEK_CUR);
        fread(&status, DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET, 1, writeFS);

        if (status == DIRECTORY_ENTRY_FILE_STATUS || status == DIRECTORY_ENTRY_DIRECTORY_STATUS || status == DIRECTORY_ENTRY_USED){
            fseek(writeFS, -(DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET), SEEK_CUR); //RESET CURSOR TO BEGINNING OF ENTRY
            fseek(writeFS, DIRECTORY_ENTRY_SIZE, SEEK_CUR);
            continue;
        }
        break;
    }
    fseek(writeFS, -(DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET), SEEK_CUR); //RESET CURSOR TO BEGINNING OF ENTRY

    //STATUS
    int status = entryToSave.status;
    fwrite(&status, DIRECTORY_START_BLOCK_OFFSET - DIRECTORY_STATUS_OFFSET, 1, writeFS);
    //START BLOCK
    int startBlock = ntohl(entryToSave.startBlock);
    fwrite(&startBlock, DIRECTORY_BLOCK_COUNT_OFFSET - DIRECTORY_START_BLOCK_OFFSET, 1, writeFS);
    //NUM BLOCKS
    int numBlocks = ntohl(entryToSave.numBlocks);
    fwrite(&numBlocks, DIRECTORY_FILE_SIZE_OFFSET - DIRECTORY_BLOCK_COUNT_OFFSET, 1, writeFS);
    //FILE SIZE
    int fileSize = ntohl(entryToSave.fileSize);
    fwrite(&fileSize, DIRECTORY_CREATE_OFFSET - DIRECTORY_FILE_SIZE_OFFSET, 1, writeFS);
    
    //CREATE DATE
    int createYear = ntohs(entryToSave.createYear);
    fwrite(&createYear, DATE_YEAR_ALLOCATED_SIZE, 1, writeFS);
    int createMonth = entryToSave.createMonth;
    fwrite(&createMonth, DATE_MONTH_ALLOCATED_SIZE, 1, writeFS);
    int createDay = entryToSave.createDay;
    fwrite(&createDay, DATE_DAY_ALLOCATED_SIZE, 1, writeFS);
    int createHour = entryToSave.createHour;
    fwrite(&createHour, DATE_HOUR_ALLOCATED_SIZE, 1, writeFS);
    int createMin = entryToSave.createMin;
    fwrite(&createMin, DATE_MIN_ALLOCATED_SIZE, 1, writeFS);
    int createSec = entryToSave.createSec;
    fwrite(&createSec, DATE_SEC_ALLOCATED_SIZE, 1, writeFS);

    //MOD DATE
    int modYear = ntohs(entryToSave.modYear);
    fwrite(&modYear, DATE_YEAR_ALLOCATED_SIZE, 1, writeFS);
    int modMonth = entryToSave.modMonth;
    fwrite(&modMonth, DATE_MONTH_ALLOCATED_SIZE, 1, writeFS);
    int modDay = entryToSave.modDay;
    fwrite(&modDay, DATE_DAY_ALLOCATED_SIZE, 1, writeFS);
    int modHour = entryToSave.modHour;
    fwrite(&modHour, DATE_HOUR_ALLOCATED_SIZE, 1, writeFS);
    int modMin = entryToSave.modMin;
    fwrite(&modMin, DATE_MIN_ALLOCATED_SIZE, 1, writeFS);
    int modSec = entryToSave.modSec;
    fwrite(&modSec, DATE_SEC_ALLOCATED_SIZE, 1, writeFS);

    //FILE NAME
    char fileName[DIRECTORY_MAX_NAME_LENGTH];
    strcpy(fileName, entryToSave.fileName);
    fwrite(fileName, DIRECTORY_MAX_NAME_LENGTH, 1, writeFS);
}

void readFileToSave(){
    strcpy(entryToSave.fileName, basename(fileToSave));
    entryToSave.status =  DIRECTORY_ENTRY_FILE_STATUS;

    //GET CURRENT TIME
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    entryToSave.createYear = timeinfo->tm_year + 1900;
    entryToSave.createMonth = timeinfo->tm_mon + 1;
    entryToSave.createDay = timeinfo->tm_mday;
    entryToSave.createHour = timeinfo->tm_hour;
    entryToSave.createMin = timeinfo->tm_min;
    entryToSave.createSec = timeinfo->tm_sec;

    entryToSave.modYear = entryToSave.createYear;
    entryToSave.modMonth = entryToSave.createMonth;
    entryToSave.modDay = entryToSave.createDay;
    entryToSave.modHour = entryToSave.createHour;
    entryToSave.modMin = entryToSave.createMin;
    entryToSave.modSec = entryToSave.createSec;
}

void writeToDisk(){
    int allocatedBlocks[superblock.blockCount];
    int numBlocks = 0;
    int fileSize = 0;

    //set reader to beginning of FAT
    fseek(readFS, superblock.fatStart * superblock.blockSize, SEEK_SET);

    for(int i = 0; i < superblock.blockCount; i++){
        int findFreeBlock = 0;
        //use read pointer to read in block to see if it's free
        fread(&findFreeBlock, FAT_ENTRY_SIZE, 1, readFS);

        if (findFreeBlock != DIRECTORY_ENTRY_FREE){
            continue;
        }

        //if you reach here that means the block is free
        allocatedBlocks[numBlocks++] = i;
        
        //read file contents into block size buffer
        char contentsToCopy[superblock.blockSize];
        int bytesRead= fread(&contentsToCopy, 1, superblock.blockSize, saveFile);
        fileSize += bytesRead;

        if(fileSize >= totalFreeSpace){
            fprintf(stderr, "File exceeds free space available (%d bytes).\n", totalFreeSpace);
            fclose(writeFS);
            fclose(readFS);
            fclose(saveFile);
            exit(0);
        }
        
        //go to the write buffered file contents to this block
        fseek(writeFS, i * superblock.blockSize, SEEK_SET);
        fwrite(contentsToCopy, bytesRead, 1, writeFS);
        
        //if file contents don't fill the block then EOF reached
        if(bytesRead < superblock.blockSize){
            allocatedBlocks[numBlocks] = FAT_EOF;
            break;
        }

    }

    //save root entry data 
    entryToSave.startBlock = allocatedBlocks[0];
    entryToSave.fileSize = fileSize;
    entryToSave.numBlocks = numBlocks;

    //update FAT with the file's block locations
    for (int i = 0; i < numBlocks; i++){
        //set write pointer to beginning of FAT table
        fseek(writeFS, superblock.fatStart * superblock.blockSize, SEEK_SET);
        //go to the respective FAT entry for the block location
        fseek(writeFS, allocatedBlocks[i] * FAT_ENTRY_SIZE, SEEK_CUR);

        //write the next block location to this FAT entry
        int blockNum = ntohl(allocatedBlocks[i+1]);
        fwrite(&blockNum, FAT_ENTRY_SIZE, 1, writeFS);
    }
    //write EOF to the respective FAT entry for the last block location
    int eof = allocatedBlocks[numBlocks];
    fwrite(&eof, FAT_ENTRY_SIZE, 1, writeFS);
}

int findFile(struct RootDirEntry * dirEntries){

    int MATCH = 0;

    for(int i = 0; i < superblock.rootDirBlock * DIRECTORY_ENTRY_PER_BLOCK; i++){
        if(strcmp(dirEntries[i].fileName, basename(fileToSave)) == MATCH){
            return 0;
        }
    }
    
    return -1;
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
        if (readFS == NULL){
            fprintf(stderr, "error opening file %s in read mode\n", imgfilename);
            exit(0);
        }

        writeFS = fopen(imgfilename, "r+");
        if(writeFS == NULL){
            fprintf(stderr, "error opening file %s in write mode\n", imgfilename);
            fclose(readFS);
            exit(0);
        }
    }

    if( argc == 3){
        fileToSave = argv[2];

        saveFile = fopen(fileToSave, "r");

        if(saveFile == NULL){
            fprintf(stderr, "File not found.\n");
            fclose(writeFS);
            fclose(readFS);
            exit(0);
        }

        if (strlen(fileToSave) > DIRECTORY_MAX_NAME_LENGTH)
        {
            fprintf(stderr, "File name exceeds max length (%d).\n", DIRECTORY_MAX_NAME_LENGTH);
            fclose(writeFS);
            fclose(readFS);
            fclose(saveFile);
            exit(0);
        }
    }

    readSuperblock();
    readFAT();
    totalFreeSpace = superblock.blockSize * fatBlock.fatFreeBlock;
    // printf("--SUPERBLOCK INFO--\n");
    // printf("Block Size: %d\n", superblock.blockSize);
    // printf("Block count: %d\n", superblock.blockCount);
    // printf("Fat start: %d\n", superblock.fatStart);
    // printf("Root dir start: %d\n", superblock.rootDirStart);
    // printf("Root dir block: %d\n", superblock.rootDirBlock);
    // printf("--------------------\n\n");
    
    if(findFile(readRootDir()) == 0){
        fprintf(stderr, "File already exists.\n");

        fclose(writeFS);
        fclose(readFS);
        fclose(saveFile);
        exit(0);
    }

    writeToDisk();
    readFileToSave();

    // printf("--SAVE FILE INFO--\n");
    // printf("File name: %s\n", entryToSave.fileName);
    // printf("Status: %02x\n", entryToSave.status);
    // printf("Start Block: %04x,  %d\n", entryToSave.startBlock,  entryToSave.startBlock);
    // printf("Num Blocks: %04x, %d\n", entryToSave.numBlocks, entryToSave.numBlocks);
    // printf("File Size: %04x, %d\n", entryToSave.fileSize, entryToSave.fileSize);

    writeToRootDir();

    fclose(writeFS);
    fclose(readFS);
    fclose(saveFile);
    return 0;
}


