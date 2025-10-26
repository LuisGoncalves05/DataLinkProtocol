#include "state_machines.h"
#include <stddef.h>
#include <stdio.h>

int nextStateControl(ControlState *state, unsigned char *byte, unsigned char *frame, unsigned int *idx) {
    if (NULL == state || NULL == byte || NULL == frame || NULL == idx) {
        printf("ERROR: NULL parameter state: %p byte: %p frame: %p idx: %p.\n", state, byte, frame, idx);
        return -1;
    }

    switch (*state) {
        case CONTROL_START:
            if (F_FLAG == *byte) {
                frame[(*idx)++] = *byte;
                *state = CONTROL_F_RECEIVED;
            }
            break;
        case CONTROL_F_RECEIVED:
            if (VALID_A(*byte)) {
                frame[(*idx)++] = *byte;
                *state = CONTROL_A_RECEIVED;
            } else if (F_FLAG == *byte) {
                *state = CONTROL_F_RECEIVED;
            } else {
                *idx = 0;
                *state = CONTROL_START;
            }
            break;
        case CONTROL_A_RECEIVED:
            if (VALID_C_CONTROL(*byte)) {
                frame[(*idx)++] = *byte;
                *state = CONTROL_C_RECEIVED;
            } else if (F_FLAG == *byte) {
                *idx = 1;
                *state = CONTROL_F_RECEIVED;
            } else {
                *idx = 0;
                *state = CONTROL_START;
            }
            break;
        case CONTROL_C_RECEIVED: {
            unsigned char A = frame[1];
            unsigned char C = frame[2];
            if ((A ^ C) == *byte) {
                frame[(*idx)++] = *byte;
                *state = CONTROL_BCC_RECEIVED;
            } else if (F_FLAG == *byte) {
                *idx = 1;
                *state = CONTROL_F_RECEIVED;
            } else {
                *idx = 0;
                *state = CONTROL_START;
            }
            break;
        }
        case CONTROL_BCC_RECEIVED:
            if (F_FLAG == *byte) {
                frame[(*idx)++] = *byte;
                *state = CONTROL_STOP;
            } else {
                *idx = 0;
                *state = CONTROL_START;
            }
            break;
        default:
            printf("ERROR: Invalid ControlState.\n");
            return -1;
    }

    return 0;
}

int nextStateInformation(InformationState *state, unsigned char *byte, unsigned char *frame, unsigned int *idx) {
    if (NULL == state || NULL == byte || NULL == frame || NULL == idx) {
        printf("ERROR: NULL parameter state: %p byte: %p frame: %p idx: %p.\n", state, byte, frame, idx);
        return -1;
    }

    switch (*state) {
        case INFORMATION_START:
            if (F_FLAG == *byte) {
                frame[(*idx)++] = *byte;
                *state = INFORMATION_F_RECEIVED;
            }
            break;
        case INFORMATION_F_RECEIVED:
            if (VALID_A(*byte)) {
                frame[(*idx)++] = *byte;
                *state = INFORMATION_A_RECEIVED;
            } else if (F_FLAG == *byte) {
                *state = INFORMATION_F_RECEIVED;
            } else {
                *idx = 0;
                *state = INFORMATION_START;
            }
            break;
        case INFORMATION_A_RECEIVED:
            if (VALID_C_INFORMATION(*byte)) {
                frame[(*idx)++] = *byte;
                *state = INFORMATION_C_RECEIVED;
            } else if (F_FLAG == *byte) {
                *idx = 1;
                *state = INFORMATION_F_RECEIVED;
            } else {
                *idx = 0;
                *state = INFORMATION_START;
            }
            break;
        case INFORMATION_C_RECEIVED: {
            unsigned char A = frame[1];
            unsigned char C = frame[2];
            if ((A ^ C) == *byte) {
                frame[(*idx)++] = *byte;
                *state = INFORMATION_READING_DATA;
            } else if (F_FLAG == *byte) {
                *idx = 1;
                *state = INFORMATION_F_RECEIVED;
            } else {
                *idx = 0;
                *state = INFORMATION_START;
            }
            break;
        }
        case INFORMATION_READING_DATA:
            frame[(*idx)++] = *byte;
            if (F_FLAG == *byte) {
                *state = INFORMATION_STOP;
            }
            break;
        default:
            printf("ERROR: Invalid InformationState.\n");
            return -1;
    }

    return 0;
}