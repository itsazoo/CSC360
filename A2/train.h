/*
 * train.h
 *
 * Some definitions for the Trains
 */
#ifndef __TRAIN__H
#define __TRAIN__H

/* Trains at maximum MAX_LENGTH long and minimum MIN_LENGTH long */
#define MIN_LENGTH	3
#define MAX_LENGTH	25

/* Trains can be headed in one of two directions: EAST or WEST */
#define DIRECTION_NONE	0
#define	DIRECTION_WEST  1 	
#define	DIRECTION_EAST	2	

/* To simulate the length of the train, we will sleep for 
 * length*SLEEP_MULTIPLE when leaving the station and
 * crossing the bridge.
 */
 #define SLEEP_MULTIPLE	100000
 
/*
 * The information about a train.  You may need to add more fields to this
 * structure.
 *
 * Make sure you update the trainCreate function to provide default values for
 * your new fields.
 */
typedef struct
{
	int	trainId;
	int	direction;
	int	length;
	int	arrival;// you might not need this, I used it in my solution
} TrainInfo;

/*
 * Initialize the train library.  You must call this before any other
 * function.
 * 
 * The parameter is an optional file name which contains a 
 * description of trains to simulate.  If the filename is 
 * NULL, trains are generated randomly.
 */
void	initTrain ( char *filename );

/*
 * Allocate a new train structure with a new trainId, 
 * trainIds are assigned consecutively, starting at 0
 *
 * Randomly choose a direction and a length.
 *
 * This function malloc's space for the TrainInfo structure.  
 * The caller is responsible for freeing it.'
 */
TrainInfo *createTrain ( void );

#endif


