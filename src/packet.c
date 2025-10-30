#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int buildDataPacket(unsigned char *packet, unsigned char *data, size_t size) {
    if (NULL == packet || NULL == data) {
        printf("ERROR: NULL parameter packet: %p data: %p.\n", packet, data);
        return -1;
    }

    if (size > MAX_DATA_FIELD_SIZE) {
        printf("ERROR: size too big, %zu.\n", size);
        return -1;
    }

    packet[0] = PCF_DATA;
    packet[1] = (size >> 8) & 0xff;
    packet[2] = size & 0xff;
    memcpy(packet + 3, data, size);

    return size + 3;
}

unsigned char *readDataPacket(unsigned char *packet, size_t *size) {
    if (NULL == packet|| NULL == size) {
        printf("ERROR: NULL parameter packet:%p size:%p.\n", packet, size);
        return NULL;
    }

    PacketControlField control = packet[0];
    if (PCF_DATA != control) {
        printf("ERROR: Unknown packet control field=%d.\n", control);
        return NULL;
    }

    *size = (packet[1] << 8) + packet[2];
    packet += 3;

    return packet;
}

int buildControlPacket(unsigned char *packet, PacketControlField control, size_t size, const char *filename) {
    if (NULL == packet || NULL == filename) {
        printf("ERROR: NULL parameter packet=%p filename=%p.\n", packet, filename);
        return -1;
    }

    switch (control) {
        case PCF_START:
        case PCF_END:
            break;
        default:
            printf("ERROR: Unknown control field, %d.\n", control);
            return -1;
    }

    unsigned char *initialPacket = packet;

    packet[0] = control;
    packet++;

    // File size field
    packet[0] = CPPT_SIZE;
    packet[1] = sizeof(size_t);
    for (int i = 0; i < sizeof(size_t); i++) {
        int shift = (sizeof(size_t) - 1 - i) * 8;
        packet[2 + i] = (size >> shift) & 0xFF;
    }

    packet += 2 + packet[1];

    // File name field
    packet[0] = CPPT_NAME;
    packet[1] = strlen(filename);
    memcpy(&packet[2], filename, packet[1]);

    packet += 2 + packet[1];

    int packetSize = packet - initialPacket;

    return packetSize;
}

int readControlPacket(unsigned char *packet, PacketControlField *control, size_t *size, char *filename) {
    if (NULL == packet || NULL == control || NULL == size || NULL == filename) {
        printf("ERROR: NULL parameter packet:%p control:%p size:%p filename:%p.\n", packet, control, size, filename);
        return -1;
    }

    *control = packet[0];
    packet++;

    switch (*control) {
        case PCF_START:
        case PCF_END:
            break;
        default:
            printf("ERROR: Invalid control field, %d.\n", *control);
            return -1;
    }

    for (int i = 0; i < NUM_PARAMETERS_CONTROL_PACKET; i++) {
        ControlPacketParameterType type = packet[0];
        unsigned char length = packet[1];
        packet += 2;

        switch (type) {
            case CPPT_SIZE:
                *size = 0;
                for (int j = 0; j < length; j++) {
                    *size = (*size << 8) | packet[j];
                }
                packet += length;
                break;

            case CPPT_NAME:
                memcpy(filename, packet, length);
                filename[length] = '\0';
                packet += length;
                break;

            default:
                printf("ERROR: Unknown parameter type, %d.\n", type);
                return -1;
        }
    }

    return 0;
}

int isStartPacket(unsigned char *packet) {
    if (NULL == packet) {
        printf("ERROR: NULL parameter packet.\n");
        return -1;
    }

    return PCF_START == packet[0];
}

int isEndPacket(unsigned char *packet) {
    if (NULL == packet) {
        printf("ERROR: NULL parameter packet.\n");
        return -1;
    }

    return PCF_END == packet[0];
}
