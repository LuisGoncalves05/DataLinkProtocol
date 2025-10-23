#ifndef _STATE_MACHINES_H_
#define _STATE_MACHINES_H_

#include "link_layer_utils.h"

typedef enum {
    CONTROL_START,
    CONTROL_F_RECEIVED,
    CONTROL_A_RECEIVED,
    CONTROL_C_RECEIVED,
    CONTROL_BCC_RECEIVED,
    CONTROL_STOP
} ControlState;

typedef enum {
    INFORMATION_START,
    INFORMATION_F_RECEIVED,
    INFORMATION_A_RECEIVED,
    INFORMATION_C_RECEIVED,
    INFORMATION_BCC1_RECEIVED,
    INFORMATION_READING_DATA,
    INFORMATION_STOP
} InformationState;

// Transition function of the Control State Machine. Mutates state to the respective transition, writing on packet and moving idx accordingly.
// Return 0 on success or -1 on error.
int nextStateControl(ControlState *state, unsigned char *byte, unsigned char *packet, unsigned int *idx);

// Transition function of the Information State Machine. Mutates state to the respective transition, writing on packet and moving idx accordingly.
// Return 0 on success or -1 on error.
int nextStateInformation(InformationState *state, unsigned char *byte, unsigned char *packet, unsigned int *idx);

#endif