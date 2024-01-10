// Name:  base64.c
// Synopsis: an implementation of the encoding and decoding of base64 
// Usage:
//    base64    // encodes ASCII string on stdin into base64
//    base64 -d // decodes binary string on stdin into ASCII-bytes 
//
// API:
#define byte char

int encode(byte * in_buff, byte * out_buff);
int decode(byte * in_buff, byte * out_buff);

// Encoding Method: 
//    1. read a block of information
//       1. load load three bytes from the input location (totaling 24 bits;  3 * 8 == 24)
//       1. merge bytes into one register      
//       1. chop the bytes into 4 6-bit chunks
//       1. map each of the 4 6-bit chunks into an 8-bit ASCII value
//       1. store each of these 8-bit ASCII values to the output location
//    2. handle padding
//
// Decoding Method
//
//

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>


#define _4K_ (4096)

#define size_in_buff   (3072) //  1024 * 3 == 3072 
#define size_out_buff  (3072) //   768 * 4 == 3072 
#define num_in_buff
#define num_out_buff 

byte in_buffer[ _4K_ ];
byte out_buffer[ _4K_ ];


int main(int argc, char * argv[]) {
  if (argc == 2) {
    // just assume that argv[1] == "-d"
    decode(in_buffer, out_buffer);
  } else {
    encode(in_buffer, out_buffer);
  }
  printf("\n");
  return 0;
}
