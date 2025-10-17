#ifndef _STATE_MACHINES_H_
#define _STATE_MACHINES_H_

#define F_FLAG 0x7E

#define A_SEND_TRANSMITTER 0x03
#define A_REPLY_RECEIVER 0x03
#define A_SEND_RECEIVER 0x01
#define A_REPLY_TRANSMITTER 0x01
#define VALID_A(byte) ((byte) == 0x03 || (byte) == 0x01)

#define C_SET 0x03
#define C_UA 0x07
#define C_RR0 0xAA
#define C_RR1 0xAB
#define C_REJ0 0x54
#define C_REJ1 0x55
#define C_DISC 0x0B
#define VALID_C_CONTROL(byte) (C_SET == (byte) ||  \
                               C_UA == (byte) ||   \
                               C_RR0 == (byte) ||  \
                               C_RR1 == (byte) ||  \
                               C_REJ0 == (byte) || \
                               C_REJ1 == (byte) || \
                               C_DISC == (byte))

#define C_FRAME0 0x00
#define C_FRAME1 0x80
#define VALID_C_INFORMATION(byte) (C_FRAME0 == (byte) || C_FRAME1 == (byte))

#define CONTROL_FRAME_SIZE 5
#define INFORMATION_FRAME_BASE_SIZE 6

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
    INFORMATION_BCC2_RECEIVED,
    INFORMATION_STOP
} InformationState;

int readPacketControl(unsigned char *buf);

// Transition function of the Control State Machine. Mutates state to the respective transition, writing on packet and moving idx accordingly.
// Return 0 on success or -1 on error.
int nextStateControl(ControlState *state, unsigned char *byte, unsigned char *packet, unsigned int *idx);

int readPacketData(unsigned char *buf);

// Transition function of the Information State Machine. Mutates state to the respective transition, writing on packet and moving idx accordingly.
// Return 0 on success or -1 on error.
int nextStateInformation(InformationState *state, unsigned char *byte, unsigned char *packet, unsigned int *idx);

#endif