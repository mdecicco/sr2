#pragma once
typedef unsigned char byte;
unsigned int VAG_UnpackTest(byte *data, unsigned int datasize, int offset);
void VAG_Unpack(byte *data, int offset, int datasize, byte **bufferptr, int *outsize);