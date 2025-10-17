#ifndef _PACKET_H_
#define _PACKET_H_

#include "link_layer.h"

// Payload has a limited number of bytes, and each data packet needs C, L2 and L1 bytes and Data therefore lowering that size by 3
#define MAX_DATA_FIELD_SIZE (MAX_PAYLOAD_SIZE - 3)
#define MAX_FILENAME_SIZE 64

typedef enum {
    PCF_START = 1,
    PCF_DATA = 2,
    PCF_END = 3
} PacketControlField;

typedef enum {
    CPPT_SIZE = 0,
    CPPT_NAME = 1
} ControlPacketParameterType;

#define NUM_PARAMETERS_CONTROL_PACKET 2

// Sets the fields of the data packet.
// Return size of packet on success or -1 on error.
int buildDataPacket(unsigned char *packet, unsigned char *data, size_t size);

// Sets the fields of the control packet.
// Return size of packet on success or -1 on error.
int buildControlPacket(unsigned char *packet, PacketControlField control, size_t size, const char *filename);

// Reads the fields from the data packet.
// Return size of packet on success or -1 on error.
int readDataPacket(unsigned char *packet, unsigned char *data, size_t *size);

// Reads the fields from the control packet.
// Return size of packet on success or -1 on error.
int readControlPacket(unsigned char *packet, PacketControlField *control, size_t *size, char *filename);

// Check if it is an START packet.
// Return -1 if NULL, TRUE if is START packet, else FALSE
int isStartPacket(unsigned char *packet);

// Check if it is an END packet.
// Return -1 if NULL, TRUE if is END packet, else FALSE
int isEndPacket(unsigned char *packet);

#endif