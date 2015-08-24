#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "train_data.h"
#include "priority_queue.h"

static void init_queue_item(station_queue *queue);

// queue: Allocated queue
static void init_queue_item(station_queue *queue)
{
	queue->next = NULL;
	queue->prev = NULL;
	queue->train = NULL;
}

// queue: pointer
// train: train_info structure
// Adds a train to the queue

void add_to_station(station_queue **queue, train_info *train)
{
	/* Malloc the new item here, will be freed when taking trains from the queue */
	station_queue* new_item = (station_queue*)malloc(sizeof(station_queue));
	station_queue* temp = NULL;

	init_queue_item(new_item);

	/* Attach the train */
	new_item->train = train;

	/* Attach the new item to the head of the queue */
	temp = *queue;

	/* Attach new head */
	*queue = new_item;
	
	/* Arrange pointers */
	(*queue)->prev = NULL;

	/* If temp is NULL the next pointer will also be NULL */
	(*queue)->next = temp;

	if(temp)
	{
		/* temp is not NULL
		 * that means the queue was not empty
		 */

		temp->prev = *queue;
	}
}

// queue: previously allocated station_queue
// train: allocated structure
// last_train_direction: last direction 'e' or 'w'
//
// Will detach a train from queue and put it to train_info structure.
// The correct train will be selected in one call.

void get_from_station(station_queue **queue, train_info **train, char last_train_direction)
{
	/* selected item is the currently selected item which is might be the item with
	 * highest priority
	 */
	station_queue* selected_item = (*queue);
	
	/* current item is the item which is compared with the selected item to see whether
	 * this has better claim to use the main track
	 */

	station_queue* current_item = (*queue);
	
	/* following pointers are used to store train data temporarily while removing the item
	 */

	station_queue* prev_item = NULL;
	station_queue* temp = (*queue);

	if(!queue)
	{
		printf("Fatal error!!!\n");
		return;
	}

	/* Find the train with highest priority. */
	while(current_item)
	{
		if((current_item->train->priority) > (selected_item->train->priority)){
			// Entry under consideration, have higher priority
			selected_item = current_item;
			
		}else if((current_item->train->priority) == (selected_item->train->priority)){
			// Priorities are equal
			if((selected_item->train->direction) != (current_item->train->direction)){
				// Trains are traveling in the opposite direction	
				
				// Compare with the direction of the last train which crossed the main track.
				if((selected_item->train->direction) == last_train_direction){
					// Selected item should be changed.
					selected_item = current_item;
				}
				
			}else{
				// Trains are traveling in the same direction
				
				// Compare the loading times
				if((current_item->train->loadTime) < (selected_item->train->loadTime)){
					// Selected item should be changed.
					selected_item = current_item;
					
				}else if((current_item->train->loadTime) == (selected_item->train->loadTime)){
					// They have same loading time
					
					// Compare the location in the file
					if((current_item->train->inputId) < (selected_item->train->inputId)){
						// Need to change the selection
						selected_item = current_item;
					}
				}
			}
		}
		
		current_item = current_item->next;
	}

	if(!selected_item)
	{
		printf("Fatal error!!!\n");
		return;
	}

	*train = (selected_item->train);

	// We have selected the train. Need to remove it from the queue.
	prev_item = selected_item->prev;
	current_item = selected_item->next;

	free(selected_item);

	if(prev_item)
	{
		// There is a item before. 
		prev_item->next = current_item;
		
		if(current_item)
		{
			// There is an item after
			current_item->prev = prev_item;
		}
	}else
	{
		if(current_item)
		{
			// Head replacement
			current_item->prev = NULL;
			(*queue) = current_item;
		}else
		{
			(*queue) = NULL;
		}
	}

}
