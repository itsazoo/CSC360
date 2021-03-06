/*
 * assign2.c
 *
 * Simulate trains arriving, leaving, and crossing a bridge using threads
 * Author:Annie Zhou
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include "train.h"

/*
 * If you uncomment the following line, some debugging
 * output will be produced.
 *
 * Be sure to comment this line out again before you submit 
 */

//#define DEBUG	1 
pthread_mutex_t	bridge_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t arrive_bridge = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t leave_bridge = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convar = PTHREAD_COND_INITIALIZER;

void ArriveBridge (TrainInfo *train);
void CrossBridge (TrainInfo *train);
void LeaveBridge (TrainInfo *train);

int east_arrive = 0;
int west_arrive = 0;
int east_leave = 2;
int waiting_east[200];
int waiting_west[200];

/*
 * This function is started for each thread created by the
 * main thread.  Each thread is given a TrainInfo structure
 * that specifies information about the train the individual 
 * thread is supposed to simulate.
 */
void * Train ( void *arguments )
{
	TrainInfo	*train = (TrainInfo *)arguments;

	/* Sleep to simulate different arrival times */
	usleep (train->length*SLEEP_MULTIPLE);

	ArriveBridge (train);
	CrossBridge  (train);
	LeaveBridge  (train); 

	/* I decided that the paramter structure would be malloc'd 
	 * in the main thread, but the individual threads are responsible
	 * for freeing the memory.
	 *
	 * This way I didn't have to keep an array of parameter pointers
	 * in the main thread.
	 */
	free (train);
	return NULL;
}

/**
 * debugging method
 * prints out trains in waiting_east[]
 * */
void print_East_Trains()
{
	printf("East bound: {");
	for (int i = 0; i < 200; i++)
	{
		if (waiting_east[i] > -1)
		{
			printf("%d:%d,", i, waiting_east[i]);
		}
	}
	printf("}\n");
}
/**
 * debugging method
 * prints out trains in waiting_west[]
 * */
void print_West_Trains()
{
	printf("West Bound: {");
	for (int i = 0; i < 200; i++)
	{

		if (waiting_west[i] > -1)
		{
			printf("%d:%d,", i, waiting_west[i]);
		}
	}
	printf("}\n");
}

/*
 * You will need to add code to this function to ensure that
 * the trains cross the bridge in the correct order.
 */
