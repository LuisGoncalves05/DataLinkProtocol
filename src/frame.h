#ifndef _FRAME_H_
#define _FRAME_H_

#include "link_layer_utils.h"
#include <stddef.h>

// Byte stuff frame of size size.
// Return size of frame on success or -1 on error.
int byteStuff(unsigned char *frame, size_t size);

// Sets the fields of the control frame.
// Return 0 on success or -1 on error.
int buildControlFrame(unsigned char *frame, int addressField, int controlField);

// Sets the fields of the information frame.
// Return size of frame on success or -1 on error.
int buildInformationFrame(unsigned char *frame);

// Reads the fields from the control packet.
// Return size of frame on success or -1 on error.
int readControlFrame(unsigned char *frame);

// Reads the fields from the information frame.
// Return size of frame on success or -1 on error.
int readDataFrame(unsigned char *frame);

#endif