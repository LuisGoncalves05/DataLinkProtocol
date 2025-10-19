#ifndef _LINK_LAYER_UTILS_H_
#define _LINK_LAYER_UTILS_H_

#define F_FLAG 0x7E
#define ESCAPE 0x7D
#define ESCAPE_REPLACE(byte) ((byte) ^ 0x20)

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

#endif