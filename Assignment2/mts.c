#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "train_data.h"
#include "priority_queue.h"

// Number of trains in the file
int number_of_trains;

// Number of active threads
static int active_train_threads;

// Number of trains waiting
static int number_of_waiting_trains;

// Used to signal all the train threads to start together
int start_threads;

// Single queue to store all the trains.
station_queue *station = NULL;

static void* TrainThread(void *param);
static void* ControllerThread(void *param);

pthread_mutex_t controller_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t train_left_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t main_track = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t signal_controller = PTHREAD_COND_INITIALIZER;
pthread_cond_t train_left = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
	char *filename;
	
	FILE *inputFile;
	int i; // For loops
	char read_line[100]; // To store characters coming from fgetc
	char* read_char;
	int temp_int;
	int error = 0;

	// Validate arguments
	
	if(argc == 3){

		// Store the arguments
		filename = (char *)argv[1];

		//printf("Entered filename is: %s\n",filename);

		number_of_trains = (int)atoi(argv[2]);

		//printf("Entered number of trains: %d\n",number_of_trains);

		if(number_of_trains > 0){
			// Initialize the data structure
			train_info *train = (train_info*)malloc(sizeof(train_info) * number_of_trains);

			// Read the file
			inputFile = fopen(argv[1],"r");

			if(inputFile){
				
				i = 0;
				// Process file
				while(!feof(inputFile) && (i < number_of_trains)){
					
					// Read line of te file
					if(NULL != fgets(read_line,100,inputFile))
					{
					
						read_char = strtok(read_line,":,");
														
						// Read the priority
						switch(read_char[0]){
							case 'e':
								train[i].direction = 'e';
								train[i].priority = 0;
								break;
							case 'E':
								train[i].direction = 'e';
								train[i].priority = 1;
								break;
							case 'w':
								train[i].direction = 'w';
								train[i].priority = 0;
		
								break;
							case 'W':
								train[i].direction = 'w';
								train[i].priority = 1;
		
								break;
							default:
								error = 1;
								break;
						}

						if(!error)
						{
							// Read the loading time
							read_char = strtok(NULL,":,");
								
							temp_int = (int)atoi(read_char);
							train[i].loadTime = temp_int;

							// Read the crossing time
							read_char = strtok(NULL,":,");

							temp_int = (int)atoi(read_char);
							train[i].crossTime = temp_int;
							
							// Include the position in the file
							train[i].inputId = i;
							i++;	
							
							// Each train will have a seperate condition
							// this will allow each train to be signalled
							// seperately. It will increase the effieciency.
							pthread_cond_init(&(train[i].go), NULL);
							
							train[i].signalled = 0;
						}
					}else{
						error = 1;
						break;
					}			
				}

				if(!error)
				{
					// Start the controller thread.
					pthread_t controller_thread;
					
					// The controller thread resembles the station.
					pthread_create(&controller_thread, NULL, ControllerThread, train);
					
					// Wait until controller exited
					pthread_join(controller_thread, NULL);
					
					free(train);
				}else
				{
					// File format error
					printf("Input file formatting error!!!\n");
				}
			}else{
				// File name invalid
				printf("Invalid filename!!!\n");
			}
		}else{
			// Number of trains is invalid

			printf("Number of trains is invalid!!!\n");
		}		
	}else{
		// Less than two arguments entered in command line
		printf("Input arguments are invalid!!!\n");
	}

	return 0;
}


