// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "packet.h"

#include <stdio.h>
#include <string.h>

static long get_file_size(FILE *file) {
    if (fseek(file, 0L, SEEK_END) == -1) {
        printf("ERROR: fseek failed.\n");
        return -1;
    }

    long fileSize = ftell(file);
    rewind(file);

    return fileSize;
}

static LinkLayer createLinkLayer(const char *serialPort, LinkLayerRole role, int baudRate, int nTries, int timeout) {
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

    if (0 == strcmp(role, "tx")) {
        params = createLinkLayer(serialPort, LlTx, baudRate, nTries, timeout);

        FILE *file = fopen(filename, "rb");
        if (NULL == file) {
            printf("ERROR: Couldn't open %s.\n", filename);
            return;
        }
        long fileSize = get_file_size(file);

        if (-1 == fileSize) {
            printf("ERROR: Couldn't get file size.\n");
            if (0 != fclose(file)) {
                printf("ERROR: Couldn't close %s.\n", filename);
            }
            return;
        }

        if (-1 == llopen(params)) {
            printf("ERROR: Couldn't open connection.\n");
            if (0 != fclose(file)) {
                printf("ERROR: Couldn't close %s.\n", filename);
            }
            return;
        }

        // Send start packet
        int packetSize = buildControlPacket(packet, PCF_START, fileSize, filename);
        if (-1 == llwrite(packet, packetSize)) {
            printf("ERROR: llwrite failed.\n");
            goto cleanup;
        }

        size_t read;
        while (TRUE) {
            unsigned char data[MAX_DATA_FIELD_SIZE];

            read = fread(data, sizeof(unsigned char), MAX_DATA_FIELD_SIZE, file);
            if (0 != ferror(file)) {
                printf("ERROR: Error in %s.\n", filename);
                goto cleanup;
            }

            if (0 == read) {
                break;
            }

            if (-1 == buildDataPacket(packet, data, read)) {
                printf("ERROR: Building data packet failed.\n");
                goto cleanup;
            }

            if (-1 == llwrite(packet, read)) {
                printf("ERROR: llwrite failed.\n");
                goto cleanup;
            }
        }

        // Send end packet
        packetSize = buildControlPacket(packet, PCF_END, fileSize, filename);
        if (-1 == llwrite(packet, packetSize)) {
            printf("ERROR: llwrite failed.\n");
            goto cleanup;
        }

    } else {
        params = createLinkLayer(serialPort, LlTx, baudRate, nTries, timeout);

        if (-1 == llopen(params)) {
            printf("ERROR: Couldn't open connection.\n");
            return;
        }

        while (!isStartPacket(packet)) {
            if (-1 == llread(packet)) {
                printf("ERROR: Couldn't read file.\n");
                return;
            }
        }

        PacketControlField control;
        size_t size;
        char readFilename[MAX_FILENAME_SIZE];
        if (-1 == readControlPacket(packet, &control, &size, readFilename)) {
            printf("ERROR: Reading control packet failed.\n");
            goto cleanup;
        }

        file = fopen(readFilename, "w");
        if (NULL == file) {
            printf("ERROR: Couldn't open %s.\n", readFilename);
            if (-1 == llclose(params)) {
                printf("ERROR: Couldn't close connection.\n");
            }
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
    if (-1 == llclose(params)) {
        printf("ERROR: Couldn't close connection.\n");
    }
    if (0 != fclose(file)) {
        printf("ERROR: Couldn't close %s.\n", filename);
    }
}
