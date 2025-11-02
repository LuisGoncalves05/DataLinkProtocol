// Link layer protocol implementation

#include "link_layer.h"
#include "alarm.h"
#include "frame.h"
#include "link_layer_utils.h"
#include "serial_port.h"
#include "stats.h"
#include <stdio.h>
#include <string.h>

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

    if (-1 == setupAlarm()) {
        printf("ERROR: setupAlarm failed.\n");
        return -1;
    }

    resetAlarm();
    if (LlTx == parameters.role) {
        // Send SET frame (with timeout)
        while (alarmState.alarmCount < parameters.nRetransmissions) {
            if (-1 == sendControlFrame(sentFrame, A_SEND_TRANSMITTER, C_SET)) {
                printf("ERROR: sendControlFrame failed.\n");
                removeAlarm();
                return -1;
            }

            setAlarm(parameters.timeout);
            // Receive UA frame, if any other discard it
            ControlState state = CONTROL_START;
            do {
                int retv = receiveControlFrame(receivedFrame, &state, TRUE);
                if (-1 == retv) {
                    printf("ERROR: receiveControlFrame failed.\n");
                    removeAlarm();
                    return -1;
                }
                if (-2 == retv) {
                    memset(receivedFrame, 0, CONTROL_FRAME_SIZE);
                    break;
                }
            } while (!frameIsType(receivedFrame, C_UA));
            
            // Received the correct type of control frame
            if (frameIsType(receivedFrame, C_UA)) {
                removeAlarm();
                return 0;
            }

            // If it did not receive the correct frame type then it must have timed out
        }

        removeAlarm();
        printf("ERROR: Timed out while trying to establish connection.\n");
        return -1;

    } else {
        // Receive SET frame
        ControlState state = CONTROL_START;
        int retv = receiveControlFrame(receivedFrame, &state, FALSE);

        if (-1 == retv) {
            printf("ERROR: receiveControlFrame failed.\n");
            return -1;
        }
        if (-2 == retv) {
            printf("ERROR: receiveControlFrame timed out.\n");
            return -1;
        }

        if (!frameIsType(receivedFrame, C_SET)) {
            printf("ERROR: received a non SET frame.\n");
            return -1;
        }

        // Send UA frame
        if (-1 == sendControlFrame(sentFrame, A_REPLY_RECEIVER, C_UA)) {
            printf("ERROR: sendControlFrame failed.\n");
            return -1;
        }

        return 0;
    }
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

    resetAlarm();
    int attempts = 0;
    while (alarmState.alarmCount < parameters.nRetransmissions) {
        setAlarm(parameters.timeout);

        printf("[ll] Attempt #%d, Timeouts #%d - sending frame number %d.\n", attempts, alarmState.alarmCount, frame_number);

        int read = writeBytesSerialPort(sentFrame, new_frame_size);
        if (-1 == read) {
            printf("ERROR: writeBytesSerialPort failed.\n");
            return -1;
        }
        statistics.frames++;
        statistics.bytes += read;

        // Receive Rej or RR frame
        ControlState state = CONTROL_START;
        do {
            int retv = receiveControlFrame(receivedFrame, &state, TRUE);
            if (-1 == retv) {
                printf("ERROR: receiveControlFrame failed.\n");
                removeAlarm();
                return -1;
            }
            if (-2 == retv) {
                memset(receivedFrame, 0, CONTROL_FRAME_SIZE);
                break;
            }
        } while (!frameIsType(receivedFrame, C_RR(!frame_number)) &&
                 !frameIsType(receivedFrame, C_REJ(frame_number)));

        if (frameIsType(receivedFrame, C_RR(!frame_number))) {
            frame_number = !frame_number;
            printf("[ll] Received RR(%d) - Success.\n", frame_number);
            removeAlarm();
            return 0;
        } else if (frameIsType(receivedFrame, C_REJ(frame_number))) {
            statistics.rejFrames++;
            printf("[ll] Received REJ(%d) - Resend frame.\n", frame_number);
        }

        attempts++;
    }

    removeAlarm();
    printf("ERROR: Timed out while trying to send frame.\n");
    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////

