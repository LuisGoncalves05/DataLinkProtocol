#include "frame.h"
#include "link_layer_utils.h"

#include <stdio.h>
#include <string.h>

int byteStuff(unsigned char *frame, size_t size) {
    if (NULL == frame) {
        printf("ERROR: NULL parameter frame.\n");
        return -1;
    }

    // excluding first and last bytes to mantain the real flags
    unsigned char copy[size];
    memcpy(copy, frame, size);
    size_t frameIdx = CONTROL_FRAME_SIZE - 1; // jump F A C and BCC
    for (size_t i = CONTROL_FRAME_SIZE - 1; i < size - 1; i++, frameIdx++) {
        if (F_FLAG == copy[i] || ESCAPE == copy[i]) {
            frame[frameIdx++] = ESCAPE;
            frame[frameIdx] = ESCAPE_REPLACE(copy[i]);
        } else {
            frame[frameIdx] = copy[i];
        }
    }

    // keep last element as an untouched flag
    frame[frameIdx++] = F_FLAG;
    return frameIdx;
}

int buildControlFrame(unsigned char *frame, int addressField, int controlField) {
    if (NULL == frame) {
        printf("ERROR: NULL parameter frame.\n");
        return -1;
    }

    if (!VALID_A(addressField)) {
        printf("ERROR: Invalid parameter addressField.\n");
        return -1;
    }

    if (!VALID_C_CONTROL(controlField)) {
        printf("ERROR: Invalid parameter controlField.\n");
        return -1;
    }

    frame[0] = frame[4] = F_FLAG;
    frame[1] = addressField;
    frame[2] = controlField;
    frame[3] = frame[1] ^ frame[2];

    return 0;
}

int buildInformationFrame(unsigned char *frame, int addressField, int controlField, unsigned char *data, size_t size) {
    if (NULL == frame) {
        printf("ERROR: NULL parameter frame.\n");
        return -1;
    }

    if (!VALID_A(addressField)) {
        printf("ERROR: Invalid parameter addressField.\n");
        return -1;
    }

    if (!VALID_C_INFORMATION(controlField)) {
        printf("ERROR: Invalid parameter controlField.\n");
        return -1;
    }

    unsigned char *originalFrame = frame;

    frame[0] = F_FLAG;
    frame[1] = addressField;
    frame[2] = controlField;
    frame[3] = frame[1] ^ frame[2];
    frame += 4;

    memcpy(frame, data, size);

    unsigned char xor = frame[0];
    for (size_t i = 1; i < size; i++) {
        xor ^= frame[i];
    }
    frame += size;

    frame[0] = xor;
    frame[1] = F_FLAG;
    frame += 2;

    return frame - originalFrame;
}

