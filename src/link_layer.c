// Link layer protocol implementation

#include "link_layer.h"
#include "link_layer_utils.h"
#include "serial_port.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

// Information frame has the same fields as control frame with 2 more, the payload and respective bcc that can be byte stuffed
unsigned char packet[CONTROL_FRAME_SIZE + 2 * (MAX_PAYLOAD_SIZE + 1)];

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
    if (-1 == openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate)) {
        return -1;
    }

    if (LlTx == connectionParameters.role) {
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
