// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int alarmEnabled = FALSE;
int timeout;

// Alter this for stuffing 
unsigned char controlPacket[CONTROL_PACKET_SIZE];
unsigned char informationPacket[INFORMATION_FRAME_BASE_SIZE + MAX_PAYLOAD_SIZE];

void SendPacket(int signal) {
    alarmEnabled = FALSE;

    int ret = writeBytesSerialPort(bytes, BUF_SIZE);
    if (ret == -1) {
        printf("Error writing to serial port.\n");
    } else {
        printf("5 bytes written to serial port: ");
        for (int i = 0; i < BUF_SIZE; i++) {
            printf("byte %d = 0x%02X, ", i, bytes[i]);
        }
        printf("\n");
    }

    alarm(3);
    alarmEnabled = TRUE;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
    if (-1 == openSerialPort(connectionParamenters.serialPort, connectionParamenters.baudRate)) {
        return -1;
    }
    timeout = coconnectionParameters.timeout;

    if (LlTx == connectionParamenters.role) {
        if (-1 == makePacketControl(F_FLAG, A_SEND_TRANSMITTER, C_SET, packet)) {
            return -1;
        }
        if (-1 == sendPacketTransmitter(packet)) {
            return -1;
        }

        alarm(timeout);

        int state = 0;
        int idx = 0;
        unsigned char byte[1];
        while (state != STOP) {
            int retv = readByteSerialPort(byte);
            if (-1 == retv) {
                return -1;
            } else if (0 == retv) {
                continue;
            }

            if (-1 == nextStateControl(&state, byte, packet, &idx)) {
                return -1;
            }
        }

        return 0;
    } else {
    }

    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
    // TODO: Implement this function

    return 0;
}
