#define byte char
int encode(byte * in_buff, byte * out_buff);
int decode(byte * in_buff, byte * out_buff);

// Name:  base64.c
// Synopsis: an implementation of the encoding and decoding of base64 
// Usage:
//    base64    // encodes ASCII string on stdin into base64
//    base64 -d // decodes binary string on stdin into ASCII-bytes 
//
// API:

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>

#include "base64.h"
    
byte in_buffer[ _4K_ ];
byte out_buffer[ _4K_ ];

int main(int argc, char * argv[]) {
  if (argc == 2) {
    // just assume that argv[1] == "-d"
    decode(in_buffer, out_buffer);
  } else {
    encode(in_buffer, out_buffer);
    printf("\n");
  }
  return 0;
}

