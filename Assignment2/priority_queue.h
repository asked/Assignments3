#ifndef _PRIORITY_QUEUE_H
#define _PRIORITY_QUEUE_H

#include "train_data.h"

void add_to_station(station_queue **queue, train_info *train);
void get_from_station(station_queue **queue, train_info **train, char last_train_direction);

#endif
