#include "stats.h"
#include <stdio.h>

struct Stats statistics = {
    .start = {0, 0},
    .end = {0, 0},
    .baudRate = 0,
    .dataBytesSent = 0,
    .bytesSent = 0};

int initStats() {
    statistics.start.tv_sec = 0;
    statistics.start.tv_nsec = 0;
    statistics.end.tv_sec = 0;
    statistics.end.tv_nsec = 0;
    statistics.dataBytesSent = 0;
    statistics.bytesSent = 0;

    if (-1 == clock_gettime(CLOCK_MONOTONIC, &statistics.start)) {
        return -1;
    }

    return 0;
}

int printStats(LinkLayer *params) {
    if (NULL == params) {
        return -1;
    }

    if (-1 == clock_gettime(CLOCK_MONOTONIC, &statistics.end)) {
        return -1;
    }

    statistics.baudRate = params->baudRate;
    double elapsed = (statistics.end.tv_sec - statistics.start.tv_sec) + (statistics.end.tv_nsec - statistics.start.tv_nsec) / 1000000000.0;
    double bitrate = statistics.bytesSent * 8.0 / elapsed;
    const char *role = params->role == LlTx ? "Sent:    " : "Received:";

    printf("\n");
    printf("╔═════════════════════════════════════╗\n");
    printf("║          TRANSMISSION STATS         ║\n");
    printf("╠═════════════════════════════════════╣\n");
    printf("║ Data Bits %s  %10d b   ║\n", role, statistics.dataBytesSent * 8);
    printf("║ Total Bits %s %10d b   ║\n", role, statistics.bytesSent * 8);
    printf("║ Elapsed Time:        %10.3f s   ║\n", elapsed);
    printf("║ Bitrate:             %10.2f bps ║\n", bitrate);
    printf("║ Baud Rate:           %10d bps ║\n", statistics.baudRate);
    printf("╠═════════════════════════════════════╣\n");
    printf("║ Efficiency:          %10.2f %%   ║\n", 100 * bitrate / statistics.baudRate);
    printf("╚═════════════════════════════════════╝\n");
    printf("\n");

    return 0;
}