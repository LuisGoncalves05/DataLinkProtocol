// Link layer protocol implementation

#include "link_layer.h"
#include "frame.h"
#include "link_layer_utils.h"
#include "serial_port.h"
#include <string.h>
#include <stdio.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

LinkLayer parameters;
int frame_number = 0;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
    parameters = connectionParameters;

    unsigned char sentFrame[CONTROL_FRAME_SIZE];
    unsigned char receivedFrame[CONTROL_FRAME_SIZE];

    if (-1 == openSerialPort(parameters.serialPort, parameters.baudRate)) {
        printf("ERROR: openSerialPort failed.\n");
        return -1;
    }

    if (LlTx == parameters.role) {
        // Send SET frame
        if (-1 == sendControlFrame(sentFrame, A_SEND_TRANSMITTER, C_SET)) {
            printf("ERROR: sendControlFrame failed.\n");
            return -1;
        }

        // Receive UA frame, if any other discard it
        ControlState state = CONTROL_START;
        do {
            if (-1 == receiveControlFrame(receivedFrame, &state)) {
                printf("ERROR: receiveControlFrame failed.\n");
                return -1;
            }
        } while (TRUE != frameIsType(receivedFrame, C_UA));

    } else {
        // Receive SET frame, if any other discard it
        ControlState state = CONTROL_START;
        do {
            if (-1 == receiveControlFrame(receivedFrame, &state)) {
                printf("ERROR: receiveControlFrame failed.\n");
                return -1;
            }
        } while (TRUE != frameIsType(receivedFrame, C_SET));

        // Send UA frame
        if (-1 == sendControlFrame(sentFrame, A_REPLY_RECEIVER, C_UA)) {
            printf("ERROR: sendControlFrame failed.\n");
            return -1;
        }
    }

    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    unsigned char sentFrame[CONTROL_FRAME_SIZE + 2 * (MAX_PAYLOAD_SIZE + 1)];
    unsigned char receivedFrame[CONTROL_FRAME_SIZE];

    int frame_size = buildInformationFrame(sentFrame, A_SEND_TRANSMITTER, C_FRAME(frame_number), buf, bufSize);
    if (-1 == frame_size) {
        printf("ERROR: buildInformationFrame failed.\n");
        return -1;
    }

    int new_frame_size = byteStuff(sentFrame, frame_size);
    if (-1 == new_frame_size) {
        printf("ERROR: byteStuff failed.\n");
        return -1;
    }

    do {
        if (-1 == writeBytesSerialPort(sentFrame, new_frame_size)) {
            printf("ERROR: writeBytesSerialPort failed.\n");
            return -1;
        }

        // Receive Rej or RR(other frame_number) frame
        ControlState state = CONTROL_START;
        do {
            if (-1 == receiveControlFrame(receivedFrame, &state)) {
                printf("ERROR: receiveControlFrame failed.\n");
                return -1;
            }
        } while (TRUE != (frameIsType(receivedFrame, C_REJ(frame_number)) ||
                          frameIsType(receivedFrame, C_RR(!frame_number))));

    } while (frameIsType(receivedFrame, C_REJ(frame_number)));

    frame_number = !frame_number;

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    int retriesLeft = parameters.nRetransmissions;

    unsigned char frame[CONTROL_FRAME_SIZE + 2 * (MAX_PAYLOAD_SIZE + 1)];

    while (retriesLeft > 0) {
        // Receive I frame
        InformationState state = INFORMATION_START;
        int frame_size = receiveInformationFrame(frame, &state);
        if (-1 == frame_size) {
            printf("ERROR: receiveInformationFrame failed.\n");
            return -1;
        }

        // Destuff I frame
        frame_size = byteDeStuff(frame, frame_size);
        if (-1 == frame_size) {
            printf("ERROR: byteDeStuff failed.\n");
            return -1;
        }

        int data_begin = 4;
        int data_end = frame_size - 2;
        unsigned char received_bcc2 = frame[data_end];
        unsigned char checked_bcc2 = frame[data_begin];

        for (size_t i = data_begin + 1; i < data_end; i++) {
            checked_bcc2 ^= frame[i];
        }

        if (frameIsType(frame, C_FRAME(frame_number))) {
            if (checked_bcc2 == received_bcc2) {
                // Success
                // Correct bcc2 and frame_number
                int data_size = data_end - data_begin;
                memcpy(packet, &frame[data_begin], data_size);
                frame_number = !frame_number;

                if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_RR(frame_number))) {
                    printf("ERROR: sendControlFrame failed.\n");
                    return -1;
                }

                return data_size;
            } else {
                // BCC2 failed
                // Send REJ frame and retry
                if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_REJ(frame_number))) {
                    printf("ERROR: sendControlFrame failed.\n");
                    return -1;
                }
            }
        } else {
            // Wrong frame number
            // Send RR(frame_number) and retry
            if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_RR(frame_number))) {
                printf("ERROR: sendControlFrame failed.\n");
                return -1;
            }
        }

        retriesLeft--;
    }

    printf("ERROR: Max retries exceeded.\n");
    return -1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
    unsigned char sentFrame[CONTROL_FRAME_SIZE];
    unsigned char receivedFrame[CONTROL_FRAME_SIZE];

    if (LlTx == parameters.role) {
        // Send DISC frame
        if (-1 == sendControlFrame(sentFrame, A_SEND_TRANSMITTER, C_DISC)) {
            printf("ERROR: sendControlFrame failed.\n");
            return -1;
        }

        // Receive DISC frame
        ControlState state = CONTROL_START;
        do {
            if (-1 == receiveControlFrame(receivedFrame, &state)) {
                printf("ERROR: receiveControlFrame failed.\n");
                return -1;
            }
        } while (!frameIsType(receivedFrame, C_DISC));

        // Send UA frame
        if (-1 == sendControlFrame(sentFrame, A_REPLY_TRANSMITTER, C_UA)) {
            printf("ERROR: sendControlFrame failed.\n");
            return -1;
        }

    } else {
        // Receive DISC frame
        ControlState state = CONTROL_START;
        do {
            if (-1 == receiveControlFrame(receivedFrame, &state)) {
                printf("ERROR: receiveControlFrame failed.\n");
                return -1;
            }
        } while (!frameIsType(receivedFrame, C_DISC));

        // Send DISC frame
        if (-1 == sendControlFrame(sentFrame, A_SEND_RECEIVER, C_DISC)) {
            printf("ERROR: sendControlFrame failed.\n");
            return -1;
        }

        // Receive UA frame
        state = CONTROL_START;
        do {
            if (-1 == receiveControlFrame(receivedFrame, &state)) {
                printf("ERROR: receiveControlFrame failed.\n");
                return -1;
            }
        } while (!frameIsType(receivedFrame, C_UA));
    }

    return closeSerialPort();
}
