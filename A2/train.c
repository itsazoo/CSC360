/*
 * train.c
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "train.h"
 
//IMPLICIT FUNCTION DECLARATION
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);

/* A global to assign IDs to our trains */ 
int idNumber = 0;

/* If this value is set to 1, trains lengths
 * etc will be generated randomly.
 * 
 * If it is set to 0, the lengths etc will be
 * input from a file.
 */
int doRandom = 0;

/* The file to input train data from */
FILE *inputFile;

/* You can assume that no more than 80 characters
 * will be on any line in the input file
 */
#define MAXLINE		80

char buffer[MAXLINE];
char trains[200][MAXLINE];

void	initTrain ( char *filename )
{
	doRandom = 0;
	
	/* If no filename is specified, generate randomly */
	if ( !filename )
	{
		doRandom = 1;
		srandom(getpid());
	}
	else
	{
		inputFile = fopen(filename, "r");

		while(fgets(buffer, MAXLINE, inputFile)){
			strcpy(trains[idNumber], buffer); //store train info to the respective idNumber position in trains array
			idNumber++;
		}
		idNumber = 0; //reset idNumber

		fclose(inputFile);
	}
}
 
/*
 * Allocate a new train structure with a new trainId, trainIds are
 * assigned consecutively, starting at 0
 *
 * Either randomly create the train structures or read them from a file
 *
 * This function malloc's space for the TrainInfo structure.  
 * The caller is responsible for freeing it.
 */
TrainInfo *createTrain ( void )
{
	TrainInfo *info = (TrainInfo *)malloc(sizeof(TrainInfo));

	if (!doRandom)
	{
		/* Your code here to read a line of input
		 * from the input file 
		 */

		//get train info for the associated train id
		char train[MAXLINE];
		strcpy(train, trains[idNumber]);
		//

		//read in train direction
		int direction = (train[0] == 'W' || train[0] == 'w') ? DIRECTION_WEST : DIRECTION_EAST;
		
		//read in train length
		int i = 0;
		for(i = 0 ; train[i] != '\n'; i++){
			train[i] = train[i+1];
		}
		train[i] = 0;
		int length = atoi(train);
		//

		//initalize the struct elements
		info->trainId = idNumber++;
		info->arrival = 0;
		info->direction = direction;
		info->length = length;
	} else {
		/* I'm assigning the random values here in case
		* there is a problem with the input file.  Then
		* at least we know all the fields are initialized.
		*/	 
		info->trainId = idNumber++;
		info->arrival = 0;
		info->direction = (random() % 2 + 1);
		info->length = (random() % MAX_LENGTH) + MIN_LENGTH;
	}
	return info;
}


