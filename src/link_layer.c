// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source


int alarmEnabled = FALSE;
int almc = 0;

void receiveUA() {
    int counter = 0;
    while (STOP == FALSE)
    {
        unsigned char byte;
        int received = readByteSerialPort(&byte);
        
        if (1 == received) {
            
        }
        if (counter == BUF_SIZE) {
            STOP = TRUE;
            alarmEnabled = FALSE;
        }
    }
}


// Alarm function handler.
// This function will run whenever the signal SIGALRM is received.
void sigalarmHandler(int signal)
{
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
    
    alarmEnabled = TRUE;
    alarm(3);
}

sendPacketTransmitter



////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    if (-1 == openSerialPort(connectionParamenters.serialPort, connectionParamenters.baudRate)) {
        return -1;
    }

    if (LlTx == connectionParamenters.role) {
        unsigned char *packet = malloc(CONTROL_PACKET_SIZE)
        if (-1 == makePacketControl(F_FLAG, A_SEND_TRANSMITTER, C_SET)) {
            return -1;
        }
        if (-1 == sendPacketTransmitter(packet)) {
            return -1;
        }

        alarm(coconnectionParameters.timeout);

        int state = 0;
        int idx = 0;
        unsigned char *byte = malloc(sizeof(unsigned char));
        while (state != 5) {
            int retv = readByteSerialPort(byte);
            if (-1 == retv) {
                return -1;
            } else if (0 == retv) {
                continue;
            }

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
                    if (A^C == BCC) {
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

        free(packet);
        free(byte);
    } else {

    }

    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose()
{
    // TODO: Implement this function

    return 0;
}
