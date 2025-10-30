// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "packet.h"
#include "stats.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static long getFileSize(FILE *file) {
    if (-1 == fseek(file, 0L, SEEK_END)) {
        printf("ERROR: fseek failed.\n");
        return -1;
    }

    long fileSize = ftell(file);
    rewind(file);

    return fileSize;
}

static LinkLayer buildLinkLayer(const char *serialPort, LinkLayerRole role, int baudRate, int nTries, int timeout) {
    LinkLayer params = {
        .role = role,
        .baudRate = baudRate,
        .nRetransmissions = nTries,
        .timeout = timeout};

    strncpy(params.serialPort, serialPort, sizeof(params.serialPort) - 1);
    params.serialPort[sizeof(params.serialPort) - 1] = '\0';

    return params;
}

void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename) {
    FILE *file;
    LinkLayer params;
    unsigned char packet[MAX_PAYLOAD_SIZE];

    if (-1 == initStats()) {
        printf("ERROR: initStats failed");
        return;
    }

    if (0 == strcmp(role, "tx")) {
        params = buildLinkLayer(serialPort, LlTx, baudRate, nTries, timeout);

        file = fopen(filename, "rb");
        if (NULL == file) {
            printf("ERROR: Couldn't open %s.\n", filename);
            return;
        }
        /*debug*/ printf("File opened successfully.\n");

        long fileSize = getFileSize(file);

        if (-1 == fileSize) {
            printf("ERROR: Couldn't get file size.\n");
            if (0 != fclose(file)) {
                printf("ERROR: Couldn't close %s.\n", filename);
            }
            return;
        }

        /*debug*/ printf("LinkLayer opening.\n");

        if (-1 == llopen(params)) {
            printf("ERROR: Couldn't open connection.\n");
            if (0 != fclose(file)) {
                printf("ERROR: Couldn't close %s.\n", filename);
            }
            return;
        }

        /*debug*/ printf("LinkLayer opened successfully.\n");

        // Send start packet
        int packetSize = buildControlPacket(packet, PCF_START, fileSize, filename);
        if (-1 == packetSize) {
            printf("ERROR: buildControlPacket failed.\n");
            goto cleanup;
        }

        if (-1 == llwrite(packet, packetSize)) {
            printf("ERROR: llwrite failed.\n");
            goto cleanup;
        }

        /*debug*/ printf("Sent a start packet.\n");
        /*debug*/ int sent = 0;
        /*debug*/ int total = fileSize / MAX_DATA_FIELD_SIZE + (fileSize % MAX_DATA_FIELD_SIZE > 0);

        // Send file information packets
        while (TRUE) {
            unsigned char data[MAX_DATA_FIELD_SIZE];

            size_t read = fread(data, sizeof(unsigned char), MAX_DATA_FIELD_SIZE, file);
            if (0 != ferror(file)) {
                printf("ERROR: Error in %s.\n", filename);
                goto cleanup;
            }

            if (0 == read) {
                break;
            }

            int size = buildDataPacket(packet, data, read);
            if (-1 == size) {
                printf("ERROR: Building data packet failed.\n");
                goto cleanup;
            }
            statistics.dataBytesSent += read;

            if (-1 == llwrite(packet, size)) {
                printf("ERROR: llwrite failed.\n");
                goto cleanup;
            }
            /*debug*/ printf("Sent an information packet, %d/%d.\n", ++sent, total);
        }

        // Send end packet
        packetSize = buildControlPacket(packet, PCF_END, fileSize, filename);
        if (-1 == packetSize) {
            printf("ERROR: buildControlPacket failed.\n");
            goto cleanup;
        }

        if (-1 == llwrite(packet, packetSize)) {
            printf("ERROR: llwrite failed.\n");
            goto cleanup;
        }

        /*debug*/ printf("Sent an end packet.\n");

    } else {
        params = buildLinkLayer(serialPort, LlRx, baudRate, nTries, timeout);

        /*debug*/ printf("LinkLayer opening.\n");

        if (-1 == llopen(params)) {
            printf("ERROR: Couldn't open connection.\n");
            return;
        }

        /*debug*/ printf("LinkLayer opened successfully.\n");

        // Receive start packet
        int read = llread(packet);
        if (-1 == read) {
            printf("ERROR: Couldn't read file.\n");
            return;
        }
        if (!isStartPacket(packet)) {
            printf("ERROR: Couldn't read start packet.\n");
            return;
        }

        PacketControlField control;
        size_t size;
        char readFilename[MAX_FILENAME_SIZE]; // Never used, but sent and received anyways
        if (-1 == readControlPacket(packet, &control, &size, readFilename)) {
            printf("ERROR: Reading control packet failed.\n");
            goto cleanup;
        }

        /*debug*/ printf("Received a start packet.\n");

        file = fopen(filename, "wb");
        if (NULL == file) {
            printf("ERROR: Couldn't open %s.\n", filename);
            if (-1 == llclose()) {
                printf("ERROR: Couldn't close connection.\n");
            }
            return;
        }
        /*debug*/ printf("File opened successfully.\n");

        // Receive all other packets
        while (TRUE) {
            if (-1 == llread(packet)) {
                printf("ERROR: Couldn't read file.\n");
                goto cleanup;
            }

            if (isEndPacket(packet)) {
                break;
            }

            size_t read;
            unsigned char *data = readDataPacket(packet, &read);
            if (NULL == data) {
                printf("ERROR: Couldn't read data packet\n");
                goto cleanup;
            }

            /*debug*/ printf("Received an information packet.\n");

            size_t written = fwrite(data, sizeof(unsigned char), read, file);
            if (written < read) {
                printf("ERROR: Error in %s.\n", filename);
                goto cleanup;
            }
            statistics.dataBytesSent += read;
        }

        /*debug*/ printf("Received an end packet.\n");
    }

cleanup:
    if (-1 == llclose()) {
        printf("ERROR: Couldn't close connection.\n");
    }
    /*debug*/ printf("LinkLayer closed successfully.\n");

    if (0 != fclose(file)) {
        printf("ERROR: Couldn't close %s.\n", filename);
    }
    /*debug*/ printf("File closed successfully.\n");

    if (-1 == printStats(&params)) {
        printf("ERROR: printStats failed.\n");
        return;
    }
}