int llread(unsigned char *packet) {
    unsigned char frame[CONTROL_FRAME_SIZE + 2 * (MAX_PAYLOAD_SIZE + 1)];

    int attempts = 0;
    while (TRUE) {
        printf("[ll] Attempt #%d - waiting for frame number %d.\n", attempts, frame_number);

        // Receive I frame
        InformationState state = INFORMATION_START;
        int frame_size = receiveInformationFrame(frame, &state);

        if (-1 == frame_size) {
            printf("ERROR: receiveInformationFrame failed.\n");
            return -1;
        }
        if (-2 == frame_size) {
            printf("ERROR: receiveInformationFrame timed out.\n");
            return -1;
        }
        statistics.frames++;

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
                int data_size = data_end - data_begin;
                memcpy(packet, &frame[data_begin], data_size);
                frame_number = !frame_number;
                // Success
                if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_RR(frame_number))) {
                    printf("ERROR: sendControlFrame failed.\n");
                    return -1;
                }
                printf("[ll] Sent RR(%d) - Success.\n", frame_number);
                return data_size;
            } else {
                statistics.rejFrames++;

                printf("[ll] Sent REJ(%d) - BCC2 mismatch.\n", frame_number);
                // BCC2 failed
                if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_REJ(frame_number))) {
                    printf("ERROR: sendControlFrame failed.\n");
                    return -1;
                }
            }
        } else {
            printf("[ll] Sent RR(%d) - Wrong frame.\n", frame_number);
            // Wrong frame number
            if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_RR(frame_number))) {
                printf("ERROR: sendControlFrame failed.\n");
                return -1;
            }
        }
        attempts++;
    }
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
    unsigned char sentFrame[CONTROL_FRAME_SIZE];
    unsigned char receivedFrame[CONTROL_FRAME_SIZE];

    if (LlTx == parameters.role) {
        resetAlarm();
        // Send DISC frame (with timeout)
        while (alarmState.alarmCount < parameters.nRetransmissions) {
            setAlarm(parameters.timeout);

            if (-1 == sendControlFrame(sentFrame, A_SEND_TRANSMITTER, C_DISC)) {
                printf("ERROR: sendControlFrame failed.\n");
                removeAlarm();
                if (-1 == closeSerialPort()) {
                    printf("ERROR: closeSerialPort failed.\n");
                }
                return -1;
            }

            // Receive DISC frame, if any other discard it
            ControlState state = CONTROL_START;
            do {
                int retv = receiveControlFrame(receivedFrame, &state, TRUE);
                if (-1 == retv) {
                    printf("ERROR: receiveControlFrame failed.\n");
                    removeAlarm();
                    if (-1 == closeSerialPort()) {
                        printf("ERROR: closeSerialPort failed.\n");
                    }
                    return -1;
                }
                if (-2 == retv) {
                    memset(receivedFrame, 0, CONTROL_FRAME_SIZE);
                    break;
                }
            } while (!frameIsType(receivedFrame, C_DISC));

            // Received the correct type of control frame
            if (frameIsType(receivedFrame, C_DISC)) {
                removeAlarm();

                // Send UA frame
                if (-1 == sendControlFrame(sentFrame, A_REPLY_TRANSMITTER, C_UA)) {
                    printf("ERROR: sendControlFrame failed.\n");
                    if (-1 == closeSerialPort()) {
                        printf("ERROR: closeSerialPort failed.\n");
                    }
                    return -1;
                }

                return closeSerialPort();
            }

            // If it did not receive the correct frame type then it must have timed out
        }

    } else {
        // Receive DISC frame
        ControlState state = CONTROL_START;
        int retv = receiveControlFrame(receivedFrame, &state, FALSE);

        if (-1 == retv) {
            printf("ERROR: receiveControlFrame failed.\n");
            if (-1 == closeSerialPort()) {
                printf("ERROR: closeSerialPort failed.\n");
            }
            return -1;
        }

        if (-2 == retv) {
            printf("ERROR: receiveControlFrame timed out.\n");
            if (-1 == closeSerialPort()) {
                printf("ERROR: closeSerialPort failed.\n");
            }
            return -1;
        }

        if (!frameIsType(receivedFrame, C_DISC)) {
            printf("ERROR: received a non DISC frame.\n");
            if (-1 == closeSerialPort()) {
                printf("ERROR: closeSerialPort failed.\n");
            }
            return -1;
        }

        if (-1 == sendControlFrame(sentFrame, A_SEND_TRANSMITTER, C_DISC)) {
            printf("ERROR: sendControlFrame failed.\n");
            if (-1 == closeSerialPort()) {
                printf("ERROR: closeSerialPort failed.\n");
            }
            return -1;
        }

        return 0;
    }

    printf("ERROR: Timed out while trying to close connection.\n");
    if (-1 == closeSerialPort()) {
        printf("ERROR: closeSerialPort failed.\n");
    }
    return -1;
}
