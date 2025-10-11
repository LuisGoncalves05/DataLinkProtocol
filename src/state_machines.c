#include "state_machines.h"

int nextStateControl(State state) {
    switch (state) {
        case 0:
            if (F_FLAG == *byte) {
                packet[idx++] = byte;
                state = 1;
            }
            break;
        case 1:
            if (A_REPLY_RECEIVER == *byte) {
                packet[idx++] = byte;
                state = 2;
            } else if (F_FLAG == *byte) {
                // Stay in the same state
            } else {
                idx = 0;
                state = 0;
            }
            break;
        case 2:
            if (C_RR1 == *byte) {
                packet[idx++] = byte;
                state = 3;
            } else if (F_FLAG == *byte) {
                idx = 1;
                state = 1;

            } else {
                idx = 0;
                state = 0;
            }
            break;
        case 3:
            unsigned char A = packet[1];
            unsigned char C = packet[2];
            unsigned char BCC = packet[3];
            if (A ^ C == BCC) {
                packet[idx++] = byte;
                state = 4;
            } else if (F_FLAG == *byte) {
                idx = 1;
                state = 1;
            } else {
                idx = 0;
                state = 0;
            }
            break;
        case 4:
            if (F_FLAG == *byte) {
                packet[idx++] = byte;
                state = 5;
            } else {
                idx = 0;
                state = 0;
            }
            break;
    }
}