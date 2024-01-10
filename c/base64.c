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

#define offset_A (  0 + 'A')
#define offset_a (-26 + 'a')
#define offset_0 (-52 + '0')

#define bin2ascii(index)              \
switch ( index ) {                    \
case 0 ... 25  : index +=   0 + 'A' ; \
  break;                              \
case 26 ... 51 : index += -26 + 'a';  \
  break;                              \
case 52 ... 61 : index += -52 + '0';  \
  break;                              \
case 62        : index = '+';         \
  break;                              \
case 63        : index = '/';         \
  break;                              \
}

#define ascii2bin(index)               \
switch ( index ) {                     \
case 'A' ... 'Z' : index -=  0 + 'A' ; \
  break;                               \
case 'a' ... 'z' : index -= -26 + 'a'; \
  break;                               \
case '0' ... '9' : index -= -52 + '0'; \
  break;                               \
case '+'         : index = 62;         \
  break;                               \
case '/'         : index = 63;         \
  break;                               \
}


int encode (byte * in_buff, byte * out_buff) {
  int b0;  // The three input bytes
  int b1;
  int b2;
  int c0;  // The four output characters
  int c1;
  int c2;
  int c3;

  int num;       // The number of bytes to process
  int in_count;  // The input byte number being processed
  int out_count; // The output byte number being processed
  int bytes;     // The number of remaining bytes;  num % 3

  do {
    in_count  = 0;
    out_count = 0;

    num = read(0, (void *) in_buff, size_in_buff);
    if (num == 0) break;

    // Process three bytes at a time -- but don't process the last row
    // The last row may need to have padding added based upon the number
    // of bytes in that row: 1, 2, or 3 bytes
    int x = num - 3;
    for (in_count=0 ; in_count < x; in_count += 3) {

      // Load 24 bits, which is three bytes
      b0 = in_buff[in_count + 0];
      b1 = in_buff[in_count + 1];
      b2 = in_buff[in_count + 2];

      // Merge the bytes into a single 24-bit value
      b0 =  (b0 << 16) | (b1 << 8) | b2;      // b0 = (((b0 << 8) | b1 )<< 8) | b2;   
                                              // Rewrite for the ARM architecture

      // Chop 24-bit value into 4 groups of 6-bits
      c0 = (b0 >> 18);           //  c0 = (b0 >> 18) & 0x3F;
      c1 = (b0 >> 12) & 0x3F;    //  c1 = (b0 >> 12) & 0x3F;
      c2 = (b0 >>  6) & 0x3F;    //  c2 = (b0 >>  6) & 0x3F;
      c3 = (b0 & 0x3F);          //  c3 = (b0 >>  0) & 0x3F;

      // Map the 6-bit binary values into ASCII
      bin2ascii(c0);
      bin2ascii(c1);
      bin2ascii(c2);
      bin2ascii(c3);

      // Store the individual ASCII values
      out_buff[out_count + 0] = c0;
      out_buff[out_count + 1] = c1;
      out_buff[out_count + 2] = c2;
      out_buff[out_count + 3] = c3;        

      out_count = out_count + 4;
    }

    // Handle the last row, which might need padding
    // Note this could be the body of the loop in total
    //   The last row is processed separately to simplify 
    //   the explanation of the code -- for teaching purposes.
    {

      // Determine the number of bytes in the row
      // For the last row it is
      bytes = num - in_count;

      // For any row, it is:
      // bytes = (num - in_count ) % 3;
      
      // Load up to 24 bits, which is three bytes
      b0 = in_buff[in_count + 0];

      switch (bytes) {
        case 3:  // i.e., three bytes to process
          b1 = in_buff[in_count + 1];
          b2 = in_buff[in_count + 2];
          break;
        case 2:
          b1 = in_buff[in_count + 1];
          b2 = 0;
          break;
        case 1:
          b1 = 0;
          b2 = 0;
          break;
      }
 
      // Merge the bytes into a single 24-bit value
      b0 =  (b0 << 16) | (b1 << 8) | b2;      // b0 = (((b0 << 8) | b1 )<< 8) | b2;   
                                              // Rewrite for the ARM architecture

      // Chop 24-bit value into 4 groups of 6-bits
      c0 = (b0 >> 18);           //  c0 = (b0 >> 18) & 0x3F;
      c1 = (b0 >> 12) & 0x3F;    //  c1 = (b0 >> 12) & 0x3F;
      c2 = (b0 >>  6) & 0x3F;    //  c2 = (b0 >>  6) & 0x3F;
      c3 = (b0 & 0x3F);          //  c3 = (b0 >>  0) & 0x3F;

      // Map the 6-bit binary values into ASCII
      bin2ascii(c0);
      bin2ascii(c1);
      // bin2ascii(c2);
      // bin2ascii(c3);

      // Store the individual ASCII values
      out_buff[out_count + 0] = c0;
      out_buff[out_count + 1] = c1;
      switch (bytes) {
        case 3:
          bin2ascii(c2);
          bin2ascii(c3);
          out_buff[out_count + 2] = c2;
          out_buff[out_count + 3] = c3;
          break;
       case 2:
          bin2ascii(c2);
          out_buff[out_count + 2] = c2;
          out_buff[out_count + 3] = '=';
          break;
       case 1:
          out_buff[out_count + 2] = '=';
          out_buff[out_count + 3] = '=';
          break;
       }
       out_count = out_count + 4;
    }

     write(1, (void *) out_buff, out_count);

  } while (1==1);
  return 0;
}



