#include "stats.h"
#include <stdio.h>

struct Stats statistics = {
    .start = {0, 0},
    .end = {0, 0},
    .frames = 0,
    .rejFrames = 0,
    .timeouts = 0,
    .baudRate = 0,
    .dataBytes = 0,
    .bytes = 0};

int initStats() {
    return clock_gettime(CLOCK_MONOTONIC, &statistics.start);
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
    double bitrate = statistics.bytes * 8.0 / elapsed;
    const char *role = params->role == LlTx ? "Sent" : "Received";
    const char *padding = params->role == LlTx ? "    " : "";

    printf("\n");
    printf("╔═════════════════════════════════════╗\n");
    printf("║          TRANSMISSION STATS         ║\n");
    printf("╠═════════════════════════════════════╣\n");
    printf("║ Data Bits %s: %s %10d b   ║\n", role, padding, statistics.dataBytes * 8);
    printf("║ Total Bits %s:%s %10d b   ║\n", role, padding, statistics.bytes * 8);
    printf("║ %s frames:    %s %10d     ║\n", role, padding, statistics.frames);
    printf("║ Rejected frames:     %10d     ║\n", statistics.rejFrames);
    printf("║ Timeouts:            %10d     ║\n", statistics.timeouts);
    printf("║ Elapsed Time:        %10.3f s   ║\n", elapsed);
    printf("║ Bitrate:             %10.2f bps ║\n", bitrate);
    printf("║ Baud Rate:           %10d bps ║\n", statistics.baudRate);
    printf("╠═════════════════════════════════════╣\n");
    printf("║ Efficiency:          %10.2f %%   ║\n", 100 * bitrate / statistics.baudRate);
    printf("╚═════════════════════════════════════╝\n");
    printf("\n");

    return 0;
}