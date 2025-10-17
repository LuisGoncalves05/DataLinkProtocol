// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "packet.h"

#include <stdio.h>


void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename) {
    unsigned char packet[MAX_PAYLOAD_SIZE];

    int transmitter = 0 == strcmp(role, "tx");
    const char *mode = transmitter ? "rb" : "wb";

    FILE *file;

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

        while (!isStartPacket(packet)){
            if (-1 == llread(packet)) {
                printf("ERROR: Couldn't read file.\n");
                return; 
            } 
        }
        
        PacketControlField control;
        size_t size;
        char readFilename[MAX_FILENAME_SIZE];
        if(-1 == readControlPacket(packet, &control, &size, readFilename)) {
            printf("ERROR: Reading control packet failed.\n");
            goto cleanup;
        }

        file = fopen(readFilename, "w");
        if (NULL == file) {
            printf("ERROR: Couldn't open %s.\n", readFilename);
            //llclose()
            return;
        }

        while (!isEndPacket(packet)) {
            int read = llread(packet);
            if (-1 == read) {
                printf("ERROR: Couldn't read file.\n");
                goto cleanup; 
            }
            
            size_t written = fwrite(packet, sizeof(unsigned char), read, file);
            if (written < read) {
                printf("ERROR: Error in %s.\n", filename);
                goto cleanup; 
            }
        }
    }

cleanup:
    if (0 != fclose(file)) {
        printf("ERROR: Couldn't close %s.\n", filename);
    }
}
