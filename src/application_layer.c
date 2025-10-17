// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "packet.h"

#include <stdio.h>


void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename) {
    unsigned char packet[MAX_PAYLOAD_SIZE];

    int transmitter = 0 == strcmp(role, "tx");
    const char *mode = transmitter ? "rb" : "wb";

    FILE *file = fopen(filename, mode);
    if (NULL == file) {
        printf("ERROR: Couldn't open %s.\n", filename);
        return;
    }

    if (transmitter) {
        LinkLayer params = {
            serialPort,
            LlTx,
            baudRate,
            nTries,
            timeout};

        if (-1 == llopen(params)) {
            printf("ERROR: Couldn't open connection.\n");
            goto cleanup;
        }

        // Send start packet
        int packetSize = buildControlPacket(packet, PCF_START, size, filename);

        while (TRUE) {
            size_t read = fread(packet, sizeof(unsigned char), MAX_PAYLOAD_SIZE, file);
            if (0 != ferror(file)) {
                printf("ERROR: Error in %s.\n", filename);
                break;
            }

            if (0 == read) {
                break;
            }

            if (-1 == llwrite(packet, read)) {
                printf("ERROR: llwrite failed.\n");
                break;
            }
        }

        if (-1 == llclose(params)) {
            printf("ERROR: Couldn't close connection.\n");
        }
    } else {
        LinkLayer params = {
            serialPort,
            LlRx,
            baudRate,
            nTries,
            timeout};

        if (-1 == llopen(params)) {
            printf("ERROR: Couldn't open connection.\n");
            return; 
        }

        int read = llread(packet);
        while (-1 != read) {
            size_t written = fwrite(packet, sizeof(unsigned char), read, file);
            if (written < read) {
                printf("ERROR: Error in %s.\n", filename);
                break;
            }
            read = llread(packet);
        }

        if (-1 == llclose(params)) {
            printf("ERROR: Couldn't close connection.\n");
        }
    }

cleanup:
    if (0 != fclose(file)) {
        printf("ERROR: Couldn't close %s.\n", filename);
    }
}