void ArriveBridge ( TrainInfo *train )
{
	printf ("Train %2d arrives going %s\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	/* Your code here... */
	int train_id = train->trainId;

	if (train->direction == DIRECTION_EAST){
		
		pthread_mutex_lock(&arrive_bridge);
		waiting_east[train_id] = east_arrive++; //assign the waiting position for east bound trains
		pthread_mutex_unlock(&arrive_bridge);
		//print_East_Trains();
	}else{
		pthread_mutex_lock(&arrive_bridge);
		waiting_west[train_id] = west_arrive++; //assign the waiting position for west bound trains
		pthread_mutex_unlock(&arrive_bridge);
		//print_West_Trains();
	}

	pthread_mutex_lock(&bridge_time);

	//catch all trains trying to lock the mutex
	for(;;){

		//let east bound train go if there's no waiting west bound train
		if (waiting_east[train_id] == 0 && west_arrive == 0)
			break;

		//let west bound train go if there's no waiting east bound train
		if (waiting_west[train_id] == 0 && east_arrive == 0)
			break;

		//east bound train has priority if the 2east:1west ration hasn't been satisfied yet
		if (waiting_east[train_id] == 0 && east_leave > 0)
			break;

		//let the west bound train go if the 2east:1west ration has been satisfied
		if (waiting_west[train_id] == 0 && east_leave == 0)
		{
			east_leave = 2; //reset the ratio 
			break;
		}
		pthread_cond_wait(&convar, &bridge_time);

	}

}


/*
 * Simulate crossing the bridge.  You shouldn't have to change this
 * function.
 */
void CrossBridge ( TrainInfo *train )
{
	printf ("Train %2d is ON the bridge (%s)\n", train->trainId,
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
	
	/* 
	 * This sleep statement simulates the time it takes to 
	 * cross the bridge.  Longer trains take more time.
	 */
	usleep (train->length*SLEEP_MULTIPLE);

	printf ("Train %2d is OFF the bridge(%s)\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
}

/*
 * Add code here to make the bridge available to waiting
 * trains...
 */
void LeaveBridge ( TrainInfo *train )
{
	pthread_mutex_unlock(&bridge_time);

	if (train->direction == DIRECTION_EAST)
	{
		pthread_mutex_lock(&leave_bridge);
		if(east_arrive > 0)
			east_arrive--; //decrement the waiting position for east bound trains
		if(east_leave > 0 && west_arrive > 0)
			east_leave--; //update the ratio counter if there's west boudn trains waiting
		pthread_mutex_unlock(&leave_bridge);

		//update all waiting position numbers for east bound trains
		for (int i = 0; i < 200; i++)
		{
			int curr_pos = waiting_east[i];
			if(curr_pos > -1){

				pthread_mutex_lock(&leave_bridge);
				waiting_east[i] = --curr_pos;
				pthread_mutex_unlock(&leave_bridge);
				
			}
		}
		//print_East_Trains();
	}else{
		pthread_mutex_lock(&leave_bridge);
		if(west_arrive > 0)
			west_arrive--; //decrement the waiting position for west bound trains
		pthread_mutex_unlock(&leave_bridge);

		//update all waiting position numbers for west bound trains;
		for (int i = 0; i < 200; i++)
		{
			int curr_pos = waiting_west[i];
			if (curr_pos > -1)
			{
				pthread_mutex_lock(&leave_bridge);
				waiting_west[i] = --curr_pos;
				pthread_mutex_unlock(&leave_bridge);
				
			}
		}
		//print_West_Trains();
	}

	pthread_cond_broadcast(&convar);
}

int main ( int argc, char *argv[] )
{
	int		trainCount = 0;
	char 		*filename = NULL;
	pthread_t	*tids;
	int		i;

		
	/* Parse the arguments */
	if ( argc < 2 )
	{
		printf ("Usage: part1 n {filename}\n\t\tn is number of trains\n");
		printf ("\t\tfilename is input file to use (optional)\n");
		exit(0);
	}
	
	if ( argc >= 2 )
	{
		trainCount = atoi(argv[1]);
	}
	if ( argc == 3 )
	{
		filename = argv[2];
	}	
	
	initTrain(filename);
	
	/*
	 * Since the number of trains to simulate is specified on the command
	 * line, we need to malloc space to store the thread ids of each train
	 * thread.
	 */
	tids = (pthread_t *) malloc(sizeof(pthread_t)*trainCount);
	
	//initialize waiting lists
	for(int i = 0; i<200; i++){
		waiting_west[i] = -1;
		waiting_east[i] = -1;
	}
	/*
	 * Create all the train threads pass them the information about
	 * length and direction as a TrainInfo structure
	 */
	for (i=0;i<trainCount;i++)
	{
		TrainInfo *info = createTrain();


		printf ("Train %2d headed %s length is %d\n", info->trainId,
			(info->direction == DIRECTION_WEST ? "West" : "East"),
			info->length );
		if ( pthread_create (&tids[i],0, Train, (void *)info) != 0 )
		{
			printf ("Failed creation of Train.\n");
			exit(0);
		}
	}

	/*
	 * This code waits for all train threads to terminate
	 */
	for (i=0;i<trainCount;i++)
	{
		pthread_join (tids[i], NULL);
	}
	
	pthread_mutex_destroy(&bridge_time);
	pthread_mutex_destroy(&arrive_bridge);
	pthread_mutex_destroy(&leave_bridge);
	pthread_cond_destroy(&convar);
	free(tids);
	return 0;
}

