// Link layer protocol implementation

#include "link_layer.h"
#include "frame.h"
#include "link_layer_utils.h"
#include "serial_port.h"
#include <string.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

LinkLayer parameters;
// Information frame has the same fields as control frame with 2 more, the payload and respective bcc that can be byte stuffed
unsigned char frame[CONTROL_FRAME_SIZE + 2 * (MAX_PAYLOAD_SIZE + 1)];
int frame_number = 0;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
    parameters = connectionParameters;
    if (-1 == openSerialPort(parameters.serialPort, parameters.baudRate)) {
        return -1;
    }

    if (LlTx == parameters.role) {
        // Send SET frame
        if (-1 == sendControlFrame(frame, A_SEND_TRANSMITTER, C_SET)) {
            return -1;
        }

        // Receive UA frame
        ControlState state = CONTROL_START;
        int received;
        do {
            received = receiveControlFrame(frame, &state);
            if (-1 == received) {
                return -1;
            }
        } while (!frameIsType(frame, C_UA));

    } else {
        // Receive SET frame
        ControlState state = CONTROL_START;
        int received;
        do {
            received = receiveControlFrame(frame, &state);
            if (-1 == received) {
                return -1;
            }
        } while (!frameIsType(frame, C_SET));

        // Send UA frame
        if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_UA)) {
            return -1;
        }
    }

    memset(frame, 0, sizeof(frame));
    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    int frame_size = buildInformationFrame(frame, A_SEND_TRANSMITTER, C_FRAME(frame_number), buf, bufSize);
    if (-1 == frame_size) {
        return -1;
    }

    int new_frame_size = byteStuff(frame, frame_size);
    if (-1 == new_frame_size) {
        return -1;
    }

    do {
        if (-1 == writeBytesSerialPort(frame, new_frame_size)) {
            return -1;
        }

        // Receive Rej or RR(other frame_number) frame
        ControlState state = CONTROL_START;
        int received;
        do {
            received = receiveControlFrame(frame, &state);
            if (-1 == received) {
                return -1;
            }
        } while (!(frameIsType(frame, C_REJ(frame_number)) ||
                   frameIsType(frame, C_RR(!frame_number))));
    } while (frameIsType(frame, C_REJ(frame_number)));

    frame_number = !frame_number;
    memset(frame, 0, sizeof(frame));
    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    // Receive I frame
    InformationState state = CONTROL_START;
    int frame_size;
    do {
        frame_size = receiveInformationFrame(frame, &state);
    } while (-1 == frame_size);

    frame_size = byteDeStuff(frame, frame_size);
    if (-1 == frame_size) {
        return -1;
    }

    unsigned char received_bcc2 = frame[frame_size - 2]; // BCC2 is the byte received before the flag
    unsigned char checked_bcc2 = frame[4];
    for (size_t i = 5; i < frame_size - 1; i++) {
        checked_bcc2 ^= frame[i];
    }

    if (checked_bcc2 != received_bcc2) {
        if (frameIsType(frame, C_FRAME(frame_number))) {
            // Send RR frame
            if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_REJ(frame_number))) {
                return -1;
            }
        } else {
            // Send RR frame
            if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_RR(frame_number))) {
                return -1;
            }
        }
    } else {
        // Send RR frame
        if (-1 == sendControlFrame(frame, A_REPLY_RECEIVER, C_RR(frame_number))) {
            return -1;
        }

        if (frameIsType(frame, C_FRAME(frame_number))) {
            frame_number = !frame_number;
            memcpy(packet, &frame[5], (frame_size-1) - 5);
        } else {
            // Discard frame
        }
    }
    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
    if (LlTx == parameters.role) {
        // Send DISC frame
        if (-1 == sendControlFrame(frame, A_SEND_TRANSMITTER, C_DISC)) {
            return -1;
        }

        // Receive DISC frame
        ControlState state = CONTROL_START;
        int received;
        do {
            received = receiveControlFrame(frame, &state);
            if (-1 == received) {
                return -1;
            }
        } while (!frameIsType(frame, C_DISC));

        // Send UA frame
        if (-1 == sendControlFrame(frame, A_REPLY_TRANSMITTER, C_UA)) {
            return -1;
        }

    } else {
        // Receive DISC frame
        ControlState state = CONTROL_START;
        int received;
        do {
            received = receiveControlFrame(frame, &state);
            if (-1 == received) {
                return -1;
            }
        } while (!frameIsType(frame, C_DISC));

        // Send DISC frame
        if (-1 == sendControlFrame(frame, A_SEND_RECEIVER, C_DISC)) {
            return -1;
        }

        // Receive UA frame
        state = CONTROL_START;
        do {
            received = receiveControlFrame(frame, &state);
            if (-1 == received) {
                return -1;
            }
        } while (!frameIsType(frame, C_UA));
    }

    return closeSerialPort();
}
