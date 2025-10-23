#ifndef _FRAME_H_
#define _FRAME_H_

#include "link_layer_utils.h"
#include "serial_port.h"
#include "state_machines.h"
#include <stddef.h>

// Byte stuff frame of size size.
// Return size of frame on success or -1 on error.
int byteStuff(unsigned char *frame, size_t size);

int byteDeStuff(unsigned char *frame, size_t size);

// Sets the fields of the control frame.
// Return 0 on success or -1 on error.
int buildControlFrame(unsigned char *frame, int addressField, int controlField);

// Sets the fields of the information frame.
// Return size of frame on success or -1 on error.
int buildInformationFrame(unsigned char *frame, int addressField, int controlField, const unsigned char *data, size_t size);

int sendControlFrame(unsigned char *frame, int addressField, int controlField);

int frameIsType(unsigned char *frame, int controlField);

int receiveControlFrame(unsigned char *frame, ControlState *state);

int receiveInformationFrame(unsigned char *frame, InformationState *state);

#endif