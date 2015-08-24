#ifndef _TRAIN_DATA_H
#define _TRAIN_DATA_H

typedef struct _train_info{
	char direction;
	int priority;
	int loadTime;
	int crossTime;
	int inputId; 		// Position of the file
	pthread_cond_t go;
	char signalled;
}train_info;


typedef struct _station_queue{
	train_info* train;
	struct _station_queue* next;
	struct _station_queue* prev;
}station_queue;


#endif
