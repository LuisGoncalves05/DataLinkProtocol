#ifndef _STATS_H_
#define _STATS_H_

#include <time.h>
#include "link_layer.h"

// Struct that stores the state of the alarm.
struct Stats {
    struct timespec start, end;
    int baudRate;
    int dataBytesSent;
    int bytesSent;
};

// Global statistics intitialized in stats.c.
extern struct Stats statistics;

int initStats();

int printStats(LinkLayer *params);

#endif
