#ifndef _STATS_H_
#define _STATS_H_

#include <time.h>
#include "link_layer.h"

// Struct that stores the state of the alarm.
struct Stats {
    struct timespec start, end;
    int frames;
    int rejFrames;
    int timeouts;
    int baudRate;
    int dataBytes;
    int bytes;
};

// Global statistics intitialized in stats.c.
extern struct Stats statistics;

// Starts the collection of statistics.
// Returns 0 on success or -1 on error.
int initStats();

// Prints the collected statistics.
// Returns 0 on success or -1 on error.
int printStats(LinkLayer *params);

#endif