int decode (byte * in_buff, byte * out_buff) {
  int b0;  // The three output bytes
  int b1;
  int b2;
  int c0;  // The four input characters
  int c1;
  int c2;
  int c3;

  int num;              // The number of bytes to process in each BLOCK
  int in_count;         // The input byte number being processed within the BLOCK
  int out_count;        // The output byte number being processed within the BLOCK
  int output_bytes;     // The number of output bytes for the current ROW

  do {
    in_count  = 0;
    out_count = 0;

    num = read(0, (void *) in_buff, size_in_buff);
    if (num == 0) break;

    // Process four bytes at a time
    // but don't process the last row

    // Process four bytes at a time -- but don't process the last row
    // The last row may need to have padding removed.
    output_bytes = 3;
    for (in_count=0 ; in_count < num; in_count += 4) {

      // Load the first ASCII characters
      c0 = in_buff[in_count + 0];
      if (c0 == '\n' ) {
        // skip over any blank lines
        break;
      }
      // Load the next 3 ASCII characters
      c0 = in_buff[in_count + 0];
      c1 = in_buff[in_count + 1];
      c2 = in_buff[in_count + 2];
      c3 = in_buff[in_count + 3];

      // Unmap the ASCII characters, creating four groups of 6-bit values

      ascii2bin(c0);
      ascii2bin(c1);

      if (c3 != '=') {
        ascii2bin(c3);
      } else {
        c3 = 0;
        output_bytes = 1;
      }
      if (c2 != '=') {
        ascii2bin(c2);
      } else {
        c2 = 0;
        output_bytes = 2;
      }

      // Merge the four 6-bit values into a single 24-bit value 
      c0 = (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

      // Chop the 24-bit value into three groups of 8-bits
      b0 = (c0 >> 16);          //  b0 = (c0 >> 16) & 0xFF;
      b1 = (c0 >> 8) & 0xFF;    //  b1 = (c0 >>  8) & 0xFF;
      b2 = c0 & 0xFF;           //  b2 = (c0 >>  0) & 0xFF;

      // Store the individual 8-bit values
      out_buff[out_count + 0] = b0;
      out_buff[out_count + 1] = b1;
      out_buff[out_count + 2] = b2;

      // Here we need to print out only the correct number of chars
      out_count = out_count + output_bytes;
    }
    write(1, (void *) out_buff, out_count);

  } while (1==1);
  return 0;
}


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