// Controller thread function
static void* ControllerThread(void *param)
{
	train_info *train = (train_info*)param;
	pthread_t *train_threads = (pthread_t*)malloc(sizeof(pthread_t) * number_of_trains);	
	int ret = 0;
	int i=0;
	char last_train_crossed = 'w';	
	train_info* get_train;	
	active_train_threads = 0;
	number_of_waiting_trains = 0;
	
	for(i = 0; i < number_of_trains; i++)
	{
		pthread_create(&train_threads[i], NULL, TrainThread, &train[i]);
	}				
	
	ret = pthread_mutex_lock(&controller_mutex);
		
	if(ret == 0)
	{
		// Wait until all train threads created
		while(number_of_trains != active_train_threads)
		{
			ret = pthread_cond_wait(&signal_controller, &controller_mutex);
			
			if(ret != 0)
			{
				printf("Internal error on pthread_cond_wait call.\n");
				break;
			}
		}
	}
	
	ret = pthread_mutex_unlock(&controller_mutex);
	
	// Signal all the threads to start
	start_threads = 1;
	
	ret = pthread_mutex_lock(&controller_mutex);
		
	if(ret == 0)
	{
		// This while loop should continue until all the trains were scheduled.
		while(active_train_threads > 0)
		{
			// Signal will not be missed
			while(number_of_waiting_trains == 0)
				ret = pthread_cond_wait(&signal_controller, &controller_mutex);
			
			if(ret != 0)
			{
				printf("Internal error on pthread_cond_wait call.\n");
				break;
			}
			
			// A train has arrived
			pthread_mutex_lock(&thread_mutex);
			get_from_station(&station, &get_train, last_train_crossed);
			pthread_mutex_unlock(&thread_mutex);
			
			last_train_crossed = get_train->direction;
			
			// Signal the selected thread /train to go
			ret = pthread_mutex_unlock(&main_track);
			
			if(ret != 0)
			{
				printf("Internal error on pthread_cond_wait call.\n");
				break;
			}
			
			get_train->signalled = 1;
			ret = pthread_cond_signal(&(get_train->go));
			
			if(ret != 0)
			{
				printf("Internal error on pthread_cond_wait call.\n");
				break;
			}
			
			ret = pthread_mutex_lock(&train_left_mutex);
		
			if(ret == 0)
			{
				// Wait until the train leave main track
				ret = pthread_cond_wait(&train_left, &train_left_mutex);
			}
			
			ret = pthread_mutex_unlock(&train_left_mutex);
			
			if(ret != 0)
			{
				printf("Internal error on pthread_cond_wait call.\n");
				break;
			}
		}
	}
	
	free(train_threads);
	pthread_exit(NULL);
}

// Train thread function
static void* TrainThread(void *param)
{
	train_info *train = (train_info*)param;
	int ret;
	char direction[5];
	
	if(train->direction == 'w')
		strcpy(direction, "West");
	else
		strcpy(direction, "East");
	
	
	ret = pthread_mutex_lock(&controller_mutex);
		
	if(ret == 0)
	{
		active_train_threads++;
		ret = pthread_mutex_unlock(&controller_mutex);
	}
	
	pthread_cond_signal(&signal_controller);
	
	// Wait until thread is activated.
	while(start_threads == 0);
	
	// Load train
	usleep((train->loadTime) * 100000);
	
	printf("Train %2d is ready to go %4s\n",train->inputId,direction);
	
	// We will be using a single list to store all the trains to make it more efficient.
	pthread_mutex_lock(&thread_mutex);
	add_to_station(&station, train);
	number_of_waiting_trains++;
	pthread_mutex_unlock(&thread_mutex);
	
	// Tell controller that the train has arrived
	pthread_cond_signal(&signal_controller);
	
	// Wait until the permission granted to use track
	ret = pthread_mutex_lock(&main_track);
	
	if(ret == 0)
	{
		while(train->signalled == 0)
		{
			ret = pthread_cond_wait(&(train->go), &main_track);
		}
		
		printf("Train %2d is ON the main track going %4s\n",train->inputId,direction);
		
		// Cross the main track
		usleep((train->crossTime) * 100000);
		printf("Train %2d is OFF the main track after going %4s\n",train->inputId,direction);
	}

	if(ret == 0)
	{
		number_of_waiting_trains--;
		active_train_threads--;
		
		pthread_cond_signal(&train_left);
	}
	
	pthread_cond_destroy(&(train->go));
	
	pthread_exit(NULL);
}
