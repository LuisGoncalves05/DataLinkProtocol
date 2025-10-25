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
#define C_RR(n) (0xAA + (n))
#define C_REJ(n) (0x54 + (n))
#define C_DISC 0x0B
#define VALID_C_CONTROL(byte) (C_SET == (byte) ||    \
                               C_UA == (byte) ||     \
                               C_RR(0) == (byte) ||  \
                               C_RR(1) == (byte) ||  \
                               C_REJ(0) == (byte) || \
                               C_REJ(1) == (byte) || \
                               C_DISC == (byte))

#define C_FRAME(n) ((n) << 7)
#define VALID_C_INFORMATION(byte) (C_FRAME(0) == (byte) || C_FRAME(1) == (byte))

#define CONTROL_FRAME_SIZE 5

#endif